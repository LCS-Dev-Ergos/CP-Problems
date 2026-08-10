"""High-level macro extraction from user source.

``extract_macro_values_from_source`` walks the source prefix before
``templates/Base.hpp``, applies a conservative folder to skip unevaluable
``#if`` blocks, then layers in defaults from ``profile_registry`` (strict
selection) and the per-IO-profile expansion of ``CP_IO_PROFILE_*``.

This is the pipeline-level wrapper around ``flattener_core.macros`` — the
latter handles individual ``#define`` lines, the former composes them
against the project's profile registry.
"""

from __future__ import annotations

import re
from typing import TextIO

from flattener_core.lexer import strip_non_code
from flattener_core.macros import MacroValueMap
from flattener_core.preprocessor import (
    ELSE_OR_ELIF_DIRECTIVE_RE,
    ENDIF_DIRECTIVE_RE,
    IF_DIRECTIVE_RE,
    fold_simple_preprocessor_conditionals,
)
from profile_registry import load_registry

_MACRO_DEFINE_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)(?:\s+(.*?))?\s*$")
_MACRO_UNDEF_RE = re.compile(r"^\s*#\s*undef\s+([A-Za-z_]\w*)\s*$")
_NUMERIC_LITERAL_RE = re.compile(r"^[+-]?(?:0|[1-9]\d*|0[xX][0-9A-Fa-f]+)(?:[uUlL]{0,3})?$")

def _parse_macro_value(value_expr: str | None, macro_values: MacroValueMap) -> int | None:
    """Parse a simple ``#define`` value into ``int`` / ``None``.

    Strips inline comments and trailing type suffixes (e.g. ``u``, ``L``).
    Falls back to looking up the token in ``macro_values`` when it is not a
    numeric literal.
    """

    if value_expr is None:
        return 1

    token = value_expr.split("//", 1)[0]
    token = token.split("/*", 1)[0].strip()
    if not token:
        return 1

    token = token.split()[0]
    if _NUMERIC_LITERAL_RE.fullmatch(token):
        suffix_trimmed = re.sub(r"[uUlL]+$", "", token)
        try:
            return int(suffix_trimmed, 0)
        except ValueError:
            return None

    return macro_values.get(token)


def extract_macro_values_from_source(
    source_prefix_content: str,
    *,
    strict_profile_enabled: bool,
    warn_stream: TextIO | None = None,
) -> MacroValueMap:
    """Extract simple macro values from user source for conservative folding.

    Defines inside conditional blocks the folder cannot evaluate are skipped.
    When ``warn_stream`` is provided, the skipped names are reported once.
    """

    macro_values: MacroValueMap = {}
    skipped: list[str] = []
    code_only = strip_non_code(source_prefix_content)
    # The flattener is authoritative over the CP_*/NEED_* namespace even in the
    # user prefix, so resolve the conventional ``#ifndef CP_X / #define CP_X``
    # override idiom (absent ⇒ undefined) while leaving the user's own
    # (non-CP_/NEED_) conditionals untouched.
    folded_code = fold_simple_preprocessor_conditionals(code_only, {}, closed_namespace=True)
    depth = 0

    for raw_line in folded_code.splitlines():
        stripped = raw_line.strip()
        if not stripped or stripped.startswith("//"):
            continue

        if IF_DIRECTIVE_RE.match(stripped):
            depth += 1
            continue

        if ENDIF_DIRECTIVE_RE.match(stripped):
            depth = max(0, depth - 1)
            continue

        if ELSE_OR_ELIF_DIRECTIVE_RE.match(stripped):
            continue

        if depth > 0:
            define_match = _MACRO_DEFINE_RE.match(stripped)
            if define_match:
                skipped.append(define_match.group(1))
            continue

        undef_match = _MACRO_UNDEF_RE.match(stripped)
        if undef_match:
            macro_values.pop(undef_match.group(1), None)
            continue

        define_match = _MACRO_DEFINE_RE.match(stripped)
        if not define_match:
            continue

        name, value_expr = define_match.group(1), define_match.group(2)
        macro_values[name] = _parse_macro_value(value_expr, macro_values)

    if warn_stream is not None and skipped:
        warn_stream.write(
            "warning: ignoring conditional #defines (guard unresolved): "
            + ", ".join(sorted(set(skipped)))
            + "\n"
        )

    registry = load_registry()
    for name, value in registry.config_defaults_as_dict(strict=strict_profile_enabled).items():
        macro_values.setdefault(name, value)

    def _is_enabled(name: str) -> bool:
        value = macro_values.get(name)
        return value is not None and value != 0

    enabled_profiles = [
        profile.macro for profile in registry.io_profiles.values() if _is_enabled(profile.macro)
    ]
    io_needs, io_defines = registry.expand_io_profiles(enabled_profiles)
    for need in io_needs:
        macro_values.setdefault(need, 1)
    for define_name, define_value in io_defines.items():
        macro_values.setdefault(define_name, define_value)

    # Legacy synonym: NEED_TYPE_SAFETY enables the advanced layer, mirroring the
    # ``#if defined(NEED_TYPE_SAFETY)`` shim in Base.hpp.
    if macro_values.get("NEED_TYPE_SAFETY"):
        macro_values.setdefault("CP_USE_ADVANCED", 1)

    # CP_USE_ADVANCED is a 0/1 switch consumed with ``#if``. It is intentionally
    # *not* a profiles.toml default (Config_defaults runs before the IO profile
    # and a default there would block the profile from turning it on). Resolve it
    # to 0 here, after the profile has had its say, so the advanced ``#if`` guards
    # fold cleanly out of non-advanced submissions.
    macro_values.setdefault("CP_USE_ADVANCED", 0)

    enabled_needs = {
        name for name, value in macro_values.items() if name.startswith("NEED_") and value != 0
    }
    normalized_needs = registry.normalize_needs(enabled_needs)
    for need in enabled_needs - normalized_needs:
        macro_values.pop(need, None)

    return macro_values
