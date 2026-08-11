"""Contract for the macro table shared by the engine and the pipeline.

``flattener_core.macros`` is the single implementation of "apply one
``#define`` / ``#undef`` line". ``flattener_pipeline.macro_resolution`` used
to carry a private copy of the same regexes, and the two had already drifted:
the core one understood function-like macros, the pipeline one silently
dropped them.

The interesting case is what a function-like macro is *worth*. In C, ``F`` is
only expanded when followed by ``(``, so ``#if F`` never sees the body — but
``defined(F)`` is still true. The table models that as defined-but-opaque
(``None``), which is neither of the two behaviors this codebase had before.
"""

from __future__ import annotations

from flattener_core.macros import MacroValueMap, update_macro_state_from_line
from flattener_core.ppexpr import evaluate_simple_if_expression, is_macro_defined
from flattener_pipeline.macro_resolution import extract_macro_values_from_source


def _apply(*lines: str) -> MacroValueMap:
    state: MacroValueMap = {}
    for line in lines:
        update_macro_state_from_line(state, line)
    return state


# ------------------------------- Object-like -------------------------------- #


def test_bare_define_is_worth_one():
    assert _apply("#define CP_PLAIN") == {"CP_PLAIN": 1}


def test_numeric_values_are_parsed_including_hex_and_suffixes():
    assert _apply("#define CP_BAZ 7") == {"CP_BAZ": 7}
    assert _apply("#define CP_Q 0x1F") == {"CP_Q": 31}
    assert _apply("#define CP_W 3uLL") == {"CP_W": 3}


def test_value_referring_to_a_known_macro_resolves_through_it():
    assert _apply("#define CP_A 4", "#define CP_B CP_A") == {"CP_A": 4, "CP_B": 4}


def test_non_numeric_body_is_defined_but_opaque():
    assert _apply("#define CP_X some_expr()") == {"CP_X": None}


def test_undef_removes_the_entry_entirely():
    assert _apply("#define CP_A 4", "#undef CP_A") == {}


def test_redefinition_after_undef_takes_the_new_value():
    assert _apply("#define CP_A 4", "#undef CP_A", "#define CP_A 9") == {"CP_A": 9}


def test_inline_comments_are_stripped_from_the_value():
    assert _apply("#define CP_A 4 // four") == {"CP_A": 4}
    assert _apply("#define CP_B 5 /* five */") == {"CP_B": 5}


# ------------------------------ Function-like ------------------------------- #


def test_function_like_macro_is_opaque_regardless_of_its_body():
    """``#define F(x) 5`` must not make ``F`` worth 5."""

    assert _apply("#define CP_FOO(x) 5") == {"CP_FOO": None}
    assert _apply("#define CP_BAR(a, b) 1") == {"CP_BAR": None}
    assert _apply("#define CP_IMPL(...) impl(__VA_ARGS__)") == {"CP_IMPL": None}


def test_function_like_macro_still_counts_as_defined():
    """``#ifndef F`` must fold to false once ``F(x)`` has been seen."""

    state = _apply("#define CP_IMPL(...) impl(__VA_ARGS__)")

    assert is_macro_defined("CP_IMPL", state, closed_namespace=True) is True


def test_if_on_a_function_like_macro_stays_unknown():
    """UNKNOWN preserves the block; folding it would decide on a phantom value."""

    state = _apply("#define CP_FOO(x) 5")

    assert evaluate_simple_if_expression("CP_FOO", state, closed_namespace=True) is None


def test_a_space_before_the_paren_makes_it_object_like():
    """Per the C standard ``#define F (x) 5`` has body ``(x) 5``, no parameters.

    Both spellings end up opaque — ``(x) 5`` is not a numeric literal either —
    so this pins the classification rather than a difference in outcome. It
    matters because the regex decides it, and ``\\s*\\(`` would get it wrong.
    """

    assert _apply("#define CP_FOO (x) 5") == {"CP_FOO": None}
    assert _apply("#define CP_FOO(x) 5") == {"CP_FOO": None}


# ----------------- Pipeline and Engine agree on one source ------------------ #


def test_pipeline_extraction_matches_the_engine_on_function_like_macros():
    """The regression the merge fixes: the pipeline used to drop these."""

    source = '#define CP_FOO(x) 5\n#define CP_BAZ 7\n#include "templates/Base.hpp"\n'

    values = extract_macro_values_from_source(source, strict_profile_enabled=False)

    assert "CP_FOO" in values, "function-like define must not be silently dropped"
    assert values["CP_FOO"] is None
    assert values["CP_BAZ"] == 7
