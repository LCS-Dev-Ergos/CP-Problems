"""Contract for the symbol surface that drives template tree-shaking.

``prune_template_headers`` keeps a header only when one of the tokens
``extract_public_symbols`` harvested from it appears in the user's source. A
name the harvester misses is therefore a header the pruner silently deletes
from a submission that needs it — the failure shows up as a compile error in
the flattened output, far from the cause.

Macro names are the case that bit us: a feature header whose whole public
surface is ``#define``s looked symbol-less, so a solution using ``SUM(v)``
had the header pruned out from under it.
"""

from __future__ import annotations

import textwrap

from flattener_core.symbols import extract_public_symbols


def _symbols(source: str) -> set[str]:
    return extract_public_symbols(textwrap.dedent(source))


# --------------------------------- Macros ----------------------------------- #


def test_object_and_function_like_macros_are_exported():
    symbols = _symbols(
        """\
        #define SUM(x) cp::sum_range(x)
        #define CP_MARKER 1
        """
    )

    assert {"SUM", "CP_MARKER"} <= symbols


def test_macros_are_collected_regardless_of_brace_depth():
    """The preprocessor ignores scope, so the harvester must too."""

    symbols = _symbols(
        """\
        namespace cp {
        struct Holder {
          int x;
        };
        #define NESTED_MACRO(x) (x)
        }
        """
    )

    assert "NESTED_MACRO" in symbols


def test_a_macro_only_header_exports_the_names_callers_actually_write():
    """The regression, stated precisely.

    The trigger set was never empty here — ``FUNC_LIKE_RE`` picked ``sum_range``
    out of the macro *body*. But callers write ``SUM(v)``, never
    ``cp::sum_range(v)``, so no trigger ever matched the user's source and the
    header was pruned away. What was missing is the macro name itself.
    """

    symbols = _symbols(
        """\
        #define SUM(x) cp::sum_range(x)
        #define MIN(x) (*std::ranges::min_element(x))
        #define MAX(x) (*std::ranges::max_element(x))
        """
    )

    assert {"SUM", "MIN", "MAX"} <= symbols


def test_indented_directives_are_recognized():
    assert "GUARDED" in _symbols(
        """\
        #if CP_SOMETHING
          #define GUARDED(x) (x)
        #endif
        """
    )


def test_undef_does_not_export_a_name():
    assert "GONE" not in _symbols("#undef GONE\n")


# ------------------------------ Declarations -------------------------------- #


def test_top_level_declarations_are_exported():
    symbols = _symbols(
        """\
        struct BitRange {
          int mask;
        };
        using Pair = std::pair<int, int>;
        template <class R>
        constexpr int sz32(const R& x) {
          return 0;
        }
        """
    )

    assert {"BitRange", "Pair", "sz32"} <= symbols


def test_declarations_nested_in_a_class_are_not_exported():
    """Only depth-0 declarations are a stable proxy for the public surface."""

    symbols = _symbols(
        """\
        struct Outer {
          struct Inner {
            int y;
          };
        };
        """
    )

    assert "Outer" in symbols
    assert "Inner" not in symbols


def test_cpp_keywords_are_not_mistaken_for_callables():
    assert "if" not in _symbols("if (cond) { return 1; }\n")


def test_names_inside_comments_and_strings_are_ignored():
    symbols = _symbols(
        """\
        // struct CommentedType {
        const char* s = "struct StringType {";
        """
    )

    assert "CommentedType" not in symbols
    assert "StringType" not in symbols
