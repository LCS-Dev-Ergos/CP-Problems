"""Multi-step workflow execution, retry policy, and JSON output marshalling.

Sits between the per-command handlers (``commands``) and the underlying
process runner (``runner`` / ``_lib.process``). Owns:

  - the live workflow state collected as each step runs;
  - the non-fatal demotion rules (some doctor checks must not fail the run);
  - the JSON serialization the ``--json`` CLI mode consumes, with optional
    ANSI stripping for downstream tooling.
"""

import argparse
import json
import os
import sys
from collections.abc import Sequence
from pathlib import Path

from _lib.process import ProcessRequest, run_capture

from .constants import DEFAULT_WORKSPACE_ROOT
from .runner import CppToolsRunner
from .types import (
    CommandResult,
    WorkflowCommandError,
    WorkflowError,
)
from .utils import is_under


class WorkflowManager:
    """Tracks workflow execution steps and exposes controlled run helpers."""

    def __init__(self, runner: CppToolsRunner, json_mode: bool, verbose: bool):
        """Initialize execution state and workspace safety notes."""
        self.runner = runner
        self.json_mode = json_mode
        self.verbose = verbose
        self.results: list[CommandResult] = []
        self.notes: list[str] = []

        workspace_root = Path(os.environ.get("CP_WORKSPACE_ROOT", str(DEFAULT_WORKSPACE_ROOT)))
        if not is_under(self.runner.cwd, workspace_root):
            self.notes.append(
                f"warning: cwd '{self.runner.cwd}' is outside CP_WORKSPACE_ROOT '{workspace_root}'"
            )

    def note(self, message: str) -> None:
        """Record an informational message and print it in text mode."""
        self.notes.append(message)
        if not self.json_mode:
            print(message)

    def run_cpp(
        self,
        function: str,
        args: Sequence[str] = (),
        check: bool = True,
        timeout: int | None = None,
        auto_confirm_deepclean: bool = False,
    ) -> CommandResult:
        """Run one cpp-tools command and store its result."""
        if self.verbose and not self.json_mode:
            print(f"[workflow] -> {' '.join([function, *args])}")

        result = self.runner.run(
            function=function,
            args=args,
            timeout=timeout,
            auto_confirm_deepclean=auto_confirm_deepclean,
        )
        self.results.append(result)

        if not self.json_mode:
            if result.stdout:
                print(result.stdout, end="")
            if result.stderr:
                print(result.stderr, end="", file=sys.stderr)

        if check and result.returncode != 0:
            raise WorkflowCommandError(result)
        return result

    def run_external(
        self,
        argv: Sequence[str],
        check: bool = True,
        timeout: int | None = None,
        env_overrides: dict[str, str] | None = None,
    ) -> CommandResult:
        """Run an external command with the same reporting semantics as cpp-tools."""
        if not argv:
            raise WorkflowError("run_external requires a non-empty command")

        if self.verbose and not self.json_mode:
            print(f"[workflow] -> {' '.join(argv)}")

        effective_timeout = float(timeout if timeout is not None else self.runner.default_timeout)
        process_result = run_capture(
            ProcessRequest(
                argv=list(argv),
                cwd=self.runner.cwd,
                env_overrides=dict(env_overrides or {}),
                timeout=effective_timeout,
            )
        )
        result = CommandResult(
            function=argv[0],
            args=tuple(argv[1:]),
            cwd=str(self.runner.cwd),
            returncode=process_result.returncode,
            duration_ms=process_result.duration_ms,
            stdout=process_result.stdout,
            stderr=process_result.stderr,
            timed_out=process_result.timed_out,
        )

        self.results.append(result)

        if not self.json_mode:
            if result.stdout:
                print(result.stdout, end="")
            if result.stderr:
                print(result.stderr, end="", file=sys.stderr)

        if check and result.returncode != 0:
            raise WorkflowCommandError(result)
        return result

    def emit_json(self, status: str) -> None:
        """Emit complete workflow results as JSON."""
        payload = {
            "status": status,
            "cwd": str(self.runner.cwd),
            "cp_tools_script": str(self.runner.script_path),
            "notes": self.notes,
            "steps": [r.to_dict(strip_ansi=True) for r in self.results],
        }
        print(json.dumps(payload, ensure_ascii=False, indent=2))

    def emit_summary(self) -> None:
        """Print a compact final status line in text mode."""
        if self.json_mode or not self.results:
            return
        total_ms = sum(r.duration_ms for r in self.results)
        failed = [r for r in self.results if r.failed]
        status = "OK" if not failed else "FAILED"
        print(
            f"[workflow] {status} | steps={len(self.results)} | "
            f"failed={len(failed)} | total={total_ms}ms"
        )


def run_step_with_policy(
    manager: WorkflowManager,
    ns: argparse.Namespace,
    function: str,
    args: Sequence[str] = (),
    auto_confirm_deepclean: bool = False,
) -> CommandResult:
    """Run one cpp-tools step honoring `--continue-on-error` policy."""

    check = not getattr(ns, "continue_on_error", False)
    result = manager.run_cpp(
        function=function,
        args=args,
        check=check,
        auto_confirm_deepclean=auto_confirm_deepclean,
    )
    if not check and result.failed:
        manager.note(
            f"[workflow] step failed but continuing: {function} (exit {result.returncode})"
        )
    return result


def run_external_step_with_policy(
    manager: WorkflowManager,
    ns: argparse.Namespace,
    argv: Sequence[str],
    env_overrides: dict[str, str] | None = None,
) -> CommandResult:
    """Run one external step honoring `--continue-on-error` policy."""

    check = not getattr(ns, "continue_on_error", False)
    result = manager.run_external(
        argv=argv,
        check=check,
        env_overrides=env_overrides,
    )
    if not check and result.failed:
        manager.note(
            f"[workflow] step failed but continuing: {' '.join(argv)} (exit {result.returncode})"
        )
    return result
