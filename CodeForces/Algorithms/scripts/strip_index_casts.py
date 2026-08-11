#!/usr/bin/env python3
"""Strip redundant index/size casts from the CP library modules.

Scope: CodeForces/Algorithms/

Transformations (applied iteratively until fix-point):
  1. as<Size>(EXPR)           -> EXPR        if EXPR is a "simple" token
                              -> (EXPR)      otherwise (preserve precedence)
  2. as<I32>(EXPR.size())     -> isz(EXPR)
  3. as<I64>(EXPR.size())     -> sz(EXPR)

Other as<...> casts (as<I128>, as<__int128>, as<Calc>, as<U>, as<F80>,
as<U32>, as<U64>, as<I32>/as<I64> NOT followed by .size(), ...) are left
untouched — those are legitimate per the cast-audit policy.

Usage:
  python3 scripts/strip_index_casts.py --dry-run   # show unified diffs
  python3 scripts/strip_index_casts.py             # write changes in place
"""

from __future__ import annotations

import argparse
import difflib
import re
import sys
from fnmatch import fnmatch
from pathlib import Path

from _lib.cli import run_cli

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SCOPE = REPO_ROOT / "modules"

SIMPLE_EXPR = re.compile(
    r"""
    ^
    [A-Za-z_][\w]*       # leading identifier
    (?:\.[A-Za-z_]\w*)*  # optional dotted member access
    (?:\([^()]*\))?      # optional flat function-call args
    (?:\[[^\[\]]*\])?    # optional flat subscript
    $
    """,
    re.VERBOSE,
)

TARGET_KINDS = ("Size", "I32", "I64")


