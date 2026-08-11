"""Golden snapshot of the workflow-manager CLI surface.

The registry, the argparse declarations, the argv builders and the preset
handlers live in four separate modules, wired together only at
``register_subcommands`` time. That keeps each piece small, but it also means
a rename or a dropped import can silently remove a flag without failing lint,
typecheck, or any behavioral test.

This snapshot is that missing check. It is deliberately dumb: if you add or
remove a subcommand or an option on purpose, update the table in the same
commit — the diff is then the record of the interface change.
"""

from __future__ import annotations

import argparse

from workflow_manager_core.parser import build_parser

EXPECTED_SURFACE: dict[str, list[str]] = {
    "archive": ["--help", "-h"],
    "batch": ["--help", "--template", "-h", "<count>"],
    "build": ["--help", "-h", "<target>"],
    "check": ["--help", "-h"],
    "clean": ["--help", "-h"],
    "conf": [
        "--build-type",
        "--compiler",
        "--extra-conf-arg",
        "--help",
        "--pch",
        "--pch-rebuild",
        "--timing",
        "-h",
    ],
    "contest": ["--help", "-h", "<directory>"],
    "cycle": [
        "--build-type",
        "--compiler",
        "--configure",
        "--extra-conf-arg",
        "--help",
        "--input",
        "--new-if-missing",
        "--pch",
        "--pch-rebuild",
        "--skip-go",
        "--skip-judge",
        "--skip-new",
        "--skip-submit",
        "--skip-submit-test",
        "--strict-submit",
        "--template",
        "--timing",
        "-h",
        "<name>",
    ],
    "deepclean": ["--help", "--yes", "-h"],
    "delete": ["--help", "--no-config", "--yes", "-h", "-y", "<names>"],
    "diag": ["--help", "-h"],
    "doctor": ["--help", "--strict", "-h"],
    "forcego": ["--help", "-h", "<target>"],
    "full": ["--help", "--input", "-h", "<target>"],
    "go": ["--force", "--help", "--input", "-h", "<target>"],
    "help": ["--help", "-h"],
    "info": ["--help", "-h"],
    "init": ["--help", "-h"],
    "judge": ["--help", "-h", "<target>"],
    "new": ["--help", "--if-missing", "--template", "-h", "<name>"],
    "preset-build": ["--help", "--jobs", "--preset", "--target", "-h"],
    "preset-conf": ["--algorithms-dir", "--fresh", "--help", "--preset", "-h"],
    "run": ["--help", "-h", "<target>"],
    "stats": ["--help", "-h"],
    "stress": ["--help", "--iterations", "-h", "<target>"],
    "submit": ["--help", "--strict", "-h", "<target>"],
    "test": ["--help", "-h", "<target>"],
    "test-submit": [
        "--help",
        "--input",
        "--no-generate",
        "--strict",
        "-h",
        "<target>",
    ],
    "watch": ["--help", "-h", "<target>"],
}


def _actual_surface() -> dict[str, list[str]]:
    """Reflect the built parser into ``{subcommand: sorted option/positional names}``."""

    parser = build_parser()
    subparsers_action = next(
        action for action in parser._actions if isinstance(action, argparse._SubParsersAction)
    )

    surface: dict[str, list[str]] = {}
    for name, subparser in subparsers_action.choices.items():
        names: list[str] = []
        for action in subparser._actions:
            if action.option_strings:
                names.extend(action.option_strings)
            elif action.dest != "help":
                names.append(f"<{action.dest}>")
        surface[name] = sorted(set(names))
    return surface


def test_every_expected_subcommand_is_registered():
    assert sorted(_actual_surface()) == sorted(EXPECTED_SURFACE)


def test_each_subcommand_exposes_exactly_its_expected_flags():
    actual = _actual_surface()
    for name, expected in sorted(EXPECTED_SURFACE.items()):
        assert actual[name] == sorted(expected), f"option surface drifted for '{name}'"


def test_every_subcommand_has_a_handler_bound():
    """A spec whose handler failed to import would parse but do nothing."""

    parser = build_parser()
    subparsers_action = next(
        action for action in parser._actions if isinstance(action, argparse._SubParsersAction)
    )

    for name, subparser in subparsers_action.choices.items():
        handler = subparser.get_default("handler")
        assert callable(handler), f"'{name}' has no callable handler"
