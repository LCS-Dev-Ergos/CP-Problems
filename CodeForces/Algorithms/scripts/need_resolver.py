#!/usr/bin/env python3
"""Parse ``NEED_*`` macros from user source and load the canonical mapping.

``load_need_mapping`` reads the canonical feature manifest from
``templates/profiles.toml`` and returns the authoritative
``NEED_<feature> -> [template headers...]`` table that the flattener uses to
decide which template sections to emit. A parser fallback remains for tests
that exercise legacy ``Base.hpp`` snippets directly.

``extract_need_macros_from_source`` walks a user file and reports which
``NEED_*`` macros are actually enabled (honoring ``#define``/``#undef``,
simple conditional folding, and the per-IO profile guards). Used by both
``flattener_pipeline.pipeline`` and ``flattener_pipeline.cli``.
"""

from __future__ import annotations

import re
from collections import OrderedDict
from collections.abc import Iterable
from pathlib import Path
from typing import TextIO

from flattener_core.includes import (
    extract_prefix_before_base_include,
    parse_project_include_line,
)
from flattener_core.lexer import strip_comments, strip_non_code
from flattener_core.preprocessor import fold_simple_preprocessor_conditionals
from profile_registry import TEMPLATES_DIR, load_registry

NEED_IFDEF_RE = re.compile(r"^\s*#\s*ifdef\s+(NEED_\w+)\s*$")
NEED_IF_DEFINED_RE = re.compile(r"^\s*#\s*if\s+defined(?:\s*\(\s*|\s+)(NEED_\w+)\s*\)?\s*$")
IF_DIRECTIVE_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef)\b")
ENDIF_DIRECTIVE_RE = re.compile(r"^\s*#\s*endif\b")
ELSE_OR_ELIF_DIRECTIVE_RE = re.compile(r"^\s*#\s*(else|elif)\b")
NEED_DEFINE_RE = re.compile(r"^\s*#\s*define\s+(NEED_\w+)\b")
NEED_UNDEF_RE = re.compile(r"^\s*#\s*undef\s+(NEED_\w+)\b")
IO_PROFILE_DEFINE_RE = re.compile(r"^\s*#\s*define\s+(CP_IO_PROFILE_[A-Z_]+)\b")
IO_PROFILE_UNDEF_RE = re.compile(r"^\s*#\s*undef\s+(CP_IO_PROFILE_[A-Z_]+)\b")


def _parse_need_mapping_from_header(base_header: Path) -> OrderedDict[str, list[str]]:
    """Parse a C++ header and reconstruct ``NEED_*`` guarded include blocks.

    This is retained as a narrow fallback for unit tests and non-canonical
    temporary headers. The production mapping comes from ``profiles.toml``.
    Mapping order follows macro declaration order in the parsed header.
    Header order follows include order within each NEED_* block.
    """

    mapping: OrderedDict[str, list[str]] = OrderedDict()
    if not base_header.is_file():
        return mapping

    # Stack stores the currently-active NEED_* macro at each conditional depth.
    active_stack: list[str | None] = [None]
    frame_kind_stack: list[str] = ["root"]

    with open(base_header, encoding="utf-8") as f:
        file_text = f.read()

    stripped_lines = strip_comments(file_text).splitlines()
    masked_lines = strip_non_code(file_text).splitlines()
    for idx, raw_line in enumerate(stripped_lines):
        masked_line = masked_lines[idx] if idx < len(masked_lines) else ""
        stripped = masked_line.strip()

        need_match = NEED_IFDEF_RE.match(stripped) or NEED_IF_DEFINED_RE.match(stripped)
        if need_match:
            macro = need_match.group(1)
            mapping.setdefault(macro, [])
            active_stack.append(macro)
            frame_kind_stack.append("need")
            continue

        if IF_DIRECTIVE_RE.match(stripped):
            # Nested non-NEED conditionals inherit outer NEED context.
            active_stack.append(active_stack[-1])
            frame_kind_stack.append("other")
            continue

        if ELSE_OR_ELIF_DIRECTIVE_RE.match(stripped):
            # Alternate branch of the NEED guard should not contribute to
            # the NEED_* mapping.
            if len(active_stack) > 1 and frame_kind_stack[-1] == "need":
                active_stack[-1] = None
            continue

        if ENDIF_DIRECTIVE_RE.match(stripped):
            if len(active_stack) > 1:
                active_stack.pop()
                frame_kind_stack.pop()
            continue

        include_name_raw = parse_project_include_line(raw_line, masked_line=masked_line)
        if include_name_raw and active_stack[-1] is not None:
            include_name = include_name_raw
            if include_name.startswith("templates/"):
                include_name = include_name[len("templates/") :]
            entries = mapping[active_stack[-1]]
            if include_name not in entries:
                entries.append(include_name)

    return mapping


