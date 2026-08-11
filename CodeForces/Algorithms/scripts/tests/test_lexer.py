"""Unit tests for the lexical primitives in :mod:`flattener_core.lexer`.

Focuses on ``FUNC_LIKE_RE``, the declaration-shaped pattern
:mod:`flattener_core.symbols` runs against every source line. The regex is
applied per line, so a single long line is enough to make a superlinear
pattern hurt — the timing guard below pins that it stays linear.
"""

from __future__ import annotations

import time

from flattener_core.lexer import FUNC_LIKE_RE


def _matched(line: str) -> str | None:
    match = FUNC_LIKE_RE.search(line)
    return match.group(0) if match else None


# ------------------------------ Accepted forms ------------------------------ #


def test_matches_plain_definition():
    assert _matched("void f() {") is not None
    assert _matched("void f(){") is not None


def test_matches_const_qualifier_with_and_without_spacing():
    """``) const {`` and the unspaced ``)const{`` are both real C++."""

    assert _matched("int g() const {") is not None
    assert _matched("int g()const{") is not None
    assert _matched("int g() const  {") is not None


def test_matches_declaration_at_end_of_line():
    assert _matched("auto h(int a, int b) const") is not None
    assert _matched("T m()") is not None


def test_captures_the_callee_name():
    match = FUNC_LIKE_RE.search("inline I64 div_floor(I64 a, I64 b) {")
    assert match is not None
    assert match.group(1) == "div_floor"


# ------------------------------ Rejected forms ------------------------------ #


def test_does_not_match_trailing_garbage_after_call():
    """A call followed by more code is not a declaration head."""

    assert _matched("f()  X") is None


def test_does_not_match_nested_parentheses():
    """``[^()]*`` deliberately refuses to span a nested group."""

    assert _matched("g(h(1)) {") is None


# ------------------------------- Complexity --------------------------------- #


def test_long_whitespace_run_stays_linear():
    """Guard against the quadratic ``\\s*(?:const)?\\s*`` spelling regressing.

    The failing input is a long whitespace run with no ``{`` to close on, so
    the engine must reject every split point. With the old pattern this cost
    ~1.8s at 16 KB; linear behavior keeps it in single-digit milliseconds.
    """

    line = "f()" + " " * 16000 + "X"

    start = time.perf_counter()
    assert _matched(line) is None
    elapsed = time.perf_counter() - start

    assert elapsed < 0.25, f"FUNC_LIKE_RE took {elapsed:.3f}s on a 16 KB line (expected linear)"
