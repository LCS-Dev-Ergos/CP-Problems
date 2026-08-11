"""Unit tests for the three-valued ``#if`` partial evaluator and the folder.

Locks in the engine that decides which conditional regions reach a submission:
:mod:`flattener_core.ppexpr` (Kleene logic, the closed-namespace authority
rule, ``defined``/``__has_include`` handling) and the recursive
:func:`flattener_core.preprocessor.fold_simple_preprocessor_conditionals`
(nesting, ``#elif``/``#else`` chains, and verbatim preservation of
compiler-dependent guards while inner known config still folds).
"""

from __future__ import annotations

import textwrap

from flattener_core.macros import is_authoritative_macro
from flattener_core.ppexpr import (
    evaluate_expression,
    evaluate_simple_if_expression,
    is_macro_defined,
)
from flattener_core.preprocessor import fold_simple_preprocessor_conditionals


def ev(expr, state=None, *, closed=False):
    return evaluate_simple_if_expression(expr, state or {}, closed_namespace=closed)


# --------------------------- Expression evaluator --------------------------- #


def test_literals_and_unary():
    assert ev("1") is True
    assert ev("0") is False
    assert ev("!0") is True
    assert ev("!1") is False
    assert ev("-1") is True  # non-zero
    assert ev("0x10") is True


def test_comparisons_and_arithmetic():
    assert ev("2 == 2") is True
    assert ev("2 != 2") is False
    assert ev("3 > 2") is True
    assert ev("1 >= 2") is False
    assert ev("1 + 1") is True
    assert ev("2 - 2") is False
    assert ev("CP_V == 1", {"CP_V": 1}) is True
    assert ev("CP_V == 1", {"CP_V": 2}) is False


def test_division_truncates_toward_zero_like_c():
    """C integer division truncates; Python's ``//`` floors. Follow C.

    ``#if`` arithmetic is evaluated with C semantics, so ``-7 / 2`` is -3 and
    ``-7 % 2`` is -1. Getting this wrong folds a guard to the wrong arm with
    full confidence, which is worse than returning UNKNOWN.
    """

    assert evaluate_expression("-7 / 2", {}) == -3
    assert evaluate_expression("7 / -2", {}) == -3
    assert evaluate_expression("-7 / -2", {}) == 3
    assert evaluate_expression("7 / 2", {}) == 3
    assert evaluate_expression("-6 / 2", {}) == -3
    assert evaluate_expression("-1 / 2", {}) == 0


def test_remainder_takes_the_sign_of_the_dividend_like_c():
    """C's ``%`` follows the dividend; Python's follows the divisor."""

    assert evaluate_expression("-7 % 2", {}) == -1
    assert evaluate_expression("7 % -2", {}) == 1
    assert evaluate_expression("-7 % -2", {}) == -1
    assert evaluate_expression("7 % 2", {}) == 1
    assert evaluate_expression("-6 % 2", {}) == 0
    assert evaluate_expression("-1 % 2", {}) == -1


def test_division_by_zero_stays_unknown():
    assert evaluate_expression("1 / 0", {}) is None
    assert evaluate_expression("1 % 0", {}) is None


def test_known_and_opaque_macros():
    assert ev("X", {"X": 5}) is True
    assert ev("X", {"X": 0}) is False
    # Defined-but-opaque (value None): truthy in #if is undecidable.
    assert ev("X", {"X": None}) is None


def test_kleene_short_circuit():
    # FALSE dominates &&, TRUE dominates ||, even with an unknown operand.
    assert ev("0 && FOO") is False
    assert ev("1 && FOO") is None
    assert ev("FOO && BAR") is None
    assert ev("1 || FOO") is True
    assert ev("0 || FOO") is None
    assert ev("(1 || 0) && 1") is True
    assert ev("!(0)") is True


def test_defined_operator():
    assert ev("defined(X)", {"X": 0}) is True  # defined even when zero
    assert ev("defined X", {"X": 1}) is True
    assert ev("defined(MISSING)") is None  # conservative (open) mode
    assert ev("!defined(MISSING)") is None