def find_matching_paren(text: str, open_idx: int) -> int:
    """Given text[open_idx] == '(', return index of the matching ')'.

    Returns -1 if unbalanced. Naive: does NOT skip over string/char literals
    or comments. The codebase under SCOPE_DIR contains no occurrences of
    `as<Size>(`, `as<I32>(`, `as<I64>(` inside literals or comments
    (verified via rg), so this is safe in practice.
    """
    assert text[open_idx] == "("
    depth = 0
    i = open_idx
    n = len(text)
    while i < n:
        c = text[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def is_word_char(c: str) -> bool:
    """Return True if c is a letter, digit, or underscore (i.e. valid in C++ identifiers)."""

    return c.isalnum() or c == "_"


def try_match_cast(text: str, i: int) -> tuple[str, int] | None:
    """If text[i:] starts with `as<KIND>(` for a tracked KIND, return (kind, '(' index).

    Returns None otherwise. Also enforces a left word-boundary so we do not
    match e.g. `xyz_as<Size>(...)`.
    """
    if i > 0 and is_word_char(text[i - 1]):
        return None
    for kind in TARGET_KINDS:
        needle = f"as<{kind}>("
        if text.startswith(needle, i):
            return kind, i + len(needle) - 1
    return None


def render_size(inner: str) -> str:
    stripped = inner.strip()
    if SIMPLE_EXPR.match(stripped):
        return stripped
    return f"({stripped})"


def render_signed_size(kind: str, inner: str) -> str | None:
    """For as<I32>/as<I64>: rewrite only if inner is `EXPR.size()`.

    Returns the replacement string, or None if the inner does not match the
    `.size()` form (in which case the original cast should be preserved).
    """
    stripped = inner.strip()
    if not stripped.endswith(".size()"):
        return None
    expr = stripped[: -len(".size()")].rstrip()
    helper = "isz" if kind == "I32" else "sz"
    if SIMPLE_EXPR.match(expr):
        return f"{helper}({expr})"
    return f"{helper}(({expr}))"


def transform_pass(text: str) -> str:
    """Apply one pass of the transformations. Returns the updated text."""

    out: list[str] = []
    i = 0
    n = len(text)
    while i < n:
        m = try_match_cast(text, i)
        if m is None:
            out.append(text[i])
            i += 1
            continue
        kind, open_idx = m
        close_idx = find_matching_paren(text, open_idx)
        if close_idx < 0:
            out.append(text[i])
            i += 1
            continue
        inner = text[open_idx + 1 : close_idx]
        if kind == "Size":
            out.append(render_size(inner))
            i = close_idx + 1
        else:
            repl = render_signed_size(kind, inner)
            if repl is None:
                out.append(text[i])
                i += 1
            else:
                out.append(repl)
                i = close_idx + 1
    return "".join(out)


def transform(text: str, max_passes: int = 8) -> str:
    """Apply transformations iteratively until fix-point or max_passes reached."""

    for _ in range(max_passes):
        new = transform_pass(text)
        if new == text:
            return new
        text = new
    return text


def strip_subscript_parens(text: str) -> str:
    """Collapse `[(EXPR)]` -> `[EXPR]` when EXPR has no top-level comma.

    Inside an array subscript, `(...)` is never needed for precedence — the
    `[` and `]` are syntactic delimiters. The only concern is the comma
    operator (`[(a, b)]` evaluates both and yields `b`), which we explicitly
    skip. Pre-existing parens whose closing `)` is NOT immediately before `]`
    (e.g. `vertices[(i + 1) % n]`) are correctly left alone, because we
    strip only when the matched `)` is followed by `]`.
    """
    out: list[str] = []
    i = 0
    n = len(text)
    while i < n:
        if text[i] == "[" and i + 1 < n and text[i + 1] == "(":
            open_idx = i + 1
            close_idx = find_matching_paren(text, open_idx)
            if close_idx > 0 and close_idx + 1 < n and text[close_idx + 1] == "]":
                inner = text[open_idx + 1 : close_idx]
                if not _has_top_level_comma(inner):
                    out.append("[")
                    out.append(inner)
                    out.append("]")
                    i = close_idx + 2
                    continue
        out.append(text[i])
        i += 1
    return "".join(out)


def _has_top_level_comma(expr: str) -> bool:
    """Return True if expr contains a comma at the top level (not nested in parens/brackets/angles)."""

    depth_paren = depth_brack = depth_angle = 0
    for c in expr:
        if c == "(":
            depth_paren += 1
        elif c == ")":
            depth_paren -= 1
        elif c == "[":
            depth_brack += 1
        elif c == "]":
            depth_brack -= 1
        elif c == "<":
            depth_angle += 1
        elif c == ">":
            depth_angle -= 1
        elif c == "," and depth_paren == 0 and depth_brack == 0 and depth_angle <= 0:
            return True
    return False


def collect_files(scope: Path, exclude_globs: list[str]) -> list[Path]:
    """Collect .hpp files under scope, excluding paths matching any of exclude_globs."""

    files = sorted(scope.rglob("*.hpp"))
    if exclude_globs:
        files = [f for f in files if not any(fnmatch(str(f), pat) for pat in exclude_globs)]
    return files


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--dry-run", action="store_true", help="show diffs only")
    parser.add_argument(
        "--scope",
        default=str(DEFAULT_SCOPE),
        help="directory to walk for .hpp files (default: modules)",
    )
    parser.add_argument(
        "--only",
        action="append",
        default=[],
        metavar="GLOB",
        help="restrict to file paths matching GLOB (repeatable)",
    )
    parser.add_argument(
        "--exclude",
        action="append",
        default=[],
        metavar="GLOB",
        help="skip file paths matching GLOB (repeatable)",
    )
    parser.add_argument(
        "--mode",
        choices=("casts", "parens", "both"),
        default="casts",
        help="casts: strip as<Size>/as<I32|I64>(.size()); "
        "parens: cleanup [(EXPR)] -> [EXPR]; "
        "both: run casts then parens",
    )
    args = parser.parse_args()

    scope = Path(args.scope).resolve()
    files = collect_files(scope, args.exclude)
    if args.only:
        files = [f for f in files if any(fnmatch(str(f), pat) for pat in args.only)]

    if not files:
        print(f"no files in scope: {scope}", file=sys.stderr)
        return 1

    def run_transform(text: str) -> str:
        if args.mode == "casts":
            return transform(text)
        if args.mode == "parens":
            return strip_subscript_parens(text)
        return strip_subscript_parens(transform(text))

    total_files = 0
    total_cast_delta = 0
    total_paren_delta = 0
    for path in files:
        original = path.read_text(encoding="utf-8")
        updated = run_transform(original)
        if updated == original:
            continue
        total_files += 1
        cast_delta = (
            original.count("as<Size>(")
            - updated.count("as<Size>(")
            + sum(original.count(f"as<{k}>(") - updated.count(f"as<{k}>(") for k in ("I32", "I64"))
        )
        paren_delta = original.count("[(") - updated.count("[(")
        total_cast_delta += cast_delta
        total_paren_delta += paren_delta
        try:
            rel = path.relative_to(REPO_ROOT.parent)
        except ValueError:
            rel = path
        if args.dry_run:
            diff = difflib.unified_diff(
                original.splitlines(keepends=True),
                updated.splitlines(keepends=True),
                fromfile=f"a/{rel}",
                tofile=f"b/{rel}",
            )
            sys.stdout.writelines(diff)
        else:
            path.write_text(updated, encoding="utf-8")
            tag = []
            if cast_delta:
                tag.append(f"-{cast_delta} casts")
            if paren_delta:
                tag.append(f"-{paren_delta} parens")
            print(f"updated {rel}  ({', '.join(tag) or 'no-op'})")

    summary = f"~{total_cast_delta} casts, ~{total_paren_delta} parens"
    prefix = "[dry-run] " if args.dry_run else ""
    print(
        f"\n{prefix}{total_files} files {'would change' if args.dry_run else 'modified'}, {summary}",
        file=sys.stderr,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(run_cli(main))
