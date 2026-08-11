"""Lexical primitives for C++ source manipulation.

Single-pass scanner that understands raw/regular string literals, line and
block comments, and identifier tokens, exposing convenient front-ends for
the three usage modes the flattener needs: ``strip_comments`` (preserve
literals), ``strip_non_code`` (mask everything non-source), and
``mask_code_literals`` (placeholder swap so regex stripping is literal-safe).

Also hosts text-shaping helpers (blank-line normalization, doc-comment
removal) used by the include-inliner and the submission pipeline.
"""

from __future__ import annotations

import re

IDENTIFIER_RE = re.compile(r"\b[A-Za-z_]\w*\b")
STRUCT_CLASS_RE = re.compile(r"^\s*(?:template\s*<[^>]*>\s*)?(?:struct|class)\s+([A-Za-z_]\w*)\b")
USING_RE = re.compile(r"^\s*using\s+([A-Za-z_]\w*)\b")
# ``(?:const\s*)?`` keeps the optional qualifier *inside* a single whitespace
# run. The earlier ``\s*(?:const)?\s*`` spelling accepted the same language but
# left two adjacent ``\s*`` to split a whitespace run between, which backtracks
# quadratically on a long line that ends in spaces without ``{`` (~1.8s for a
# 16 KB line). Matching behavior is unchanged — see test_lexer_func_like_re.
FUNC_LIKE_RE = re.compile(r"([A-Za-z_]\w*)\s*\([^()]*\)\s*(?:const\s*)?(?:\{|$)")

# fmt: off
CPP_KEYWORDS = frozenset({
    "if", "for", "while", "switch", "return", "sizeof", "alignof", "noexcept",
    "static_assert", "catch", "new", "delete", "operator",
})
# fmt: on


def _is_identifier_char(ch: str) -> bool:
    return ch == "_" or ch.isalnum()


def _consume_quoted_literal(text: str, start: int) -> int:
    """Return index right after a C/C++ quoted literal starting at ``start``."""

    n = len(text)
    quote = text[start]
    j = start + 1
    while j < n:
        ch = text[j]
        if ch == "\\" and j + 1 < n:
            j += 2
            continue
        if ch == quote:
            return j + 1
        if ch == "\n":
            return j
        j += 1
    return j


def _consume_raw_string_literal(text: str, start: int) -> int | None:
    """Return index right after a C++ raw string literal, or ``None`` if not at one."""

    prefixes = ('u8R"', 'uR"', 'UR"', 'LR"', 'R"')
    prefix = next((p for p in prefixes if text.startswith(p, start)), None)
    if not prefix:
        return None
    if start > 0 and _is_identifier_char(text[start - 1]):
        return None

    n = len(text)
    delim_start = start + len(prefix)
    j = delim_start
    while j < n and text[j] not in {"(", "\n"}:
        j += 1
    if j >= n or text[j] != "(":
        return None

    delimiter = text[delim_start:j]
    terminator = ")" + delimiter + '"'
    end_pos = text.find(terminator, j + 1)
    if end_pos < 0:
        return n
    return end_pos + len(terminator)


def _consume_string_or_char_literal(text: str, start: int) -> int | None:
    """Return index after a normal string/char literal, including standard prefixes."""

    for prefix in ("u8", "u", "U", "L", ""):
        quote_at = start + len(prefix)
        if quote_at >= len(text):
            continue
        if prefix and not text.startswith(prefix, start):
            continue
        if prefix and start > 0 and _is_identifier_char(text[start - 1]):
            continue
        if text[quote_at] not in {'"', "'"}:
            continue
        return _consume_quoted_literal(text, quote_at)
    return None


