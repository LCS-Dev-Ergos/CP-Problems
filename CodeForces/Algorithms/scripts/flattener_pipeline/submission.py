"""Submission-mode post-pass: strip comments, collapse blanks, re-indent.

Used only by ``FlattenerMode.SUBMISSION`` to turn the flattened source into the
artifact pasted to an online judge: comments are stripped and blank runs
collapsed to fit size limits, then the shared clang-format pass re-indents the
result (see :mod:`flattener_pipeline.formatting`). Compact/auto modes share the
same re-indent step but keep comments.
"""

from __future__ import annotations

from flattener_core.lexer import (
    collapse_redundant_blank_lines,
    is_section_banner,
    mask_code_literals_preserving_lines,
    strip_comments,
    trim_outer_blank_lines,
)
from flattener_pipeline.formatting import (
    normalize_section_banner_spacing,
    reindent_with_clang_format,
)


def _protect_section_banners(source: str) -> tuple[str, dict[str, str]]:
    """Replace canonical banners with format-safe sentinels."""

    banners: dict[str, str] = {}
    prefix = "__CP_SECTION_BANNER_SENTINEL_"
    while prefix in source:
        prefix += "_"

    literal_masked_lines = mask_code_literals_preserving_lines(source).splitlines()
    protected: list[str] = []
    for idx, source_line in enumerate(source.splitlines(keepends=True)):
        line = source_line.rstrip("\r\n")
        if (
            is_section_banner(line)
            and idx < len(literal_masked_lines)
            and line == literal_masked_lines[idx]
        ):
            token = f"{prefix}{len(banners)}__"
            banners[token] = line
            protected.append(f'static_assert(true, "{token}");{source_line[len(line) :]}')
            continue
        protected.append(source_line)
    return "".join(protected), banners


def _restore_section_banners(source: str, banners: dict[str, str]) -> str:
    """Restore canonical banners after formatting."""

    for token, banner in banners.items():
        source = source.replace(f'static_assert(true, "{token}");', banner)
    return source


def prepare_submission_output(flattened_source: str) -> str:
    """Minimize and re-indent flattened source for online judge submission."""

    protected, banners = _protect_section_banners(flattened_source)
    stripped = strip_comments(protected)
    compacted = collapse_redundant_blank_lines(stripped)
    trimmed = trim_outer_blank_lines(compacted) + "\n"
    return normalize_section_banner_spacing(
        _restore_section_banners(reindent_with_clang_format(trimmed), banners)
    )
