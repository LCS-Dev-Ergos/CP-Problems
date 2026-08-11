"""Subprocess runner that delegates cpp-tools functions through ``zsh``.

``CppToolsRunner`` resolves the user's ``competitive.sh`` (via env or
discovery) and invokes whitelisted shell functions through an allowlist,
piping every call through ``_lib.process.run_capture`` for consistent
timeout + capture semantics.

Login-shell behavior is controlled by the ``login_shell`` argument and the
``CP_WORKFLOW_NO_LOGIN`` env opt-out — useful in CI where sourcing the
full ``~/.zprofile`` adds latency without any benefit.
"""

import functools
import os
import re
import shutil
from collections.abc import Sequence
from importlib import resources
from pathlib import Path

from _lib.process import ProcessRequest, run_capture

from .constants import ALLOWED_FUNCTIONS
from .types import CommandResult, WorkflowError

_TRUTHY_ENV_VALUES = frozenset({"1", "true", "yes", "on"})
_WRAPPER_RESOURCE = "_wrapper.zsh"
_ALLOWED_CASES_PLACEHOLDER = "{{ALLOWED_FUNCTIONS_CASES}}"
_FUNCTION_NAME_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


@functools.cache
def _wrapper_template() -> str:
    """Read the externalized zsh wrapper template once per process."""

    return resources.files(__package__).joinpath(_WRAPPER_RESOURCE).read_text(encoding="utf-8")


def _render_wrapper(allowed_functions: frozenset[str]) -> str:
    """Substitute the allowlist case-arms into the wrapper template.

    Names are validated against ``_FUNCTION_NAME_RE`` so a malformed entry
    in :data:`constants.ALLOWED_FUNCTIONS` (e.g. one containing shell
    metacharacters) fails fast at runner construction instead of producing
    a wrapper that interprets the payload at exec time.
    """

    invalid = sorted(name for name in allowed_functions if not _FUNCTION_NAME_RE.fullmatch(name))
    if invalid:
        raise WorkflowError(f"non-identifier allowlist entries rejected: {', '.join(invalid)}")
    template = _wrapper_template()
    if template.count(_ALLOWED_CASES_PLACEHOLDER) != 1:
        raise WorkflowError(
            f"wrapper template must contain {_ALLOWED_CASES_PLACEHOLDER!r} exactly once"
        )
    cases = "\n".join(f"  {name}) ;;" for name in sorted(allowed_functions))
    return template.replace(_ALLOWED_CASES_PLACEHOLDER, cases)


def _env_flag_enabled(name: str, *, default: bool = False) -> bool:
    """Read a boolean-like environment variable with consistent semantics."""

    raw = os.environ.get(name)
    if raw is None:
        return default
    return raw.strip().lower() in _TRUTHY_ENV_VALUES


class CppToolsRunner:
    """Executes allowlisted cpp-tools functions in a controlled shell wrapper."""

    def __init__(
        self,
        cp_tools_script: Path,
        cwd: Path,
        default_timeout: int,
        quiet_load: bool = True,
        login_shell: bool | None = None,
    ):
        """Validate runtime prerequisites and cache immutable runner settings.

        ``login_shell`` controls whether the wrapper invokes ``zsh -lc`` (login,
        sources ``~/.zshenv`` and ``~/.zprofile``) or the lighter ``zsh -c``.
        When ``None`` (default) the value is taken from
        ``CP_WORKFLOW_NO_LOGIN`` (truthy → non-login) and falls back to login
        for backward compatibility with cpp-tools functions defined in
        interactive rc files.
        """
        if shutil.which("zsh") is None:
            raise WorkflowError("zsh is required but was not found in PATH")
        if not cp_tools_script.is_file():
            raise WorkflowError(f"cpp-tools script not found: {cp_tools_script}")
        if not cwd.exists() or not cwd.is_dir():
            raise WorkflowError(f"invalid working directory: {cwd}")

        self.script_path = cp_tools_script.resolve()
        self.cwd = cwd.resolve()
        self.default_timeout = default_timeout
        self.quiet_load = quiet_load
        if login_shell is None:
            login_shell = not _env_flag_enabled("CP_WORKFLOW_NO_LOGIN")
        self.login_shell = login_shell
        self._wrapper_script = _render_wrapper(frozenset(ALLOWED_FUNCTIONS))

    def run(
        self,
        function: str,
        args: Sequence[str] = (),
        timeout: int | None = None,
        auto_confirm_deepclean: bool = False,
    ) -> CommandResult:
        """Execute one cpp-tools function and return normalized command metadata."""

        if function not in ALLOWED_FUNCTIONS:
            raise WorkflowError(f"function not allowlisted: {function}")

        shell_flag = "-lc" if self.login_shell else "-c"
        argv: list[str] = [
            "zsh",
            shell_flag,
            self._wrapper_script,
            "workflow_manager",
            str(self.script_path),
            function,
            *args,
        ]
        # Both flags are written unconditionally. ``run_capture`` layers
        # overrides over a copy of ``os.environ``, so merely *omitting* the
        # auto-confirm key would let an inherited ``CP_AUTO_CONFIRM_DEEPCLEAN=1``
        # reach the wrapper and answer a destructive prompt the Python caller
        # never authorized. The decision belongs to this call, not the ambient
        # environment.
        env_overrides: dict[str, str] = {
            "CP_QUIET_LOAD": "1" if self.quiet_load else "0",
            "CP_AUTO_CONFIRM_DEEPCLEAN": "1" if auto_confirm_deepclean else "0",
        }

        effective_timeout = float(timeout if timeout is not None else self.default_timeout)
        result = run_capture(
            ProcessRequest(
                argv=argv,
                cwd=self.cwd,
                env_overrides=env_overrides,
                timeout=effective_timeout,
            )
        )
        return CommandResult(
            function=function,
            args=tuple(args),
            cwd=str(self.cwd),
            returncode=result.returncode,
            duration_ms=result.duration_ms,
            stdout=result.stdout,
            stderr=result.stderr,
            timed_out=result.timed_out,
        )