def _scan_cpp_text(
    text: str, *, keep_comments: bool, literal_mode: str
) -> tuple[str, dict[str, str]]:
    """Single-pass scanner used by flattener lexical helpers.

    ``literal_mode``:
      - ``"space"``: replace literal payload with spaces/newlines
      - ``"token"``: replace each literal with a stable placeholder token
      - ``"keep"``:  keep literals as-is
    """

    out: list[str] = []
    literals: dict[str, str] = {}
    token_id = 0
    i = 0
    n = len(text)

    while i < n:
        c = text[i]

        raw_end = _consume_raw_string_literal(text, i)
        if raw_end is not None:
            literal = text[i:raw_end]
            if literal_mode == "token":
                token = f"__CP_FLATTENER_LIT_{token_id}__"
                token_id += 1
                literals[token] = literal
                out.append(token)
            elif literal_mode == "space":
                out.extend("\n" if ch == "\n" else " " for ch in literal)
            else:
                out.append(literal)
            i = raw_end
            continue

        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = i + 2
            while j < n and text[j] != "\n":
                j += 1
            if keep_comments:
                out.append(text[i:j])
            i = j
            continue

        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = i + 2
            while j + 1 < n and not (text[j] == "*" and text[j + 1] == "/"):
                j += 1
            j = j + 2 if j + 1 < n else n
            block = text[i:j]
            if keep_comments:
                out.append(block)
            else:
                out.extend("\n" if ch == "\n" else " " for ch in block)
            i = j
            continue

        literal_end = _consume_string_or_char_literal(text, i)
        if literal_end is not None:
            literal = text[i:literal_end]
            if literal_mode == "token":
                token = f"__CP_FLATTENER_LIT_{token_id}__"
                token_id += 1
                literals[token] = literal
                out.append(token)
            elif literal_mode == "space":
                out.extend("\n" if ch == "\n" else " " for ch in literal)
            else:
                out.append(literal)
            i = literal_end
            continue

        out.append(c)
        i += 1

    return "".join(out), literals


def strip_comments(text: str) -> str:
    """Remove comments while preserving literals and line structure."""

    return _scan_cpp_text(text, keep_comments=False, literal_mode="keep")[0]


def strip_non_code(text: str) -> str:
    """Remove strings/comments while preserving line structure."""

    return _scan_cpp_text(text, keep_comments=False, literal_mode="space")[0]


def mask_code_literals(text: str) -> tuple[str, dict[str, str]]:
    """Replace string/char literals with temporary tokens while preserving comments."""

    return _scan_cpp_text(text, keep_comments=True, literal_mode="token")


def restore_code_literals(masked: str, literals: dict[str, str]) -> str:
    """Restore literal tokens emitted by :func:`mask_code_literals`."""

    for token, literal in literals.items():
        masked = masked.replace(token, literal)
    return masked


def extract_identifiers(text: str) -> set[str]:
    """Extract identifier tokens from source text (literals/comments removed)."""

    return set(IDENTIFIER_RE.findall(strip_non_code(text)))


def append_with_blank_separator(chunks: list[str], content: str) -> None:
    """Append a chunk ensuring exactly one blank line of separation."""

    if not content:
        return

    if chunks:
        last = chunks[-1]
        if not last.endswith("\n"):
            chunks.append("\n")
            last = "\n"
        if not last.endswith("\n\n"):
            chunks.append("\n")

    chunks.append(content)
    if not content.endswith("\n"):
        chunks.append("\n")


def trim_outer_blank_lines(content: str) -> str:
    """Trim only leading/trailing blank lines, keeping internal spacing."""

    lines = content.splitlines()
    while lines and not lines[0].strip():
        lines.pop(0)
    while lines and not lines[-1].strip():
        lines.pop()
    return "\n".join(lines)


def collapse_redundant_blank_lines(content: str) -> str:
    """Collapse consecutive blank lines to at most one empty line."""

    lines = content.splitlines()
    normalized: list[str] = []
    prev_blank = False
    for line in lines:
        if line.strip() == "":
            if not prev_blank:
                normalized.append("")
            prev_blank = True
            continue

        normalized.append(line.rstrip())
        prev_blank = False

    return "\n".join(normalized)


def strip_module_docs_and_blank_lines(content: str) -> str:
    """Strip documentation comments from already-inlined module sections.

    Removes ``/** ... */``, ``/*! ... */``, ``///``, ``//!`` and trailing
    ``///<`` / ``//!<`` forms while preserving regular comments.
    """

    masked, literals = mask_code_literals(content)

    def _doc_block_replacer(match: re.Match[str]) -> str:
        block = match.group(0)
        newline_count = block.count("\n")
        return "\n" * newline_count if newline_count else " "

    masked = re.sub(
        r"/\*(?:\*|!).*?\*/",
        _doc_block_replacer,
        masked,
        flags=re.DOTALL,
    )
    masked = re.sub(r"(?m)^[ \t]*///.*$", "", masked)
    masked = re.sub(r"(?m)^[ \t]*//!.*$", "", masked)
    masked = re.sub(r"(?m)[ \t]*//[/!]<.*$", "", masked)

    content = restore_code_literals(masked, literals)

    lines = content.splitlines()
    normalized: list[str] = []
    prev_blank = False
    for line in lines:
        stripped = line.strip()
        if stripped == "":
            if not prev_blank:
                normalized.append("")
            prev_blank = True
        else:
            normalized.append(line.rstrip())
            prev_blank = False
    return trim_outer_blank_lines("\n".join(normalized))