def load_need_mapping(base_header: Path) -> OrderedDict[str, list[str]]:
    """Return the canonical ``NEED_*`` to header mapping for the flattener.

    When called for the repository's real ``templates/Base.hpp``, the mapping
    is loaded from ``profiles.toml``. Other paths use the legacy parser so
    focused parser tests can still pass synthetic headers.
    """

    # ``Path.resolve()`` is non-strict by default and never raises for a
    # missing path, so this comparison is safe on a header that does not exist.
    if base_header.resolve() == (TEMPLATES_DIR / "Base.hpp").resolve():
        return load_registry().feature_headers()
    return _parse_need_mapping_from_header(base_header)


def extract_need_macros_from_source(
    source_content: str,
    known_macros: Iterable[str],
    *,
    warn_stream: TextIO | None = None,
) -> set[str]:
    """Extract enabled NEED_* macros from a source file content.

    NEED_* defines inside conditional blocks the folder could not evaluate are
    silently skipped (the flattener can't know whether they'd fire). When
    ``warn_stream`` is provided, a one-line warning is emitted for each such
    skipped macro.
    """

    known = set(known_macros)
    found: set[str] = set()
    enabled_io_profiles: set[str] = set()
    skipped_needs: list[str] = []

    code_only_prefix = strip_non_code(extract_prefix_before_base_include(source_content))
    # CP_*/NEED_* are flattener-resolved even here, so fold them under the
    # closed-namespace rule (absent ⇒ undefined); the user's own conditionals
    # stay UNKNOWN and are preserved for the depth check below.
    folded_code = fold_simple_preprocessor_conditionals(code_only_prefix, {}, closed_namespace=True)
    depth = 0

    for raw_line in folded_code.splitlines():
        stripped = raw_line.strip()
        if not stripped:
            continue

        if IF_DIRECTIVE_RE.match(stripped):
            depth += 1
            continue

        if ENDIF_DIRECTIVE_RE.match(stripped):
            depth = max(0, depth - 1)
            continue

        if ELSE_OR_ELIF_DIRECTIVE_RE.match(stripped):
            continue

        if depth > 0:
            define_match = NEED_DEFINE_RE.match(stripped)
            if define_match and define_match.group(1) in known:
                skipped_needs.append(define_match.group(1))
            continue

        undef_match = NEED_UNDEF_RE.match(stripped)
        if undef_match:
            macro = undef_match.group(1)
            if macro in known:
                found.discard(macro)
            continue

        profile_undef_match = IO_PROFILE_UNDEF_RE.match(stripped)
        if profile_undef_match:
            enabled_io_profiles.discard(profile_undef_match.group(1))
            continue

        define_match = NEED_DEFINE_RE.match(stripped)
        if define_match:
            macro = define_match.group(1)
            if macro in known:
                found.add(macro)
            continue

        profile_define_match = IO_PROFILE_DEFINE_RE.match(stripped)
        if profile_define_match:
            enabled_io_profiles.add(profile_define_match.group(1))
            continue

    if warn_stream is not None and skipped_needs:
        unique = sorted(set(skipped_needs))
        warn_stream.write(
            "warning: ignoring conditional NEED_* defines (guard unresolved): "
            + ", ".join(unique)
            + "\n"
        )

    registry = load_registry()
    io_needs, _ = registry.expand_io_profiles(enabled_io_profiles)
    found.update(macro for macro in io_needs if macro in known)
    return set(registry.normalize_needs(found))
