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

import sys
from pathlib import Path

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
        lines += [f"#if defined({target}) && ({condition})", f"  #undef {target}", "#endif"]
    lines.append("")
    return "\n".join(lines)


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


def main() -> int:
    """Regenerate config/profile/feature headers from ``templates/profiles.toml``."""

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
    if not changed:
        print("up-to-date")
    return 0


if __name__ == "__main__":
    sys.exit(main())
