"""Macro-table primitives shared across the flattener's preprocessor model.

Implements the conservative symbol table the partial evaluator runs against:
``update_macro_state_from_line`` applies ``#define``/``#undef`` directives and
``parse_macro_numeric_token`` interprets literals/known macros. The actual
``#if`` decision logic lives in :mod:`flattener_core.ppexpr`, which imports the
primitives defined here.

``is_authoritative_macro`` encodes the closed-namespace rule: the flattener
fully resolves the ``CP_*``/``NEED_*`` configuration namespace from
``profiles.toml`` plus the user source, so an absent ``CP_*``/``NEED_*`` macro
is *known* undefined. Every other macro (compiler/toolchain provided) is left
to the compiler.

The ``MacroValueMap`` alias is the canonical shape of the macro table that
flows through every other ``flattener_core`` module: name → known integer, or
name → ``None`` for a macro that is defined but whose value is opaque.
"""

from __future__ import annotations

import re

MacroValueMap = dict[str, int | None]

DEFINE_DIRECTIVE_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)\b")
# Group 2 captures the parameter list of a function-like macro, so the caller
# can tell ``#define F(x) 5`` from ``#define F 5``; group 3 is the body.
# The ``(`` must follow the name with no space: per the C standard
# ``#define F (x) 5`` is object-like with body ``(x) 5``, not a macro of one
# parameter. Both spellings end up opaque, but the classification is the
# thing this table is supposed to model correctly.
DEFINE_WITH_VALUE_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)(\([^)]*\))?(?:\s+(.*?))?\s*$")
UNDEF_DIRECTIVE_RE = re.compile(r"^\s*#\s*undef\s+([A-Za-z_]\w*)\s*$")
INTEGER_LITERAL_RE = re.compile(r"^[+-]?(?:0|[1-9]\d*|0[xX][0-9A-Fa-f]+)(?:[uUlL]{0,3})?$")

# The flattener is the single source of truth for these namespaces (resolved
# from profiles.toml + the user source); anything outside them is toolchain
# provided and must stay compiler-resolved.
_AUTHORITATIVE_PREFIXES = ("CP_", "NEED_")


def is_authoritative_macro(name: str) -> bool:
    """Return whether ``name`` lives in a namespace the flattener fully resolves."""

    return name.startswith(_AUTHORITATIVE_PREFIXES)


def parse_macro_numeric_token(token: str, macro_values: MacroValueMap) -> int | None:
    """Interpret a single token as an integer literal or known macro value.

    Strips C++ integer suffixes (``u``, ``L``, ``UL``, ...) before parsing.
    Returns ``None`` when the token is neither a literal nor a known macro.
    """

    stripped = token.strip()
    if not stripped:
        return None

    if INTEGER_LITERAL_RE.fullmatch(stripped):
        suffix_trimmed = re.sub(r"[uUlL]+$", "", stripped)
        try:
            return int(suffix_trimmed, 0)
        except ValueError:
            return None

    return macro_values.get(stripped)


def update_macro_state_from_line(macro_values: MacroValueMap, line: str) -> None:
    """Apply a single ``#define`` or ``#undef`` to the live macro table.

    Only simple numeric / identifier values are tracked; complex expressions
    are stored as ``None``.

    A *function-like* macro is recorded as defined-but-opaque (``None``) no
    matter what its body looks like. ``#define F(x) 5`` does not make ``F``
    worth 5: in an ``#if`` the name is only expanded when followed by ``(``,
    so folding ``#if F`` on the body would decide a guard on a value C never
    computes. ``defined(F)`` must still be true, which ``None`` gives us.
    """

    stripped = line.strip()
    if not stripped:
        return

    undef_match = UNDEF_DIRECTIVE_RE.match(stripped)
    if undef_match:
        macro_values.pop(undef_match.group(1), None)
        return

    define_match = DEFINE_WITH_VALUE_RE.match(stripped)
    if not define_match:
        return

    name, params, value_expr = define_match.group(1, 2, 3)
    if params is not None:
        macro_values[name] = None
        return
    if not value_expr:
        macro_values[name] = 1
        return

    token = value_expr.split("//", 1)[0]
    token = token.split("/*", 1)[0].strip()
    if not token:
        macro_values[name] = 1
        return

    token = token.split()[0]
    macro_values[name] = parse_macro_numeric_token(token, macro_values)
