"""High-level multi-step workflows exposed as CLI subcommands.

``handle_cycle`` runs the canonical "configure → build → test" loop for a
round, ``handle_doctor`` runs the environment diagnostics (toolchain
versions, cpp-tools discovery, profile registry sanity), and any future
composite recipe lives here too.

Each handler is a thin facade around ``orchestration`` and ``runner``, so
the bulk of the logic stays testable without exercising argparse.
"""

from __future__ import annotations

import argparse
import shutil

from .argbuilders import build_cppconf_args
from .orchestration import WorkflowManager, run_step_with_policy
from .types import WorkflowCommandError
from .utils import find_existing_target_source


def handle_doctor(manager: WorkflowManager, ns: argparse.Namespace) -> None:
    """Run diagnostics around cpp-tools health and environment readiness."""

    manager.note(f"[workflow] cp-tools script: {manager.runner.script_path}")
    manager.note(f"[workflow] cwd: {manager.runner.cwd}")
    manager.note("[workflow] running diagnostics suite...")

    timeout_tool = shutil.which("timeout") or shutil.which("gtimeout")
    if timeout_tool:
        manager.note(f"[workflow] timeout tool: {timeout_tool}")
    else:
        manager.note(
            "[workflow] warning: neither timeout nor gtimeout found in PATH "
            "(install coreutils if needed)."
        )

    run_step_with_policy(manager, ns, "cpphelp")
    run_step_with_policy(manager, ns, "cppinfo")

    diag_result = manager.run_cpp("cppdiag", check=False)
    if diag_result.returncode != 0:
        manager.note(f"[workflow] warning: cppdiag exited with {diag_result.returncode}")
        if ns.strict:
            raise WorkflowCommandError(diag_result)
        manager.results[-1] = diag_result.as_non_fatal()

    run_step_with_policy(manager, ns, "cppcheck")


def handle_cycle(manager: WorkflowManager, ns: argparse.Namespace) -> None:
    """Run the default CP lifecycle with optional skip flags."""

    input_name = ns.input

    if ns.configure:
        run_step_with_policy(manager, ns, "cppconf", build_cppconf_args(ns))

    if not ns.skip_new:
        target_file = find_existing_target_source(manager.runner.cwd, ns.name)
        if ns.new_if_missing and target_file is not None:
            manager.note(f"[workflow] {target_file.name} already exists, skipping cppnew")
        else:
            run_step_with_policy(manager, ns, "cppnew", [ns.name, ns.template])

    if not ns.skip_go:
        go_args = [ns.name]
        if input_name:
            go_args.append(input_name)
        run_step_with_policy(manager, ns, "cppgo", go_args)

    if not ns.skip_judge:
        run_step_with_policy(manager, ns, "cppjudge", [ns.name])

    if not ns.skip_submit:
        submit_args = [ns.name] if not ns.strict_submit else ["--strict", ns.name]
        run_step_with_policy(manager, ns, "cppsubmit", submit_args)
        if not ns.skip_submit_test:
            test_args: list[str] = []
            if ns.strict_submit:
                test_args.append("--strict")
            test_args.extend(["--no-generate", ns.name])
            if input_name:
                test_args.append(input_name)
            run_step_with_policy(manager, ns, "cpptestsubmit", test_args)
    elif not ns.skip_submit_test:
        test_args = [ns.name]
        if ns.strict_submit:
            test_args.insert(0, "--strict")
        if input_name:
            test_args.append(input_name)
        run_step_with_policy(manager, ns, "cpptestsubmit", test_args)
