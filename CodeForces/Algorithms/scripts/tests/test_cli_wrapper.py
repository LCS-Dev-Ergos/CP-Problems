"""Contract for the shared CLI entry-point wrapper.

``run_cli`` decides what reaches the user when a script fails. The line it
draws matters in both directions: a configuration error must become one
readable line, and a genuine defect must keep its traceback rather than be
flattened into "Error: ..." and an exit code.
"""

from __future__ import annotations

import argparse

import pytest

from _lib.cli import EXIT_ERROR, EXIT_INTERRUPTED, run_cli


def test_returns_the_exit_code_from_main():
    assert run_cli(lambda: 0) == 0
    assert run_cli(lambda: 3) == 3


def test_none_is_treated_as_success():
    """Entry points that only print need no sentinel return."""

    assert run_cli(lambda: None) == 0


def test_value_error_becomes_a_message_and_exit_one(capsys: pytest.CaptureFixture[str]):
    def main() -> int:
        raise ValueError("manifest is missing 'name'")

    assert run_cli(main) == EXIT_ERROR
    assert capsys.readouterr().err.strip() == "Error: manifest is missing 'name'"


def test_os_error_becomes_a_message_and_exit_one(capsys: pytest.CaptureFixture[str]):
    def main() -> int:
        raise FileNotFoundError("no such file: profiles.toml")

    assert run_cli(main) == EXIT_ERROR
    assert "profiles.toml" in capsys.readouterr().err


def test_keyboard_interrupt_uses_the_conventional_code(capsys: pytest.CaptureFixture[str]):
    def main() -> int:
        raise KeyboardInterrupt

    assert run_cli(main) == EXIT_INTERRUPTED
    assert "Interrupted." in capsys.readouterr().err


def test_unexpected_exceptions_keep_their_traceback():
    """A bug must not be disguised as a configuration error."""

    def main() -> int:
        raise RuntimeError("this is a defect, not bad input")

    with pytest.raises(RuntimeError, match="this is a defect"):
        run_cli(main)


def test_also_catch_extends_the_handled_set(capsys: pytest.CaptureFixture[str]):
    class BenchmarkError(RuntimeError):
        pass

    def main() -> int:
        raise BenchmarkError("cmake --preset failed")

    with pytest.raises(BenchmarkError):
        run_cli(main)

    assert run_cli(main, also_catch=(BenchmarkError,)) == EXIT_ERROR
    assert "cmake --preset failed" in capsys.readouterr().err


def test_system_exit_propagates_so_argparse_still_works():
    """``--help`` and invalid-argument exits must reach the interpreter."""

    def main() -> int:
        argparse.ArgumentParser().parse_args(["--help"])
        return 0

    with pytest.raises(SystemExit) as exc:
        run_cli(main)
    assert exc.value.code == 0
