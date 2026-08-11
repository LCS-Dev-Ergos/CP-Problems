"""Smoke tests for the ``workflow_manager`` CLI surface and zsh wrapper.

Verifies the user-facing argparse contract (subcommand visibility,
argument validation, JSON output shape) plus the runtime details of
``CppToolsRunner``: login-vs-non-login shell selection driven by
``CP_WORKFLOW_NO_LOGIN``, and the externalized ``_wrapper.zsh`` template
loading with allowlist-injection safety.
"""

import json
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest
from collections.abc import Callable
from pathlib import Path

import pytest

SCRIPTS_DIR = Path(__file__).resolve().parents[1]
SCRIPT = SCRIPTS_DIR / "workflow_manager.py"

from workflow_manager_core import runner as runner_module  # noqa: E402
from workflow_manager_core.constants import ALLOWED_FUNCTIONS  # noqa: E402
from workflow_manager_core.runner import CppToolsRunner  # noqa: E402
from workflow_manager_core.types import WorkflowError  # noqa: E402


class WorkflowManagerCliSmokeTests(unittest.TestCase):
    def _run(self, *args: str) -> subprocess.CompletedProcess[str]:
        """Execute workflow_manager.py with provided arguments."""

        return subprocess.run(
            [sys.executable, str(SCRIPT), *args],
            capture_output=True,
            text=True,
            check=False,
        )

    def test_root_help_includes_cycle_and_excludes_exec(self) -> None:
        """Root help should expose cycle and not mention removed exec."""

        result = self._run("--help")
        self.assertEqual(result.returncode, 0)
        self.assertIn("cycle", result.stdout)
        self.assertNotIn(" exec ", result.stdout)

    def test_exec_subcommand_is_removed(self) -> None:
        """Removed `exec` subcommand should be rejected by argparse."""

        result = self._run("exec")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("invalid choice", result.stderr)

    def test_cycle_help_is_available(self) -> None:
        """Cycle subcommand help should render successfully."""

        result = self._run("cycle", "--help")
        self.assertEqual(result.returncode, 0)
        self.assertIn("--strict-submit", result.stdout)

    def test_timeout_rejects_non_positive_values(self) -> None:
        """Top-level timeout should fail fast on invalid non-positive values."""

        result = self._run("--timeout", "0", "help")
        self.assertEqual(result.returncode, 2)
        self.assertIn("expected a positive integer", result.stderr)

    def test_preset_build_jobs_reject_non_positive_values(self) -> None:
        """Preset build jobs should be validated by argparse before execution."""

        result = self._run("preset-build", "--jobs", "0")
        self.assertEqual(result.returncode, 2)
        self.assertIn("expected a positive integer", result.stderr)

    @unittest.skipIf(shutil.which("zsh") is None, "zsh is required for runtime smoke")
    def test_json_help_command_with_stub_cp_tools(self) -> None:
        """JSON mode should serialize successful delegated command output."""

        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            cp_tools = tmp / "competitive_stub.sh"
            cp_tools.write_text(
                textwrap.dedent(
                    """\
                    cpphelp() {
                      print -- "stub-help-valid"
                    }
                    """
                ),
                encoding="utf-8",
            )

            result = self._run(
                "--json",
                "--cp-tools-script",
                str(cp_tools),
                "--cwd",
                str(tmp),
                "help",
            )
            self.assertEqual(result.returncode, 0, msg=result.stderr)

            payload = json.loads(result.stdout)
            self.assertEqual(payload.get("status"), "valid")
            self.assertEqual(len(payload.get("steps", [])), 1)
            self.assertEqual(payload["steps"][0]["function"], "cpphelp")
            self.assertIn("stub-help-valid", payload["steps"][0]["stdout"])


# Function-style tests for ``CppToolsRunner`` login-shell selection.
# Drives the runner with the shared ``make_stub_cpp_tools`` + ``tmp_path``
# fixtures so the previous ``_make_runner`` + ``addCleanup`` dance and the
# manual ``os.environ`` push/pop are no longer needed.


_zsh_required = pytest.mark.skipif(
    shutil.which("zsh") is None,
    reason="zsh is required to construct CppToolsRunner",
)


def _build_runner(
    stub_factory: Callable[..., Path],
    tmp_path: Path,
    *,
    login_shell: bool | None = None,
) -> CppToolsRunner:
    """Build a ``CppToolsRunner`` rooted at ``tmp_path`` with a minimal stub."""

    stub = stub_factory(cpphelp=":")
    return CppToolsRunner(
        cp_tools_script=stub,
        cwd=tmp_path,
        default_timeout=5,
        login_shell=login_shell,
    )


@_zsh_required
def test_default_runner_uses_login_shell(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
    make_stub_cpp_tools: Callable[..., Path],
) -> None:
    """Without env override or explicit flag the runner stays on login zsh."""

    monkeypatch.delenv("CP_WORKFLOW_NO_LOGIN", raising=False)
    runner = _build_runner(make_stub_cpp_tools, tmp_path)
    assert runner.login_shell is True


