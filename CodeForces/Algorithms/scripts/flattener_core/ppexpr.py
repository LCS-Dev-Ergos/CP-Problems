"""Three-valued partial evaluator for C preprocessor ``#if`` expressions.

The flattener must decide which conditional regions of the templates reach a
submission. This module is the single source of that decision: it tokenizes
and parses the restricted grammar that appears in ``#if``/``#elif`` directives
(integers, identifiers, ``defined``, ``!``/``&&``/``||``, comparisons,
parentheses, and ``__has_*(...)`` builtins) and evaluates it under a *partial*
macro state.

The value domain is ``int | None`` where ``None`` means UNKNOWN — the
expression depends on a macro whose value the flattener cannot know
(compiler/environment provided) or on a form the parser does not model. The
boolean front-ends collapse that to ``bool | None`` (``None`` == UNKNOWN), the
shape every caller already expects: a known truth folds the block, ``None``
preserves it verbatim.

Authority model (``closed_namespace``): the flattener is authoritative over the
``CP_*``/``NEED_*`` configuration namespace — when that flag is set, an absent
``CP_*``/``NEED_*`` macro is *definitely undefined* (``defined()`` → False,
value → 0, matching C ``#if`` semantics). Every other absent macro
(``__GNUC__``, ``HAS_INT128``, ``PBDS_AVAILABLE``, ``__has_include`` …) stays
UNKNOWN so its block survives. With the flag off every absent macro is UNKNOWN
(the conservative mode used when folding unresolved user source).
"""

from __future__ import annotations

import re

from flattener_core.macros import (
    MacroValueMap,
    is_authoritative_macro,
    parse_macro_numeric_token,
)

_WS_RE = re.compile(r"\s+")
_ID_RE = re.compile(r"[A-Za-z_]\w*")
_NUM_RE = re.compile(r"\d\w*")
_OP_RE = re.compile(r"<<|>>|<=|>=|==|!=|&&|\|\||[()!<>+\-*/%]")


class _ParseError(Exception):
    """Raised on any construct the evaluator does not model (→ UNKNOWN)."""


# A token is a ``(kind, value)`` pair: kind ∈ {"id", "num", "op", "unknown"}.
_Token = tuple[str, object]


def _tokenize(expr: str) -> list[_Token]:
    """Tokenize a preprocessor ``#if`` expression; raise on anything unmodeled."""

    tokens: list[_Token] = []
    i = 0
    n = len(expr)
    while i < n:
        ws = _WS_RE.match(expr, i)
        if ws:
            i = ws.end()
            continue

        ident = _ID_RE.match(expr, i)
        if ident:
            name = ident.group(0)
            i = ident.end()
            # ``__has_include(<...>)`` / ``__has_builtin(...)`` etc.: the argument
            # is not a normal expression (it carries ``<>``/paths), so swallow the
            # whole balanced call and treat it as an opaque UNKNOWN atom.
            if name.startswith("__has_"):
                i = _consume_balanced_call(expr, i)
                tokens.append(("unknown", None))
                continue
            tokens.append(("id", name))
            continue

        number = _NUM_RE.match(expr, i)
        if number:
            tokens.append(("num", number.group(0)))
            i = number.end()
            continue

        operator = _OP_RE.match(expr, i)
        if operator:
            tokens.append(("op", operator.group(0)))
            i = operator.end()
            continue

        raise _ParseError(f"unexpected character {expr[i]!r}")

    return tokens


def _consume_balanced_call(expr: str, start: int) -> int:
    """Skip an optional whitespace-then-balanced ``(...)`` group; return new index."""

    j = start
    n = len(expr)
    while j < n and expr[j].isspace():
        j += 1
    if j >= n or expr[j] != "(":
        return start
    depth = 0
    while j < n:
        if expr[j] == "(":
            depth += 1
        elif expr[j] == ")":
            depth -= 1
            if depth == 0:
                return j + 1
        j += 1
    raise _ParseError("unbalanced __has_* call")


