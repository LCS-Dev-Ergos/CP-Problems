"""Per-subcommand argparse declarations.

Each ``configure_*`` attaches one subcommand's flags to its subparser, and the
``add_*`` helpers hold the option groups several subcommands share (a target
name, the ``cppconf`` tuning flags, the Algorithms-directory override).

Kept apart from the command registry because these functions are shared
*across* subcommand families — ``configure_target_optional`` alone backs six
commands — so splitting the CLI by family would scatter them and force
cross-imports. The split is by pipeline stage instead: declare here, build
argv in ``argbuilders``, dispatch in ``commands``.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from .constants import (
    BUILD_TYPE_CHOICES,
    CMAKE_BUILD_PRESET_CHOICES,
    CMAKE_CONFIG_PRESET_CHOICES,
    COMPILER_CHOICES,
    INPUT_FILE_HELP,
    PCH_CHOICES,
    TEMPLATE_CHOICES,
    TOGGLE_CHOICES,
)
from .utils import (
    normalize_contest_dir,
    normalize_input_name,
    normalize_target,
    parse_positive_int,
)

__all__ = [
    "add_algorithms_dir_option",
    "add_conf_options",
    "add_target_arg",
    "configure_batch",
    "configure_conf",
    "configure_contest",
    "configure_cycle",
    "configure_deepclean",
    "configure_delete",
    "configure_doctor",
    "configure_full",
    "configure_go",
    "configure_new",
    "configure_preset_build",
    "configure_preset_conf",
    "configure_stress",
    "configure_submit",
    "configure_target_optional",
    "configure_test_submit",
]


# ------------------------------ Shared fragments ---------------------------- #


def add_target_arg(
    parser: argparse.ArgumentParser, required: bool = False, name: str = "target"
) -> None:
    """Attach a normalized target argument to a subparser."""

    parser.add_argument(
        name,
        nargs=None if required else "?",
        type=normalize_target,
        help="target/problem name (e.g. problem_A or problem_A.cpp)",
    )


def add_conf_options(parser: argparse.ArgumentParser) -> None:
    """Attach shared `cppconf` options."""

    parser.add_argument("--build-type", choices=BUILD_TYPE_CHOICES)
    parser.add_argument("--compiler", choices=COMPILER_CHOICES)
    parser.add_argument("--timing", choices=TOGGLE_CHOICES)
    parser.add_argument("--pch", choices=PCH_CHOICES)
    parser.add_argument("--pch-rebuild", action="store_true")
    parser.add_argument(
        "--extra-conf-arg",
        action="append",
        default=[],
        help="extra raw argument forwarded to cppconf (repeatable)",
    )


def add_algorithms_dir_option(parser: argparse.ArgumentParser) -> None:
    """Attach optional centralized Algorithms directory override."""

    parser.add_argument(
        "--algorithms-dir",
        type=Path,
        default=None,
        help="override centralized Algorithms directory path",
    )


# ---------------------------- Per-subcommand setup -------------------------- #


def configure_contest(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `contest` subcommand."""

    parser.add_argument("directory", type=normalize_contest_dir)


def configure_new(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `new` subcommand."""

    parser.add_argument("name", type=normalize_target)
    parser.add_argument("--template", choices=TEMPLATE_CHOICES, default="base")
    parser.add_argument(
        "--if-missing",
        action="store_true",
        help="skip cppnew if <name>.cpp already exists",
    )


def configure_batch(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `batch` subcommand."""

    parser.add_argument("count", type=parse_positive_int)
    parser.add_argument("--template", choices=TEMPLATE_CHOICES, default="base")


def configure_delete(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `delete` subcommand."""

    parser.add_argument("names", nargs="+", type=normalize_target)
    parser.add_argument("-y", "--yes", action="store_true")
    parser.add_argument("--no-config", action="store_true")


def configure_conf(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `conf` subcommand."""

    add_conf_options(parser)


def configure_preset_conf(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `preset-conf` subcommand."""

    parser.add_argument(
        "--preset",
        choices=CMAKE_CONFIG_PRESET_CHOICES,
        default="cp-debug-gcc",
    )
    parser.add_argument(
        "--fresh",
        action="store_true",
        help="pass --fresh to cmake preset configure",
    )
    add_algorithms_dir_option(parser)


def configure_preset_build(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `preset-build` subcommand."""

    parser.add_argument(
        "--preset",
        choices=CMAKE_BUILD_PRESET_CHOICES,
        default="cp-build-debug-gcc",
    )
    parser.add_argument("--target", default=None, help="optional build target name")
    parser.add_argument(
        "--jobs",
        type=parse_positive_int,
        default=None,
        help="parallel build jobs",
    )


def configure_target_optional(parser: argparse.ArgumentParser) -> None:
    """Configure parser with optional target-only signature."""

    add_target_arg(parser)


def configure_go(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `go` subcommand."""

    add_target_arg(parser)
    parser.add_argument(
        "--force",
        action="store_true",
        help="force rebuild by touching the target source before build",
    )
    parser.add_argument("--input", type=normalize_input_name, help=INPUT_FILE_HELP)


def configure_stress(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `stress` subcommand."""

    add_target_arg(parser)
    parser.add_argument("--iterations", type=parse_positive_int, default=None)


def configure_submit(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `submit` subcommand."""

    add_target_arg(parser)
    parser.add_argument(
        "--strict",
        action="store_true",
        help="use judge-like strict submission compilation profile",
    )


def configure_test_submit(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `test-submit` subcommand."""

    add_target_arg(parser)
    parser.add_argument("--input", type=normalize_input_name, help=INPUT_FILE_HELP)
    parser.add_argument(
        "--strict",
        action="store_true",
        help="use judge-like strict submission compilation profile",
    )
    parser.add_argument("--no-generate", action="store_true")


def configure_full(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `full` subcommand."""

    add_target_arg(parser)
    parser.add_argument("--input", type=normalize_input_name, help=INPUT_FILE_HELP)


def configure_deepclean(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `deepclean` subcommand."""

    parser.add_argument("--yes", action="store_true")


def configure_doctor(parser: argparse.ArgumentParser) -> None:
    """Configure parser for `doctor` diagnostics subcommand."""

    parser.add_argument(
        "--strict",
        action="store_true",
        help="treat cppdiag non-zero exit as fatal",
    )


def configure_cycle(parser: argparse.ArgumentParser) -> None:
    """Configure parser for the high-level `cycle` workflow."""

    parser.add_argument("name", type=normalize_target)
    parser.add_argument("--template", choices=TEMPLATE_CHOICES, default="base")
    parser.add_argument("--input", type=normalize_input_name, help=INPUT_FILE_HELP)
    parser.add_argument("--configure", action="store_true", help="run cppconf first")
    add_conf_options(parser)
    parser.add_argument(
        "--new-if-missing",
        action="store_true",
        help="skip cppnew if the target source already exists",
    )
    parser.add_argument("--skip-new", action="store_true")
    parser.add_argument("--skip-go", action="store_true")
    parser.add_argument("--skip-judge", action="store_true")
    parser.add_argument("--skip-submit", action="store_true")
    parser.add_argument("--skip-submit-test", action="store_true")
    parser.add_argument(
        "--strict-submit",
        action="store_true",
        help="run submission and submission-test with judge-like strict profile",
    )
