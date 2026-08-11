"""Static constants used by workflow manager components.

The user-facing CLI choices are exposed as ``enum.StrEnum`` types so that:

- mypy can spot typos like ``BuildType.DEBUF`` at check time;
- argparse keeps accepting the same string literals (``StrEnum`` members are
  ``str`` subclasses, so ``choices=tuple(e.value for e in BuildType)`` is
  unchanged on the command line);
- downstream dispatch (``match preset:``) can branch on enum members rather
  than fragile string comparisons.
"""

import re
import sys
from enum import StrEnum
from pathlib import Path

DEFAULT_WORKSPACE_ROOT = Path(__file__).resolve().parents[4]
# Resolution chain in utils.discover_cp_tools_script covers $CPP_TOOLS_SCRIPT
# and ~/.config/cpp-tools/competitive.sh, so no hardcoded fallback is needed.
DEFAULT_CP_TOOLS_SCRIPT: Path | None = None

_SCRIPTS_DIR = Path(__file__).resolve().parents[1]
if str(_SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(_SCRIPTS_DIR))
from profile_registry import load_registry  # noqa: E402

TEMPLATE_CHOICES = load_registry().all_scaffold_names()


class BuildType(StrEnum):
    """CMake build-type identifiers accepted by ``cppconf``."""

    DEBUG = "Debug"
    RELEASE = "Release"
    SANITIZE = "Sanitize"


class CompilerChoice(StrEnum):
    """Front-end compiler families exposed to the CLI."""

    GCC = "gcc"
    CLANG = "clang"
    AUTO = "auto"


class Toggle(StrEnum):
    """Generic on/off switch used by tuning flags (timing, etc.)."""

    ON = "on"
    OFF = "off"


class PchMode(StrEnum):
    """Precompiled-header toggle, including the ``auto`` heuristic."""

    ON = "on"
    OFF = "off"
    AUTO = "auto"


class ConfigPreset(StrEnum):
    """CMake configure preset names registered in ``CPRoundPresets.json``."""

    CP_DEBUG_GCC = "cp-debug-gcc"
    CP_RELEASE_GCC = "cp-release-gcc"
    CP_SANITIZE_GCC = "cp-sanitize-gcc"
    CP_DEBUG_CLANG = "cp-debug-clang"
    CP_RELEASE_CLANG = "cp-release-clang"
    CP_SANITIZE_CLANG = "cp-sanitize-clang"


class BuildPreset(StrEnum):
    """CMake build preset names registered in ``CPRoundPresets.json``."""

    CP_BUILD_DEBUG_GCC = "cp-build-debug-gcc"
    CP_BUILD_RELEASE_GCC = "cp-build-release-gcc"
    CP_BUILD_SANITIZE_GCC = "cp-build-sanitize-gcc"
    CP_BUILD_DEBUG_CLANG = "cp-build-debug-clang"
    CP_BUILD_RELEASE_CLANG = "cp-build-release-clang"
    CP_BUILD_SANITIZE_CLANG = "cp-build-sanitize-clang"


# Tuple views kept for argparse ``choices`` argument compatibility.
# ``StrEnum`` members iterate as strings, so this stays bit-identical to the
# pre-Phase-3 tuples that argparse accepted.
BUILD_TYPE_CHOICES: tuple[str, ...] = tuple(e.value for e in BuildType)
COMPILER_CHOICES: tuple[str, ...] = tuple(e.value for e in CompilerChoice)
TOGGLE_CHOICES: tuple[str, ...] = tuple(e.value for e in Toggle)
PCH_CHOICES: tuple[str, ...] = tuple(e.value for e in PchMode)
CMAKE_CONFIG_PRESET_CHOICES: tuple[str, ...] = tuple(e.value for e in ConfigPreset)
CMAKE_BUILD_PRESET_CHOICES: tuple[str, ...] = tuple(e.value for e in BuildPreset)

TARGET_RE = re.compile(r"^[A-Za-z]\w*$")
CONTEST_SEGMENT_RE = re.compile(r"^[A-Za-z0-9._-]+$")

# The gate the zsh wrapper is rendered from: exactly the cpp-tools functions
# this CLI dispatches, and nothing else. Entries no CommandSpec can reach
# widen what the wrapper will run without giving the CLI anything, so a
# regression test keeps this set equal to the dispatched set — adding a
# function here without a subcommand to use it will fail that test.
ALLOWED_FUNCTIONS = {
    "cpparchive",
    "cppbatch",
    "cppbuild",
    "cppcheck",
    "cppclean",
    "cppconf",
    "cppcontest",
    "cppdeepclean",
    "cppdelete",
    "cppdiag",
    "cppfull",
    "cppgo",
    "cpphelp",
    "cppinfo",
    "cppinit",
    "cppjudge",
    "cppnew",
    "cpprun",
    "cppstats",
    "cppstress",
    "cppsubmit",
    "cpptestsubmit",
    "cppwatch",
}

INPUT_FILE_HELP = "input filename inside input_cases/"