@_zsh_required
def test_env_flag_disables_login_shell(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
    make_stub_cpp_tools: Callable[..., Path],
) -> None:
    """``CP_WORKFLOW_NO_LOGIN=1`` should drop the ``-l`` flag for ``zsh -c``."""

    monkeypatch.setenv("CP_WORKFLOW_NO_LOGIN", "1")
    runner = _build_runner(make_stub_cpp_tools, tmp_path)
    assert runner.login_shell is False


@_zsh_required
def test_explicit_argument_beats_env_flag(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
    make_stub_cpp_tools: Callable[..., Path],
) -> None:
    """Explicit ``login_shell=True`` must override the env opt-out."""

    monkeypatch.setenv("CP_WORKFLOW_NO_LOGIN", "1")
    runner = _build_runner(make_stub_cpp_tools, tmp_path, login_shell=True)
    assert runner.login_shell is True


@_zsh_required
def test_auto_confirm_deepclean_is_not_inherited_from_the_environment(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
    make_stub_cpp_tools: Callable[..., Path],
) -> None:
    """An ambient ``CP_AUTO_CONFIRM_DEEPCLEAN=1`` must not reach the wrapper.

    ``run_capture`` layers overrides onto a copy of ``os.environ``, so the
    runner has to write the flag on every call rather than only when it is
    enabled — otherwise the surrounding shell could pre-answer a destructive
    confirmation the Python caller never asked for.
    """

    monkeypatch.setenv("CP_AUTO_CONFIRM_DEEPCLEAN", "1")
    stub = make_stub_cpp_tools(cppinfo='print -- "seen=${CP_AUTO_CONFIRM_DEEPCLEAN:-unset}"')
    runner = CppToolsRunner(
        cp_tools_script=stub,
        cwd=tmp_path,
        default_timeout=20,
        login_shell=False,
    )

    result = runner.run("cppinfo", auto_confirm_deepclean=False)
    assert result.returncode == 0
    assert "seen=0" in result.stdout


@_zsh_required
def test_auto_confirm_deepclean_is_forwarded_when_requested(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
    make_stub_cpp_tools: Callable[..., Path],
) -> None:
    """The opt-in path must still reach the wrapper from a clean environment."""

    monkeypatch.delenv("CP_AUTO_CONFIRM_DEEPCLEAN", raising=False)
    stub = make_stub_cpp_tools(cppinfo='print -- "seen=${CP_AUTO_CONFIRM_DEEPCLEAN:-unset}"')
    runner = CppToolsRunner(
        cp_tools_script=stub,
        cwd=tmp_path,
        default_timeout=20,
        login_shell=False,
    )

    result = runner.run("cppinfo", auto_confirm_deepclean=True)
    assert result.returncode == 0
    assert "seen=1" in result.stdout


class WrapperTemplateTests(unittest.TestCase):
    """Cover the externalized zsh wrapper loaded via ``importlib.resources``."""

    def test_template_loads_with_placeholder(self) -> None:
        """The on-disk wrapper template must expose the substitution marker exactly once."""

        runner_module._wrapper_template.cache_clear()
        template = runner_module._wrapper_template()
        self.assertEqual(
            template.count(runner_module._ALLOWED_CASES_PLACEHOLDER),
            1,
            "placeholder must appear exactly once or replace() would rewrite comments",
        )
        self.assertIn('source "$1"', template)

    def test_render_substitutes_every_allowlisted_name(self) -> None:
        """Every entry in ``ALLOWED_FUNCTIONS`` must appear as a ``name) ;;`` arm."""

        rendered = runner_module._render_wrapper(frozenset(ALLOWED_FUNCTIONS))
        self.assertNotIn(runner_module._ALLOWED_CASES_PLACEHOLDER, rendered)
        for name in ALLOWED_FUNCTIONS:
            self.assertIn(f"  {name}) ;;", rendered)

    def test_render_rejects_shell_metacharacters(self) -> None:
        """Allowlist entries that break the identifier shape must fail fast."""

        bad = frozenset({"cppbuild", "rm -rf /"})
        with self.assertRaises(WorkflowError) as ctx:
            runner_module._render_wrapper(bad)
        self.assertIn("rm -rf /", str(ctx.exception))

    def test_template_read_is_cached(self) -> None:
        """``_wrapper_template`` is wrapped in ``functools.cache``."""

        runner_module._wrapper_template.cache_clear()
        runner_module._wrapper_template()
        runner_module._wrapper_template()
        info = runner_module._wrapper_template.cache_info()
        self.assertEqual(info.misses, 1)
        self.assertGreaterEqual(info.hits, 1)


if __name__ == "__main__":
    unittest.main()
