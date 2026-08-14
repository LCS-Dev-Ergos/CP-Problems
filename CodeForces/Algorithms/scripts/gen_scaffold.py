#!/usr/bin/env python3
"""Code-generate the per-profile scaffold ``.cpp`` files under ``templates/cpp/``.

Each scaffold (``default.cpp``, ``advanced.cpp``, ``pbds.cpp``, ...) starts
from a common skeleton and applies the profile-specific ``NEED_*`` /
``CP_*`` block declared in ``profiles.toml``. Running this script keeps the
scaffolds aligned with the central registry so a new round can be spun up
from any profile without manual editing.
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
    ScaffoldProfile,
    load_registry,
    reset_cache,
)

SCAFFOLD_DIR = TEMPLATES_DIR / "cpp"

HEADER_DOC_BLOCK = """\
//===----------------------------------------------------------------------===//
/**
 * @file: __FILE_NAME__
 * @brief __PROBLEM_BRIEF__
 * @author: C.L.
 *
 * @status: In Progress
 */
//===----------------------------------------------------------------------===//

"""


def _section_banner(name: str) -> str:
    prefix = f"//=====----- [ {name} ] "
    return prefix + "-" * (80 - len(prefix) - len("-----=====//")) + "-----=====//"


def _append_macro_guard(lines: list[str], name: str, value: int | None = None) -> None:
    """Append an ``#ifndef`` guarded macro definition to ``lines``."""

    define = f"#define {name}" if value is None else f"#define {name} {value}"
    lines += [f"#ifndef {name}", f"  {define}", "#endif"]


def _render_scaffold(profile: ScaffoldProfile) -> str:
    """Render one scaffold source from its declarative registry profile."""

    lines: list[str] = []
    if profile.header_doc:
        lines.append(HEADER_DOC_BLOCK.rstrip("\n"))
        lines.append("")

    if profile.strict:
        _append_macro_guard(lines, "CP_TEMPLATE_PROFILE_STRICT")
    for define_name, define_value in profile.defines.items():
        _append_macro_guard(lines, define_name, define_value)
    if profile.advanced:
        if lines and lines[-1] != "":
            lines.append("")
        _append_macro_guard(lines, "CP_USE_ADVANCED", 1)
    if lines and lines[-1] != "":
        lines.append("")

    for need in sorted(profile.needs):
        lines.append(f"#define {need}")
    lines.append(f"#define CP_IO_PROFILE_{profile.io_profile.upper()}")
    lines += [
        "",
        '#include "templates/Base.hpp"',
        "",
        _section_banner("Solve"),
        "",
        "void solve() {",
        "  // Optimized solution here",
        "}",
        "",
        _section_banner("Main"),
        "",
        "auto main() -> int {",
        "#ifdef LOCAL",
        "  Stopwatch timer;",
        "#endif",
        "",
        "  INT(T);",
        "  FOR(T) solve();",
        "",
        "  return 0;",
        "}",
        "",
    ]
    return "\n".join(lines)


def _write_if_changed(path: Path, content: str) -> bool:
    """Write ``content`` only when it differs from the current file."""

    if not content.endswith("\n"):
        content += "\n"
    if path.is_file() and path.read_text(encoding="utf-8") == content:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")
    return True


def build_parser() -> argparse.ArgumentParser:
    """Build the CLI parser.

    The generator takes no options — the parser exists so that ``--help``
    answers "what does this write, and from where" without reading the source.
    """

    return argparse.ArgumentParser(
        description=(
            "Regenerate the per-profile scaffold .cpp files from "
            f"{DEFAULT_PROFILES_PATH.name}. Writes one file per [scaffold.*] "
            f"entry into {SCAFFOLD_DIR.parent.name}/{SCAFFOLD_DIR.name}/, and "
            "only when the rendered content differs. Run via "
            "`make regen-templates`."
        ),
    )


def main(argv: Sequence[str] | None = None) -> int:
    """Regenerate all scaffold sources from ``templates/profiles.toml``."""

    build_parser().parse_args(argv)
    reset_cache()
    registry: ProfileRegistry = load_registry(str(DEFAULT_PROFILES_PATH))
    changed = False
    for name, profile in registry.scaffolds.items():
        if _write_if_changed(SCAFFOLD_DIR / f"{name}.cpp", _render_scaffold(profile)):
            print(f"wrote {name}.cpp")
            changed = True
    if not changed:
        print("up-to-date")
    return 0


if __name__ == "__main__":
    raise SystemExit(run_cli(main))