def test_closed_namespace_authority():
    # Absent CP_*/NEED_* are known-undefined under the closed-namespace rule.
    assert ev("CP_X", closed=True) is False
    assert ev("defined(CP_X)", closed=True) is False
    assert ev("!defined(NEED_Y)", closed=True) is True
    assert ev("CP_X || 1", closed=True) is True
    # Toolchain macros stay UNKNOWN even in closed mode → block preserved.
    assert ev("HAS_INT128", closed=True) is None
    assert ev("defined(__GNUC__)", closed=True) is None


def test_has_include_is_unknown():
    assert ev("defined(__has_include) && __has_include(<ext/pb_ds/x.hpp>)") is None
    assert ev("__has_include(<vector>)", closed=True) is None


def test_unparseable_is_unknown():
    # A line-continuation / unmodeled form must degrade to UNKNOWN, not raise.
    assert ev("defined(__has_include) && \\") is None


def test_is_macro_defined_and_authority():
    assert is_macro_defined("X", {"X": 0}) is True
    assert is_macro_defined("CP_X", {}, closed_namespace=True) is False
    assert is_macro_defined("__GNUC__", {}, closed_namespace=True) is None
    assert is_macro_defined("CP_X", {}) is None  # open mode
    assert is_authoritative_macro("CP_FOO") and is_authoritative_macro("NEED_BAR")
    assert not is_authoritative_macro("HAS_INT128")
    assert not is_authoritative_macro("__GNUC__")


# --------------------------------------------------------------------------- #
# Recursive folder
# --------------------------------------------------------------------------- #


def fold(src, state=None, *, closed=False):
    return fold_simple_preprocessor_conditionals(
        textwrap.dedent(src), state or {}, closed_namespace=closed
    )


def test_fold_nested_decided_blocks():
    out = fold(
        """\
        #if 1
        KEEP
          #if 0
        DROP
          #endif
        #endif
        """
    )
    assert "KEEP" in out
    assert "DROP" not in out
    assert "#if" not in out and "#endif" not in out


def test_fold_elif_chain_selects_true_arm():
    out = fold(
        """\
        #if 0
        A
        #elif 1
        B
        #else
        C
        #endif
        """
    )
    assert "B" in out
    assert "A" not in out and "C" not in out
    assert "#elif" not in out


def test_fold_if_else_keeps_else_when_false():
    out = fold("#if 0\nA\n#else\nB\n#endif\n")
    assert "B" in out and "A" not in out


def test_fold_closed_namespace_collapses_config():
    state = {"CP_FEATURE": 1}
    out = fold(
        """\
        #if CP_FEATURE
        ON
        #else
        OFF
        #endif
        #ifndef CP_ABSENT
        DEFAULTED
        #endif
        """,
        state,
        closed=True,
    )
    assert "ON" in out and "OFF" not in out
    assert "DEFAULTED" in out  # #ifndef of an absent authoritative macro → true
    assert "#if" not in out


def test_fold_preserves_compiler_guard_but_folds_inner_config():
    out = fold(
        """\
        #if defined(__GNUC__) && !defined(__clang__)
          #if CP_OPT
          FAST
          #else
          SLOW
          #endif
        #endif
        """,
        {"CP_OPT": 1},
        closed=True,
    )
    # Outer compiler guard is undecidable → preserved verbatim.
    assert "#if defined(__GNUC__) && !defined(__clang__)" in out
    # Inner decidable config still folds.
    assert "FAST" in out and "SLOW" not in out
    assert "#if CP_OPT" not in out


def test_fold_unknown_block_does_not_leak_define():
    out = fold(
        """\
        #if FOO
        #define X 1
        #endif
        #if X
        YES
        #endif
        """
    )
    # FOO/X are non-authoritative and unknown → both blocks preserved verbatim,
    # and the guarded #define must not be applied to the live macro state.
    assert "#if FOO" in out
    assert "#if X" in out
    assert "#define X 1" in out
