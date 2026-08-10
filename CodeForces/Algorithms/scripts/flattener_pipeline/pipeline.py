"""Two-stage orchestrator: render the template bundle, then rewrite the source.

``_render_template_bundle`` discovers the required ``NEED_*`` headers,
prunes them against actual user identifiers, walks transitive template
dependencies of any included module, and inlines each survivor. The
resulting :class:`FlattenedTemplateBundle` carries the rendered preamble,
ordered template sections, and the effective macro state.

``_render_flattened_source`` then walks the user source line-by-line: it
swaps the ``templates/Base.hpp`` include for the bundle, inlines any
``modules/`` headers behind a single section banner, and emits the deferred
``using namespace std;`` at the right anchor when requested.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

from flattener_core.headers import MODULE_SECTION_SEPARATOR, prune_template_headers_with_policy
from flattener_core.includes import (
    INLINE_ROOT_PREFIXES,
    collect_transitive_template_deps,
    inline_local_header,
    order_template_headers_by_dependencies,
    parse_project_include_line,
    resolve_project_include,
)
from flattener_core.lexer import append_with_blank_separator
from flattener_core.macros import MacroValueMap
from flattener_pipeline.context import FlattenContext, FlattenedTemplateBundle
from need_resolver import extract_need_macros_from_source
from profile_registry import load_registry

NEED_DEFINE_LINE_RE = re.compile(r"^\s*#\s*define\s+(NEED_\w+)\b")
MAIN_SOLVER_SECTION_MARKER = "/* Main Solver Function */"

_COND_DIRECTIVE_RE = re.compile(r"^\s*#\s*(?:if|elif|ifdef|ifndef)\b(.*)$")
_IDENT_RE = re.compile(r"[A-Za-z_]\w*")
SURVIVING_PROJECT_INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"(?:templates|modules)/')


def _referenced_condition_macros(blocks: list[str]) -> set[str]:
    """Collect identifiers used inside ``#if``-family directives in ``blocks``."""

    referenced: set[str] = set()
    for block in blocks:
        for line in block.splitlines():
            match = _COND_DIRECTIVE_RE.match(line)
            if match:
                referenced.update(_IDENT_RE.findall(match.group(1)))
    return referenced


def _render_resolved_prelude(blocks: list[str], macro_values: MacroValueMap) -> str:
    """Emit ``#ifndef``-guarded defines for every config switch the output tests.

    The flattener resolves the whole ``CP_*``/``NEED_*`` configuration itself
    (from ``profiles.toml`` + the source) but historically left those values
    only inside the inlined ``Config_defaults.hpp`` cascade. Restating the
    resolved value of each switch *actually referenced* by an emitted header
    lets the submission drop that cascade entirely while guaranteeing the
    compiler reaches the same feature decisions the flattener did. Compiler /
    feature-detect macros (``__GNUC__``, ``HAS_INT128``, ``PBDS_AVAILABLE``)
    are intentionally excluded — they must stay compiler-resolved.
    """

    lines: list[str] = []
    for name in sorted(_referenced_condition_macros(blocks)):
        if not (name.startswith("CP_") or name.startswith("NEED_")):
            continue
        if name not in macro_values:
            continue
        value = macro_values[name]
        if value is None:
            continue
        lines += [f"#ifndef {name}", f"  #define {name} {value}", "#endif"]
    return "\n".join(lines)


def _collect_needed_template_headers(
    ctx: FlattenContext,
    *,
    need_macros_found: set[str],
    enable_template_pruning: bool,
    macro_values: MacroValueMap,
) -> list[Path]:
    """Resolve and prune template headers implied by enabled ``NEED_*`` macros."""

    need_macros_found = set(load_registry().normalize_needs(need_macros_found))

    files_to_include: list[Path] = []
    included_names: set[str] = set()
    for macro, files in ctx.need_mapping.items():
        if macro not in need_macros_found:
            continue
        for filename in files:
            if filename in included_names:
                continue
            files_to_include.append(ctx.templates_dir / filename)
            included_names.add(filename)

    # Mirror Base.hpp's ``#if CP_USE_ADVANCED`` block: the opt-in type-safety
    # layer is included by the macro switch, not by a NEED_* feature, so without
    # this its symbols (cp::StrongType, cp::cast::*) only reach a submission when
    # pulled transitively (e.g. via the fast-I/O StrongType extension). Usage
    # pruning drops whichever of these the source does not reference.
    if macro_values.get("CP_USE_ADVANCED"):
        for filename in ("advanced/Concepts.hpp", "advanced/Cast.hpp", "advanced/Strong_Type.hpp"):
            if filename not in included_names:
                files_to_include.append(ctx.templates_dir / filename)
                included_names.add(filename)

    files_to_include = prune_template_headers_with_policy(
        files_to_include,
        ctx.source_content,
        enable_pruning=enable_template_pruning,
    )

    # Keep template deps from modules in the template section (before module separator).
    included_paths = {p.resolve() for p in files_to_include}
    for include_target in ctx.source_module_includes:
        for dep in collect_transitive_template_deps(
            ctx.project_root,
            include_target,
            exclude_template_files={"Preamble.hpp"},
            macro_values=macro_values,
        ):
            dep_resolved = dep.resolve()
            if dep_resolved in included_paths:
                continue
            files_to_include.append(dep)
            included_paths.add(dep_resolved)

    # Never inline Preamble as a template section; it is emitted once at the top.
    files_to_include = [p for p in files_to_include if p.resolve() != ctx.preamble_resolved]
    return order_template_headers_by_dependencies(
        ctx.project_root,
        files_to_include,
        macro_values=macro_values,
    )


