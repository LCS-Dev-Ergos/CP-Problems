"""Shared clang-format re-indentation pass for flattened output.

The conditional folder removes ``#if``/``#endif`` lines and leaves their bodies
carrying the indentation of the now-absent nesting. Every mode that emits a
final artifact (compact, auto, submission) runs the output through clang-format
to restore correct directive indentation and collapse the redundant blank runs
the fold can leave behind.

Best-effort by design: if clang-format is missing or errors, the (still valid)
unformatted source is returned so flattening never breaks on a bare machine.
"""

from __future__ import annotations

import shutil
import sys

from _lib.process import ProcessRequest, run_capture
from flattener_core.lexer import is_section_banner, mask_code_literals_preserving_lines

# Self-contained, gentle style for the generated artifact. The repo's own
# ``.clang-format`` is ``DisableFormat: true`` (sources are hand-laid-out), so
# the flattened output cannot use ``-style=file`` and carries its own policy
# instead: re-indent preprocessor directives by nesting depth (the actual fix)
# without reflowing the crafted ``constexpr`` tables / long lines
# (``ColumnLimit: 0``) or reordering the ordered template includes
# (``SortIncludes: false``); ``MaxEmptyLinesToKeep: 1`` collapses fold leftovers.
_CLANG_FORMAT_STYLE = (
    "{BasedOnStyle: LLVM, IndentWidth: 2, ColumnLimit: 0, "
    "IndentPPDirectives: BeforeHash, SortIncludes: false, MaxEmptyLinesToKeep: 1, "
    "AlignConsecutiveAssignments: true, PointerAlignment: Left }"
)


def normalize_section_banner_spacing(source: str) -> str:
    """Keep one blank line around every canonical section banner."""

    lines = source.splitlines()
    literal_masked_lines = mask_code_literals_preserving_lines(source).splitlines()
    normalized: list[str] = []
    after_banner = False
    for idx, line in enumerate(lines):
        is_banner = (
            is_section_banner(line)
            and idx < len(literal_masked_lines)
            and line == literal_masked_lines[idx]
        )
        if is_banner:
            while normalized and not normalized[-1].strip():
                normalized.pop()
            if normalized:
                normalized.append("")
            normalized.append(line)
            after_banner = True
            continue
        if after_banner:
            if not line.strip():
                continue
            normalized.append("")
            after_banner = False
        normalized.append(line)

    output = "\n".join(normalized)
    return output + "\n" if source.endswith("\n") else output


def reindent_with_clang_format(source: str) -> str:
    """Re-indent ``source`` via clang-format and normalize section banners."""

    executable = shutil.which("clang-format")
    if executable is None:
        return normalize_section_banner_spacing(source)
    result = run_capture(
        ProcessRequest(
            argv=[
                executable,
                f"-style={_CLANG_FORMAT_STYLE}",
                "--assume-filename=submission.cpp",
            ],
            stdin=source,
            timeout=30.0,
        )
    )
    if result.failed or not result.stdout:
        sys.stderr.write(
            "warning: clang-format failed, emitting unformatted output "
            f"(exit {result.returncode})\n"
        )
        return normalize_section_banner_spacing(source)
    return normalize_section_banner_spacing(result.stdout)
