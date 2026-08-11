"""Unit tests for the ``strip_index_casts`` source-rewriting utility.

This script edits the C++ library in place, so its transformations are pinned
here rather than trusted to a smoke import. The contract has two halves:

- what it *must* rewrite (``as<Size>``, and ``as<I32>``/``as<I64>`` applied to
  a ``.size()`` call);
- what it must leave strictly alone (every other ``as<...>`` cast, casts whose
  name merely ends in ``as``, and subscripts whose parentheses carry meaning).

The second half is the one that matters: a false positive here silently
changes the semantics of a shipped module.
"""

from __future__ import annotations

from pathlib import Path

from strip_index_casts import (
    _has_top_level_comma,
    collect_files,
    find_matching_paren,
    strip_subscript_parens,
    transform,
)

# ------------------------------ Cast Stripping ------------------------------ #


def test_size_cast_on_simple_expression_drops_the_parens():
    assert transform("as<Size>(n)") == "n"
    assert transform("as<Size>(v.size())") == "v.size()"


def test_size_cast_on_compound_expression_keeps_parens_for_precedence():
    """``as<Size>(a + b)`` must not become a bare ``a + b``."""

    assert transform("as<Size>(a + b)") == "(a + b)"


def test_signed_size_casts_become_helpers():
    assert transform("as<I32>(v.size())") == "isz(v)"
    assert transform("as<I64>(v.size())") == "sz(v)"
    assert transform("as<I64>(grid[i].size())") == "sz(grid[i])"


def test_signed_cast_without_size_call_is_preserved():
    """``as<I32>`` is only rewritten in the ``.size()`` form; otherwise it stays."""

    assert transform("as<I32>(x)") == "as<I32>(x)"
    assert transform("as<I64>(x + 1)") == "as<I64>(x + 1)"


def test_untracked_cast_kinds_are_untouched():
    for source in ("as<U64>(x)", "as<I128>(x)", "as<F80>(x)", "as<Calc>(v.size())"):
        assert transform(source) == source


def test_left_word_boundary_is_enforced():
    """An identifier ending in ``as`` must not be treated as a cast."""

    assert transform("xyz_as<Size>(n)") == "xyz_as<Size>(n)"


def test_multiple_casts_on_one_line_are_all_rewritten():
    assert transform("f(as<Size>(i), as<I64>(v.size()))") == "f(i, sz(v))"


def test_nested_casts_reach_a_fixed_point():
    """The iterative pass unwraps the inner cast; the outer parens are kept."""

    assert transform("as<Size>(as<Size>(n))") == "(n)"


def test_transform_is_idempotent():
    once = transform("f(as<Size>(i), as<I64>(v.size()))")
    assert transform(once) == once


# ------------------------- Subscript Paren Cleanup -------------------------- #


def test_redundant_subscript_parens_are_collapsed():
    assert strip_subscript_parens("v[(i)]") == "v[i]"
    assert strip_subscript_parens("v[(i+1)]") == "v[i+1]"
    assert strip_subscript_parens("m[(x)][(y)]") == "m[x][y]"


def test_partial_subscript_parens_are_left_alone():
    """The ``)`` is not immediately before ``]``, so the parens still bind."""

    assert strip_subscript_parens("v[(i + 1) % n]") == "v[(i + 1) % n]"


def test_comma_operator_subscript_is_left_alone():
    """``v[(a, b)]`` evaluates both operands; dropping the parens changes it."""

    assert strip_subscript_parens("v[(a, b)]") == "v[(a, b)]"


def test_nested_call_commas_do_not_block_the_cleanup():
    assert strip_subscript_parens("v[(f(a,b))]") == "v[f(a,b)]"


# -------------------------------- Primitives -------------------------------- #


def test_find_matching_paren_handles_nesting():
    assert find_matching_paren("(a(b)c)", 0) == 6


def test_find_matching_paren_reports_unbalanced_input():
    assert find_matching_paren("(a", 0) == -1


def test_top_level_comma_detection_ignores_nested_groups():
    assert _has_top_level_comma("a, b") is True
    assert _has_top_level_comma("f(a, b)") is False
    assert _has_top_level_comma("v[a, b]") is False
    assert _has_top_level_comma("a") is False


def test_collect_files_applies_exclude_globs(tmp_path: Path):
    (tmp_path / "nested").mkdir()
    (tmp_path / "keep.hpp").write_text("", encoding="utf-8")
    (tmp_path / "nested" / "drop.hpp").write_text("", encoding="utf-8")
    (tmp_path / "ignored.cpp").write_text("", encoding="utf-8")

    assert [p.name for p in collect_files(tmp_path, [])] == ["keep.hpp", "drop.hpp"]
    assert [p.name for p in collect_files(tmp_path, ["*/nested/*"])] == ["keep.hpp"]