def _render_template_bundle(
    ctx: FlattenContext,
    *,
    enable_template_pruning: bool,
    enable_module_pruning: bool,
) -> FlattenedTemplateBundle:
    """Render preamble and template sections independently from source rewrite."""

    need_macros_found = extract_need_macros_from_source(
        ctx.source_content, ctx.need_mapping.keys(), warn_stream=sys.stderr
    )
    effective_macro_values: MacroValueMap = dict(ctx.macro_values)
    if "NEED_FAST_IO" in need_macros_found:
        effective_macro_values["CP_FAST_IO_NAMESPACE_DEFINED"] = 1

    files_to_include = _collect_needed_template_headers(
        ctx,
        need_macros_found=need_macros_found,
        enable_template_pruning=enable_template_pruning,
        macro_values=effective_macro_values,
    )

    # The resolved config prelude (emitted below) supersedes the generated
    # Config_defaults cascade, so suppress those headers from the inlined
    # preamble instead of shipping ~30 lines of now-redundant ``#ifndef``s.
    preamble_inlined_headers: set[Path] = {
        path.resolve()
        for path in (
            ctx.templates_dir / "core" / "Config.hpp",
            ctx.templates_dir / "core" / "Config_defaults.hpp",
        )
        if path.is_file()
    }
    preamble_content = inline_local_header(
        ctx.project_root,
        ctx.preamble_path,
        preamble_inlined_headers,
        used_identifiers=set(ctx.used_identifiers),
        module_leaf_tokens=ctx.module_leaf_tokens,
        strip_module_docs=False,
        strip_template_docs=ctx.strip_template_docs,
        macro_values=effective_macro_values,
        enable_module_pruning=enable_module_pruning,
    )

    inlined_headers = set(preamble_inlined_headers)
    template_sections: list[str] = []
    for filepath in files_to_include:
        content = inline_local_header(
            ctx.project_root,
            filepath,
            inlined_headers,
            used_identifiers=set(ctx.used_identifiers),
            module_leaf_tokens=ctx.module_leaf_tokens,
            strip_module_docs=ctx.strip_module_docs,
            strip_template_docs=ctx.strip_template_docs,
            macro_values=effective_macro_values,
            enable_module_pruning=enable_module_pruning,
        )
        if content:
            template_sections.append(content)

    prelude_defines = _render_resolved_prelude(
        [preamble_content, *template_sections],
        effective_macro_values,
    )

    return FlattenedTemplateBundle(
        preamble_content=preamble_content,
        template_sections=tuple(template_sections),
        effective_macro_values=effective_macro_values,
        inlined_headers=frozenset(inlined_headers),
        prelude_defines=prelude_defines,
    )


