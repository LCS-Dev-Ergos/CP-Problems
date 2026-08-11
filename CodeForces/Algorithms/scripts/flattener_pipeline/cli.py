"""Argparse front-end and ``main`` entry point for the flattener CLI.

``_build_arg_parser`` declares the user-facing flags (``--mode``,
``--validate``, ``--strip-docs``, ``--no-prune``, etc.).
``_build_flatten_context`` reads the source file, resolves macro state,
discovers module includes, and constructs the immutable
:class:`FlattenContext` that downstream stages consume.

``main`` dispatches on ``FlattenerMode`` via ``match``: SAFE / COMPACT emit
directly, AUTO retries the safe variant when the compact one fails the
syntax check, SUBMISSION pipes the output through the submission post-pass.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path

from flattener_core.includes import (
    extract_prefix_before_base_include,
    parse_project_include_line,
    resolve_project_include,
)
from flattener_core.lexer import extract_identifiers, strip_non_code
from flattener_core.symbols import collect_module_leaf_trigger_tokens
from flattener_pipeline.context import FlattenContext, FlattenerMode, ValidationStatus
from flattener_pipeline.formatting import reindent_with_clang_format
from flattener_pipeline.macro_resolution import extract_macro_values_from_source
from flattener_pipeline.pipeline import build_flattened_output
from flattener_pipeline.submission import prepare_submission_output
from flattener_pipeline.validation import (
    env_flag_enabled,
    resolve_flattener_mode,
    validate_flattened_output,
)
from need_resolver import load_need_mapping


def _build_arg_parser() -> argparse.ArgumentParser:
    """Construct the argparse parser for the flattener CLI."""

    parser = argparse.ArgumentParser(
        description="Flatten a competitive-programming source file into a single submission.",
    )
    parser.add_argument("source_file", type=Path, help="path to the .cpp to flatten")
    parser.add_argument(
        "--mode",
        choices=[m.value for m in FlattenerMode],
        default=None,
        help="output mode (default: env CP_FLATTENER_MODE or 'compact')",
    )
    parser.add_argument(
        "--validate",
        choices=("auto", "on", "off"),
        default=None,
        help="syntax-check the output (default: env CP_FLATTENER_AUTO_VALIDATE)",
    )
    parser.add_argument(
        "--validation-compiler",
        default=None,
        help="override the compiler used for --validate (default: env or auto-detect)",
    )
    parser.add_argument(
        "--validation-std",
        default=None,
        help="override the -std= flag used for --validate (default: gnu++20)",
    )
    parser.add_argument(
        "--strip-docs",
        action="store_true",
        help="strip template documentation comments (independent of CP_TEMPLATE_PROFILE_STRICT)",
    )
    parser.add_argument(
        "--no-prune",
        action="store_true",
        help="disable template-header tree-shaking (equivalent to safe mode for that axis)",
    )
    parser.add_argument(
        "--no-module-prune",
        action="store_true",
        help="disable per-module-leaf pruning",
    )
    return parser


def _build_flatten_context(args: argparse.Namespace) -> tuple[FlattenContext, FlattenerMode, bool]:
    """Resolve files, macros, modes from CLI args; return ``(ctx, mode, enable_module_pruning)``."""

    source_file: Path = args.source_file
    if not source_file.is_file():
        sys.stderr.write(f"Error: File not found: {source_file}\n")
        raise SystemExit(1)

    script_dir = Path(__file__).resolve().parents[1]
    project_root = script_dir.parent
    templates_dir = project_root / "templates"
    base_template_path = templates_dir / "Base.hpp"
    preamble_path = templates_dir / "core" / "Preamble.hpp"

    source_content = source_file.read_text(encoding="utf-8")
    source_lines = tuple(source_content.splitlines(keepends=True))
    source_masked_lines = tuple(strip_non_code(source_content).splitlines())

    source_prefix = extract_prefix_before_base_include(source_content)
    strict_profile_enabled = bool(
        re.search(
            r"^\s*#\s*define\s+CP_TEMPLATE_PROFILE_STRICT\b",
            source_prefix,
            re.MULTILINE,
        )
    )
    macro_values = extract_macro_values_from_source(
        source_prefix,
        strict_profile_enabled=strict_profile_enabled,
        warn_stream=sys.stderr,
    )

    used_identifiers = frozenset(extract_identifiers(source_content))

    # Flattened output is always a standalone judge submission (never the
    # PCH/workspace build), so enable architecture-target pragmas; they stay
    # auto-guarded by the __x86_64__/__aarch64__ checks in core/Compiler.hpp.
    # The profiles.toml default stays 0 to keep PCH/.gch builds mismatch-free,
    # and that default is already merged into ``macro_values`` above, so a plain
    # setdefault would be a no-op. Force the flattener default on instead, but
    # honor an explicit pin in the user's own source prefix.
    if not re.search(
        r"^\s*#\s*define\s+CP_ENABLE_ARCH_TARGET_PRAGMAS\b",
        source_prefix,
        re.MULTILINE,
    ):
        macro_values["CP_ENABLE_ARCH_TARGET_PRAGMAS"] = 1

    strip_module_docs = env_flag_enabled("CP_FLATTENER_STRIP_MODULE_DOCS")
    strip_template_docs = args.strip_docs or env_flag_enabled("CP_FLATTENER_STRIP_TEMPLATE_DOCS")

    mode = FlattenerMode(args.mode) if args.mode else resolve_flattener_mode()
    if args.no_prune:
        mode = FlattenerMode.SAFE
    if mode is FlattenerMode.SUBMISSION:
        macro_values["CP_USE_BITS_HEADER"] = 1
        strip_module_docs = True
        strip_template_docs = True

    enable_module_pruning = not args.no_module_prune and mode is not FlattenerMode.SAFE

    source_module_includes: list[Path] = []
    has_module_umbrella_include = False
    for idx, line in enumerate(source_lines):
        masked_line = source_masked_lines[idx] if idx < len(source_masked_lines) else ""
        include_name = parse_project_include_line(line, masked_line=masked_line)
        if not include_name:
            continue
        include_target = resolve_project_include(project_root, source_file, include_name)
        if not include_target:
            continue
        rel = include_target.relative_to(project_root).as_posix()
        if not rel.startswith("modules/"):
            continue
        source_module_includes.append(include_target)
        if rel.count("/") == 1:
            has_module_umbrella_include = True

    module_leaf_tokens = (
        collect_module_leaf_trigger_tokens(project_root) if has_module_umbrella_include else None
    )

    need_mapping = load_need_mapping(base_template_path)
    if not need_mapping:
        sys.stderr.write(f"Error: Unable to derive NEED_* mapping from {base_template_path}\n")
        raise SystemExit(1)

    ctx = FlattenContext(
        source_file=source_file,
        source_content=source_content,
        source_lines=source_lines,
        source_masked_lines=source_masked_lines,
        project_root=project_root,
        templates_dir=templates_dir,
        preamble_path=preamble_path,
        preamble_resolved=preamble_path.resolve(),
        need_mapping=need_mapping,
        used_identifiers=used_identifiers,
        source_module_includes=tuple(source_module_includes),
        module_leaf_tokens=module_leaf_tokens,
        strip_module_docs=strip_module_docs,
        strip_template_docs=strip_template_docs,
        macro_values=macro_values,
    )
    return ctx, mode, enable_module_pruning


def main() -> None:
    """Flatten one source file by expanding template and local project includes."""

    args = _build_arg_parser().parse_args()
    ctx, mode, enable_module_pruning = _build_flatten_context(args)

    validation_std = args.validation_std or os.environ.get("CP_FLATTENER_VALIDATION_STD", "gnu++20")
    validation_compiler = args.validation_compiler or os.environ.get(
        "CP_FLATTENER_VALIDATION_COMPILER", ""
    )
    validate_mode = args.validate or "auto"
    force_validation = {"on": True, "off": False, "auto": None}.get(validate_mode)

    def _run_validation(source: str) -> ValidationStatus:
        return validate_flattened_output(
            source,
            force=force_validation,
            validation_compiler=validation_compiler or None,
            validation_std=validation_std,
        )

    match mode:
        case FlattenerMode.SAFE:
            print(
                build_flattened_output(
                    ctx,
                    enable_template_pruning=False,
                    enable_module_pruning=enable_module_pruning,
                ),
                end="",
            )
        case FlattenerMode.COMPACT:
            # clang-format re-indents the directives the folder left dangling and
            # collapses fold-leftover blank runs; comments are kept in this mode.
            print(
                reindent_with_clang_format(
                    build_flattened_output(
                        ctx,
                        enable_template_pruning=True,
                        enable_module_pruning=enable_module_pruning,
                    )
                ),
                end="",
            )
        case FlattenerMode.AUTO:
            compact_output = build_flattened_output(
                ctx,
                enable_template_pruning=True,
                enable_module_pruning=enable_module_pruning,
            )
            # Validation runs on the raw output; the emitted text is re-indented
            # afterwards (clang-format is whitespace-only, so validity holds).
            compact_valid = _run_validation(compact_output)
            if compact_valid is ValidationStatus.FAILED:
                sys.stderr.write(
                    "warning: compact output failed syntax check; trying safe variant\n"
                )
                safe_output = build_flattened_output(
                    ctx,
                    enable_template_pruning=False,
                    enable_module_pruning=enable_module_pruning,
                )
                safe_valid = _run_validation(safe_output)
                if safe_valid is not ValidationStatus.FAILED:
                    print(reindent_with_clang_format(safe_output), end="")
                    return
                sys.stderr.write(
                    "warning: safe variant also failed syntax check; emitting compact output anyway\n"
                )
            print(reindent_with_clang_format(compact_output), end="")
        case FlattenerMode.SUBMISSION:
            submission_output = build_flattened_output(
                ctx,
                enable_template_pruning=True,
                enable_module_pruning=enable_module_pruning,
            )
            print(prepare_submission_output(submission_output), end="")