class _Parser:
    """Recursive-descent parser over the token list, evaluating as it goes.

    Each production returns ``int | None`` (None == UNKNOWN). Logical operators
    use Kleene three-valued semantics with short-circuit on the dominating value
    (FALSE for ``&&``, TRUE for ``||``).
    """

    def __init__(self, tokens: list[_Token], state: MacroValueMap, closed: bool) -> None:
        self._tokens = tokens
        self._pos = 0
        self._state = state
        self._closed = closed

    def parse(self) -> int | None:
        value = self._or()
        if self._pos != len(self._tokens):
            raise _ParseError("trailing tokens")
        return value

    def _peek(self) -> _Token:
        return self._tokens[self._pos] if self._pos < len(self._tokens) else ("end", None)

    def _advance(self) -> _Token:
        token = self._peek()
        self._pos += 1
        return token

    def _or(self) -> int | None:
        value = self._and()
        while self._peek() == ("op", "||"):
            self._advance()
            value = _kleene_or(value, self._and())
        return value

    def _and(self) -> int | None:
        value = self._equality()
        while self._peek() == ("op", "&&"):
            self._advance()
            value = _kleene_and(value, self._equality())
        return value

    def _equality(self) -> int | None:
        value = self._relational()
        while self._peek() in (("op", "=="), ("op", "!=")):
            op = self._advance()[1]
            value = _compare(value, self._relational(), str(op))
        return value

    def _relational(self) -> int | None:
        value = self._additive()
        while self._peek() in (("op", "<"), ("op", ">"), ("op", "<="), ("op", ">=")):
            op = self._advance()[1]
            value = _compare(value, self._additive(), str(op))
        return value

    def _additive(self) -> int | None:
        value = self._multiplicative()
        while self._peek() in (("op", "+"), ("op", "-")):
            op = self._advance()[1]
            value = _arith(value, self._multiplicative(), str(op))
        return value

    def _multiplicative(self) -> int | None:
        value = self._unary()
        while self._peek() in (("op", "*"), ("op", "/"), ("op", "%")):
            op = self._advance()[1]
            value = _arith(value, self._unary(), str(op))
        return value

    def _unary(self) -> int | None:
        token = self._peek()
        if token == ("op", "!"):
            self._advance()
            value = self._unary()
            return None if value is None else int(value == 0)
        if token == ("op", "-"):
            self._advance()
            value = self._unary()
            return None if value is None else -value
        if token == ("op", "+"):
            self._advance()
            return self._unary()
        return self._primary()

    def _primary(self) -> int | None:
        kind, value = self._peek()
        if (kind, value) == ("op", "("):
            self._advance()
            inner = self._or()
            if self._peek() != ("op", ")"):
                raise _ParseError("missing ')'")
            self._advance()
            return inner
        if kind == "unknown":
            self._advance()
            return None
        if kind == "num":
            self._advance()
            literal = parse_macro_numeric_token(str(value), {})
            if literal is None:
                raise _ParseError(f"bad numeric literal {value!r}")
            return literal
        if kind == "id":
            self._advance()
            if value == "defined":
                return self._defined()
            return self._macro_value(str(value))
        raise _ParseError("unexpected token")

    def _defined(self) -> int | None:
        if self._peek() == ("op", "("):
            self._advance()
            kind, name = self._advance()
            if kind != "id":
                raise _ParseError("defined() expects an identifier")
            if self._peek() != ("op", ")"):
                raise _ParseError("missing ')' after defined()")
            self._advance()
        else:
            kind, name = self._advance()
            if kind != "id":
                raise _ParseError("defined expects an identifier")
        truth = self._is_defined(str(name))
        return None if truth is None else int(truth)

    def _is_defined(self, name: str) -> bool | None:
        if name in self._state:
            return True
        if self._closed and is_authoritative_macro(name):
            return False
        return None

    def _macro_value(self, name: str) -> int | None:
        if name in self._state:
            return self._state[name]  # int (known) or None (defined-but-opaque)
        if self._closed and is_authoritative_macro(name):
            return 0
        return None


def _kleene_and(a: int | None, b: int | None) -> int | None:
    if a == 0 or b == 0:  # FALSE dominates (None != 0 in Python)
        return 0
    if a is None or b is None:
        return None
    return 1 if (a != 0 and b != 0) else 0


def _kleene_or(a: int | None, b: int | None) -> int | None:
    if (a is not None and a != 0) or (b is not None and b != 0):  # TRUE dominates
        return 1
    if a is None or b is None:
        return None
    return 0


def _compare(a: int | None, b: int | None, op: str) -> int | None:
    if a is None or b is None:
        return None
    return int(
        {
            "==": a == b,
            "!=": a != b,
            "<": a < b,
            ">": a > b,
            "<=": a <= b,
            ">=": a >= b,
        }[op]
    )


def _arith(a: int | None, b: int | None, op: str) -> int | None:
    if a is None or b is None:
        return None
    if op == "+":
        return a + b
    if op == "-":
        return a - b
    if op == "*":
        return a * b
    if b == 0:  # division/modulo by zero is undecidable here
        return None
    # C/C++ integer division truncates toward zero; Python's ``//`` floors and
    # its ``%`` takes the sign of the divisor. Both differ from the
    # preprocessor on negative operands (C: -7/2 == -3, -7%2 == -1), so the
    # quotient is computed explicitly and the remainder derived from it.
    quotient = -(-a // b) if (a < 0) != (b < 0) else a // b
    return quotient if op == "/" else a - b * quotient


def evaluate_expression(
    expr: str, macro_values: MacroValueMap, *, closed_namespace: bool = False
) -> int | None:
    """Evaluate a ``#if``/``#elif`` expression to ``int | None`` (None == UNKNOWN)."""

    try:
        tokens = _tokenize(expr)
        if not tokens:
            return None
        return _Parser(tokens, macro_values, closed_namespace).parse()
    except (_ParseError, RecursionError):
        return None


def evaluate_simple_if_expression(
    expr: str, macro_values: MacroValueMap, *, closed_namespace: bool = False
) -> bool | None:
    """Truth of a ``#if``/``#elif`` expression: ``True``/``False``/``None`` (UNKNOWN)."""

    value = evaluate_expression(expr, macro_values, closed_namespace=closed_namespace)
    return None if value is None else value != 0


def is_macro_defined(
    name: str, macro_values: MacroValueMap, *, closed_namespace: bool = False
) -> bool | None:
    """Definedness of ``name`` for ``#ifdef``/``#ifndef`` (``None`` == UNKNOWN)."""

    if name in macro_values:
        return True
    if closed_namespace and is_authoritative_macro(name):
        return False
    return None


def evaluate_condition(
    kind: str, text: str | None, macro_values: MacroValueMap, *, closed_namespace: bool = False
) -> bool | None:
    """Truth of a conditional directive of the given ``kind`` (``None`` == UNKNOWN).

    ``kind`` is one of ``if``/``elif`` (expression), ``ifdef``/``ifndef``
    (definedness), or ``else`` (always reached when its chain falls through).
    """

    if kind in ("if", "elif"):
        return evaluate_simple_if_expression(
            text or "", macro_values, closed_namespace=closed_namespace
        )
    if kind == "ifdef":
        return is_macro_defined(text or "", macro_values, closed_namespace=closed_namespace)
    if kind == "ifndef":
        truth = is_macro_defined(text or "", macro_values, closed_namespace=closed_namespace)
        return None if truth is None else not truth
    if kind == "else":
        return True
    return None