def _render_flattened_source(
    ctx: FlattenContext,
    bundle: FlattenedTemplateBundle,
    *,
    enable_module_pruning: bool,
) -> str:
    """Rewrite original source using already-rendered template sections."""

    output_lines: list[str] = []
    skip_need_defines = False
    seen_base_include = False
    skip_blank_after_sections = False
    should_emit_std_namespace = (
        bundle.effective_macro_values.get("CP_USE_GLOBAL_STD_NAMESPACE") not in (None, 0)
        and "using namespace std;" not in ctx.source_content
    )
    defer_std_namespace_to_solver_section = (
        should_emit_std_namespace and MAIN_SOLVER_SECTION_MARKER in ctx.source_content
    )

    module_section_emitted = False
    inlined_headers = set(bundle.inlined_headers)

    for idx, line in enumerate(ctx.source_lines):
        stripped = line.strip()
        if skip_blank_after_sections:
            if not stripped:
                continue
            skip_blank_after_sections = False
        masked_line = ctx.source_masked_lines[idx] if idx < len(ctx.source_masked_lines) else ""
        include_name = parse_project_include_line(line, masked_line=masked_line)
        is_base_include = include_name is not None and include_name.replace("\\", "/").endswith(
            "templates/Base.hpp"
        )

        need_define_match = NEED_DEFINE_LINE_RE.match(stripped)
        if (
            not seen_base_include
            and need_define_match
            and need_define_match.group(1) in ctx.need_mapping
        ):
            skip_need_defines = True
            continue

        if is_base_include:
            seen_base_include = True
            while output_lines and not output_lines[-1].strip():
                output_lines.pop()

            if output_lines and output_lines[-1].strip():
                output_lines.append("\n")

            prelude_text = bundle.prelude_defines.rstrip("\n")
            if prelude_text:
                output_lines.append(prelude_text)
                output_lines.append("\n\n")

            preamble_text = bundle.preamble_content.rstrip("\n")
            if preamble_text:
                output_lines.append(preamble_text)
                output_lines.append("\n\n")

            for section in bundle.template_sections:
                section_text = section.rstrip("\n")
                if not section_text:
                    continue
                output_lines.append(section_text)
                output_lines.append("\n\n")
            if should_emit_std_namespace and not defer_std_namespace_to_solver_section:
                output_lines.append("using namespace std;\n")
                should_emit_std_namespace = False
            skip_blank_after_sections = True
            continue

        if include_name:
            include_target = resolve_project_include(
                ctx.project_root,
                ctx.source_file,
                include_name,
            )
            if include_target:
                rel = include_target.relative_to(ctx.project_root).as_posix()
                if rel.startswith(INLINE_ROOT_PREFIXES):
                    inlined = inline_local_header(
                        ctx.project_root,
                        include_target,
                        inlined_headers,
                        used_identifiers=set(ctx.used_identifiers),
                        module_leaf_tokens=ctx.module_leaf_tokens,
                        strip_module_docs=ctx.strip_module_docs,
                        strip_template_docs=ctx.strip_template_docs,
                        macro_values=bundle.effective_macro_values,
                        enable_module_pruning=enable_module_pruning,
                    )
                    if inlined:
                        if rel.startswith("modules/") and not module_section_emitted:
                            append_with_blank_separator(output_lines, MODULE_SECTION_SEPARATOR)
                            module_section_emitted = True
                        append_with_blank_separator(output_lines, inlined)
                    continue

        if skip_need_defines and stripped and not stripped.startswith("#define NEED_"):
            skip_need_defines = False
            if output_lines and output_lines[-1].strip():
                output_lines.append("\n")

        output_lines.append(line)
        if stripped == MAIN_SOLVER_SECTION_MARKER and defer_std_namespace_to_solver_section:
            output_lines.append("\nusing namespace std;\n")
            should_emit_std_namespace = False
            defer_std_namespace_to_solver_section = False

    return "".join(output_lines)


def build_flattened_output(
    ctx: FlattenContext,
    *,
    enable_template_pruning: bool,
    enable_module_pruning: bool,
) -> str:
    """Build flattened output through explicit dependency/render/rewrite stages."""

    bundle = _render_template_bundle(
        ctx,
        enable_template_pruning=enable_template_pruning,
        enable_module_pruning=enable_module_pruning,
    )
    output = _render_flattened_source(
        ctx,
        bundle,
        enable_module_pruning=enable_module_pruning,
    )
    _warn_on_surviving_project_includes(output)
    return output


def _warn_on_surviving_project_includes(output: str) -> None:
    """Warn loudly if a project-local include leaked into the final output.

    A surviving ``#include "templates/…"``/``"modules/…"`` means a header was
    neither inlined nor folded away; it will not resolve on an online judge.
    Emitted to stderr rather than raised so a contest run still produces
    output, but tests treat any occurrence as a regression.
    """

    leaked = [
        line.strip() for line in output.splitlines() if SURVIVING_PROJECT_INCLUDE_RE.match(line)
    ]
    if leaked:
        sys.stderr.write(
            "warning: project-local include survived flattening (will not "
            "resolve on a judge): " + ", ".join(sorted(set(leaked))) + "\n"
        )
