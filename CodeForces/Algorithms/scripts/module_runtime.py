"""Shared compiler/toolchain discovery for the module tester and verifier.

This module owns the public API that ``module_tester`` and ``module_verify``
were previously sharing through underscore-prefixed cross-imports. Anything
that touches ``.template_config.json`` or selects the C++ compiler used to
build template/module probes lives here.

The ``Compiler`` Protocol gives downstream code a narrow seam for unit tests
that want to swap in a stub instead of spawning ``g++``.
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol, TypedDict, runtime_checkable

from _lib.process import ProcessRequest, run_capture

__all__ = [
    "DEFAULT_PROBE_JOBS",
    "DEFAULT_PROBE_TIMEOUT_SECONDS",
    "MAX_PROBE_JOBS",
    "CompilationOutcome",
    "Compiler",
    "CompilerInvocation",
    "TemplateConfigPayload",
    "build_compiler_flags",
    "compiler_supports_std_headers",
    "load_template_config",
    "parse_jobs_argument",
    "select_compiler",
]

DEFAULT_PROBE_TIMEOUT_SECONDS: float = 10.0
MAX_PROBE_JOBS: int = 8
DEFAULT_PROBE_JOBS: int = min(os.cpu_count() or 1, MAX_PROBE_JOBS)
_COMPILER_CANDIDATES: tuple[str, ...] = ("g++-16", "g++-15", "g++-14", "g++-13", "g++", "c++")


def parse_jobs_argument(raw: str) -> int:
    """Argparse ``type=`` validator: positive integer for ``--jobs``.

    Rejects zero, negatives, and non-integers with the same error shape
    other CLI validators in this codebase produce.
    """

    try:
        parsed = int(raw)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(
            f"expected a positive integer for --jobs, got {raw!r}"
        ) from exc
    if parsed < 1:
        raise argparse.ArgumentTypeError(f"expected a positive integer for --jobs, got {parsed}")
    return parsed


class TemplateConfigPayload(TypedDict, total=False):
    """Validated subset of `.template_config.json` consumed by the tester."""

    compiler: str
    compiler_flags: list[str]


@dataclass(frozen=True, slots=True)
class CompilationOutcome:
    """Result of compiling one generated test translation unit."""

    success: bool
    error: str | None = None
    elapsed_seconds: float | None = None


@runtime_checkable
class Compiler(Protocol):
    """Narrow surface a compile-only probe needs.

    Implementations are typically a thin wrapper over ``subprocess`` (see
    ``CompilerInvocation``) but can be replaced in tests with an in-process
    stub. ``runtime_checkable`` lets ``isinstance(obj, Compiler)`` work for
    duck-typed substitutes without inheritance.
    """

    def compile_source(self, source: str, *, timeout_seconds: float = ...) -> CompilationOutcome:
        """Compile ``source`` and return a normalized outcome."""


@dataclass(frozen=True, slots=True)
class CompilerInvocation:
    """Concrete ``Compiler`` that shells out to ``g++``/``clang++``."""

    compiler: str
    compiler_flags: tuple[str, ...]
    include_dir: Path

    def compile_source(
        self, source: str, *, timeout_seconds: float = DEFAULT_PROBE_TIMEOUT_SECONDS
    ) -> CompilationOutcome:
        """Compile one source string and return normalized result metadata."""

        with tempfile.TemporaryDirectory() as tmpdir:
            source_file = Path(tmpdir) / "probe.cpp"
            source_file.write_text(source, encoding="utf-8")
            try:
                result = run_capture(
                    ProcessRequest(
                        argv=[
                            self.compiler,
                            *self.compiler_flags,
                            f"-I{self.include_dir}",
                            str(source_file),
                        ],
                        timeout=timeout_seconds,
                    )
                )
            except OSError as exc:
                return CompilationOutcome(
                    success=False,
                    error=f"Unable to execute compiler '{self.compiler}': {exc}",
                )

            if result.timed_out:
                return CompilationOutcome(success=False, error="Compilation timeout")
            elapsed = result.duration_ms / 1000.0
            if result.returncode == 0:
                return CompilationOutcome(success=True, elapsed_seconds=elapsed)
            return CompilationOutcome(
                success=False,
                error=result.stderr,
                elapsed_seconds=elapsed,
            )


def load_template_config(templates_dir: Path) -> TemplateConfigPayload:
    """Load and lightly validate `.template_config.json` if present."""

    config_path = templates_dir.parent / ".template_config.json"
    if not config_path.is_file():
        return {}

    try:
        payload = json.loads(config_path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return {}

    if not isinstance(payload, dict):
        return {}

    config: TemplateConfigPayload = {}
    compiler = payload.get("compiler")
    if isinstance(compiler, str) and compiler.strip():
        config["compiler"] = compiler.strip()

    compiler_flags = payload.get("compiler_flags")
    if isinstance(compiler_flags, list) and all(isinstance(flag, str) for flag in compiler_flags):
        config["compiler_flags"] = list(compiler_flags)

    return config


def build_compiler_flags(config: TemplateConfigPayload) -> tuple[str, ...]:
    """Build compiler flags from config plus tester defaults."""

    config_flags = list(config.get("compiler_flags", ()))
    if not any(flag.startswith("-std=") for flag in config_flags):
        config_flags.insert(0, "-std=c++23")
    if "-fsyntax-only" not in config_flags:
        config_flags.append("-fsyntax-only")
    return tuple(config_flags)


def compiler_supports_std_headers(compiler: str) -> bool:
    """Quick probe to ensure ``compiler`` accepts ``<bits/stdc++.h>``."""

    with tempfile.TemporaryDirectory() as tmpdir:
        probe_file = Path(tmpdir) / "probe.cpp"
        probe_file.write_text(
            "#include <bits/stdc++.h>\nint main() { return 0; }\n",
            encoding="utf-8",
        )
        try:
            result = run_capture(
                ProcessRequest(
                    argv=[compiler, "-std=c++23", "-fsyntax-only", str(probe_file)],
                    timeout=5.0,
                )
            )
        except OSError:
            return False
        return not result.timed_out and result.returncode == 0


def select_compiler(config: TemplateConfigPayload) -> str:
    """Pick the most suitable compiler for CP templates.

    Order: configured override → known modern g++ variants → fallback ``c++``.
    Each candidate is probed via :func:`compiler_supports_std_headers` to
    ensure ``<bits/stdc++.h>`` is available — the C++ codebase relies on it.
    """

    configured = config.get("compiler")
    candidates: list[str | None] = [configured, *_COMPILER_CANDIDATES]
    seen: set[str] = set()
    for candidate in candidates:
        if not candidate or candidate in seen:
            continue
        seen.add(candidate)
        resolved = shutil.which(candidate)
        if resolved and compiler_supports_std_headers(resolved):
            return resolved
    return configured or "g++"
