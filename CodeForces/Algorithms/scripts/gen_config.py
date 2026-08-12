#!/usr/bin/env python3
"""Code-generate C++ template config headers from ``profiles.toml``.

The TOML file is the single source of truth for template tunables (base vs
strict defaults), per-IO profile defines, and ``NEED_*`` feature headers. This
script materializes those values into the C++ headers the template system
includes, so the compiler sees the same defaults the Python pipeline does.

Intended to be re-run via ``make regen-templates`` whenever ``profiles.toml``
changes; the output is committed alongside the source.
"""

from __future__ import annotations

import argparse
from collections.abc import Sequence
from pathlib import Path

from _lib.cli import run_cli
from profile_registry import (
    DEFAULT_PROFILES_PATH,
    TEMPLATES_DIR,
    ProfileRegistry,
    load_registry,
    reset_cache,
)

CONFIG_OUTPUT = TEMPLATES_DIR / "core" / "Config_defaults.hpp"
BASE_PROFILES_OUTPUT = TEMPLATES_DIR / "Base_profiles.hpp"
BASE_FEATURES_OUTPUT = TEMPLATES_DIR / "Base_features.hpp"
BASE_CONTRACTS_OUTPUT = TEMPLATES_DIR / "Base_contracts.hpp"

AUTOGEN_HEADER = "// Generated from profiles.toml. Do not edit by hand.\n"


def _render_config_defaults(registry: ProfileRegistry) -> str:
    """Render default ``CP_*`` config values and strict-profile overrides."""

    lines = ["#pragma once", "", AUTOGEN_HEADER.rstrip("\n"), ""]
    lines.append("#if defined(CP_TEMPLATE_PROFILE_STRICT)")
    for macro, value in registry.defaults.strict_overrides.items():
        lines += [f"  #ifndef {macro}", f"    #define {macro} {value}", "  #endif"]
    lines.append("#endif")
    lines.append("")
    for macro, value in registry.defaults.base.items():
        lines += [f"#ifndef {macro}", f"  #define {macro} {value}", "#endif"]
    lines.append("")
    return "\n".join(lines)


def _render_base_profiles(registry: ProfileRegistry) -> str:
    """Render IO-profile expansion and canonical ``NEED_*`` shadow rules."""

    profile_macros = registry.io_profile_macros()
    lines = [
        "#pragma once",
        "",
        AUTOGEN_HEADER.rstrip("\n"),
        "",
        "#if (" + " + ".join(f"defined({m})" for m in profile_macros) + ") > 1",
        '  #error "Choose at most one CP_IO_PROFILE_* macro."',
        "#endif",
        "",
    ]
    for profile in registry.io_profiles.values():
        macro = profile.macro
        lines.append(f"#ifdef {macro}")
        for need in sorted(profile.needs):
            lines += [f"  #ifndef {need}", f"    #define {need}", "  #endif"]
        for define_name, define_value in profile.defines.items():
            lines += [
                f"  #ifndef {define_name}",
                f"    #define {define_name} {define_value}",
                "  #endif",
            ]
        lines += ["#endif", ""]

    for target, blockers in registry.need_shadow_rules():
        condition = " || ".join(f"defined({blocker})" for blocker in sorted(blockers))
        lines += [
            f"#if defined({target}) && ({condition})",
            "  #if defined(CP_TEMPLATE_PROFILE_STRICT) || defined(CP_STRICT_TEMPLATE_NEEDS)",
            f'    #error "Conflicting template features: {target} is shadowed by a selected backend."',
            "  #else",
            f"    #undef {target}",
            "  #endif",
            "#endif",
        ]
    lines.append("")
    return "\n".join(lines)


def _render_base_contracts() -> str:
    """Render compile-time validation for public 0/1 and variant switches."""

    return "\n".join(
        [
            "#pragma once",
            "",
            AUTOGEN_HEADER.rstrip("\n"),
            "",
            '#include "templates/core/Config_defaults.hpp"',
            "",
            "#if CP_USE_ADVANCED != 0 && CP_USE_ADVANCED != 1",
            '  #error "CP_USE_ADVANCED must be 0 or 1."',
            "#endif",
            "#if CP_FAST_IO_VARIANT != 0 && CP_FAST_IO_VARIANT != 1",
            '  #error "CP_FAST_IO_VARIANT must be 0 (minimal) or 1 (refill)."',
            "#endif",
            "#if CP_FLOAT_PRECISION < 0",
            '  #error "CP_FLOAT_PRECISION must be non-negative."',
            "#endif",
            "#if CP_FAST_IO_MAX_TOKEN_SIZE <= 0",
            '  #error "CP_FAST_IO_MAX_TOKEN_SIZE must be positive."',
            "#endif",
            "",
        ]
    )


def _include_line(header: str) -> str:
    """Render one project-local include directive, rooted at the repo."""

    return f'  #include "templates/{header}"'


def _render_base_features(registry: ProfileRegistry) -> str:
    """Render the generated ``NEED_*`` feature-to-header include layer."""

    lines = ["#pragma once", "", AUTOGEN_HEADER.rstrip("\n"), ""]
    for feature in registry.features.values():
        lines.append(f"#ifdef {feature.name}")
        for header in feature.headers:
            lines.append(_include_line(header))
        for group in feature.conditional_headers:
            lines.append(f"  #if {group.condition}")
            for header in group.headers:
                lines.append("  " + _include_line(header))
            lines.append("  #endif")
        lines += ["#endif", ""]
    return "\n".join(lines)


def _write_if_changed(path: Path, content: str) -> bool:
    """Write ``content`` only when it differs from the existing file."""

    if not content.endswith("\n"):
        content += "\n"
    if path.is_file() and path.read_text(encoding="utf-8") == content:
        return False
    path.write_text(content, encoding="utf-8")
    return True


def build_parser() -> argparse.ArgumentParser:
    """Build the CLI parser.

    The generator takes no options — the parser exists so that ``--help``
    answers "what does this write, and from where" without reading the source.
    """

    return argparse.ArgumentParser(
        description=(
            "Regenerate the C++ template config headers from "
            f"{DEFAULT_PROFILES_PATH.name}. Writes "
            f"{CONFIG_OUTPUT.name}, {BASE_PROFILES_OUTPUT.name} and "
            f"{BASE_FEATURES_OUTPUT.name}, {BASE_CONTRACTS_OUTPUT.name} under "
            "templates/, and only when the "
            "rendered content differs. Run via `make regen-templates`."
        ),
    )


def main(argv: Sequence[str] | None = None) -> int:
    """Regenerate config/profile/feature headers from ``templates/profiles.toml``."""

    build_parser().parse_args(argv)
    reset_cache()
    registry = load_registry(str(DEFAULT_PROFILES_PATH))
    changed = False
    if _write_if_changed(CONFIG_OUTPUT, _render_config_defaults(registry)):
        print(f"wrote {CONFIG_OUTPUT}")
        changed = True
    if _write_if_changed(BASE_PROFILES_OUTPUT, _render_base_profiles(registry)):
        print(f"wrote {BASE_PROFILES_OUTPUT}")
        changed = True
    if _write_if_changed(BASE_FEATURES_OUTPUT, _render_base_features(registry)):
        print(f"wrote {BASE_FEATURES_OUTPUT}")
        changed = True
    if _write_if_changed(BASE_CONTRACTS_OUTPUT, _render_base_contracts()):
        print(f"wrote {BASE_CONTRACTS_OUTPUT}")
        changed = True
    if not changed:
        print("up-to-date")
    return 0


if __name__ == "__main__":
    raise SystemExit(run_cli(main))
