"""Translations from a parsed ``argparse.Namespace`` into a cpp-tools argv.

Every function here has the same shape — ``Namespace -> list[str]`` — and none
of them touch the runner, the manager, or the filesystem. That is the point:
this is the one layer of the workflow CLI that can be tested by calling it
with a plain namespace and comparing lists, so it is kept free of imports
from the rest of the package.
"""

from __future__ import annotations

import argparse

__all__ = [
    "batch_args",
    "build_args",
    "build_cppconf_args",
    "conf_args",
    "delete_args",
    "forcego_args",
    "full_args",
    "go_args",
    "new_args",
    "no_args",
    "run_args",
    "stress_args",
    "submit_args",
    "target_args",
    "test_submit_args",
]


def no_args(_: argparse.Namespace) -> list[str]:
    """Return an empty argument vector."""

    return []


def target_args(ns: argparse.Namespace) -> list[str]:
    """Build args containing an optional target name."""

    return [ns.target] if ns.target else []


def build_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cppbuild`."""

    return target_args(ns)


def run_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cpprun`."""

    return target_args(ns)


def go_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cppgo`."""

    args: list[str] = []
    if ns.force:
        args.append("--force")
    if ns.target:
        args.append(ns.target)
    if ns.input:
        args.append(ns.input)
    return args


def forcego_args(ns: argparse.Namespace) -> list[str]:
    """Build args for force-run flow."""

    args: list[str] = ["--force"]
    if ns.target:
        args.append(ns.target)
    return args


def stress_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cppstress`."""

    args: list[str] = []
    if ns.target:
        args.append(ns.target)
    if ns.iterations:
        args.append(str(ns.iterations))
    return args


def submit_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cppsubmit`."""

    args: list[str] = []
    if getattr(ns, "strict", False):
        args.append("--strict")
    if ns.target:
        args.append(ns.target)
    return args


def test_submit_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cpptestsubmit`."""

    args: list[str] = []
    if ns.strict:
        args.append("--strict")
    if ns.no_generate:
        args.append("--no-generate")
    if ns.target:
        args.append(ns.target)
    if ns.input:
        args.append(ns.input)
    return args


def full_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cppfull`."""

    args: list[str] = []
    if ns.target:
        args.append(ns.target)
    if ns.input:
        args.append(ns.input)
    return args


def new_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cppnew`."""

    return [ns.name, ns.template]


def batch_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cppbatch`."""

    return [str(ns.count), ns.template]


def delete_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cppdelete`."""

    args: list[str] = []
    if ns.yes:
        args.append("-y")
    if ns.no_config:
        args.append("--no-config")
    args.extend(ns.names)
    return args


def build_cppconf_args(ns: argparse.Namespace) -> list[str]:
    """Translate CLI namespace fields into raw `cppconf` arguments."""

    args: list[str] = []
    if ns.build_type:
        args.append(ns.build_type)
    if ns.compiler:
        args.append(ns.compiler)
    if ns.timing:
        args.append(f"timing={ns.timing}")
    if ns.pch:
        args.append(f"pch={ns.pch}")
    if ns.pch_rebuild:
        args.append("pch-rebuild")
    if ns.extra_conf_arg:
        args.extend(ns.extra_conf_arg)
    return args


def conf_args(ns: argparse.Namespace) -> list[str]:
    """Build args for `cppconf`."""

    return build_cppconf_args(ns)
