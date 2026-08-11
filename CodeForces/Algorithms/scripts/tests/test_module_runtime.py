"""Unit tests for the shared probe runtime behind both module harnesses.

``module_runtime`` used to be covered only through ``parse_jobs_argument``;
everything else was exercised indirectly by running real compilers. These
tests pin the parts that decide what a probe *result means* — the three
distinct compile failure modes, ordering under parallelism, and the toolchain
selection fallbacks — without spawning ``g++``.
"""

from __future__ import annotations

import json
import threading
import time
from pathlib import Path
from unittest import mock

import pytest

import module_runtime
from _lib.process import ProcessResult
from module_runtime import (
    ToolingConfig,
    build_compiler_flags,
    load_tooling_config,
    resolve_dir_argument,
    run_compile,
    run_parallel_ordered,
    select_compiler,
    write_json_report,
)


def _process_result(*, returncode: int = 0, stderr: str = "", timed_out: bool = False):
    return ProcessResult(
        argv=("g++",),
        cwd="",
        returncode=returncode,
        duration_ms=250,
        stdout="",
        stderr=stderr,
        timed_out=timed_out,
    )


# ------------------------------- run_compile -------------------------------- #


def test_run_compile_reports_success_with_elapsed_time():
    with mock.patch.object(module_runtime, "run_capture", return_value=_process_result()):
        outcome = run_compile(["g++", "probe.cpp"])

    assert outcome.success is True
    assert outcome.error is None
    assert outcome.elapsed_seconds == pytest.approx(0.25)


def test_run_compile_surfaces_compiler_stderr_on_failure():
    failure = _process_result(returncode=1, stderr="probe.cpp:3:1: error: boom")
    with mock.patch.object(module_runtime, "run_capture", return_value=failure):
        outcome = run_compile(["g++", "probe.cpp"])

    assert outcome.success is False
    assert "boom" in (outcome.error or "")
    assert outcome.elapsed_seconds == pytest.approx(0.25)


def test_run_compile_distinguishes_timeout_from_a_failed_compile():
    """A timeout is not a compile error; the message must say so."""

    with mock.patch.object(
        module_runtime, "run_capture", return_value=_process_result(timed_out=True)
    ):
        outcome = run_compile(["g++", "probe.cpp"], timeout_seconds=0.1)

    assert outcome.success is False
    assert outcome.error == "Compilation timeout"


def test_run_compile_reports_a_missing_executable_by_name():
    """``OSError`` means the toolchain is wrong, not that the source is bad."""

    with mock.patch.object(module_runtime, "run_capture", side_effect=OSError("not found")):
        outcome = run_compile(["/nonexistent/g++", "probe.cpp"], compiler_label="g++-99")

    assert outcome.success is False
    assert "Unable to execute compiler 'g++-99'" in (outcome.error or "")
    assert outcome.elapsed_seconds is None


def test_run_compile_falls_back_to_argv0_as_the_label():
    with mock.patch.object(module_runtime, "run_capture", side_effect=OSError("nope")):
        outcome = run_compile(["clang++", "probe.cpp"])

    assert "'clang++'" in (outcome.error or "")


# --------------------------- run_parallel_ordered --------------------------- #


def test_parallel_results_follow_submission_order_not_completion_order():
    """The slowest item is submitted first, so completion order is reversed."""

    def worker(item: int) -> int:
        time.sleep(0.02 * (5 - item))
        return item * 10

    assert run_parallel_ordered([0, 1, 2, 3, 4], worker, jobs=5) == [0, 10, 20, 30, 40]


def test_single_job_runs_inline_on_the_caller_thread():
    seen: set[int] = set()

    def worker(item: int) -> int:
        seen.add(threading.get_ident())
        return item

    run_parallel_ordered(list(range(6)), worker, jobs=1)
    assert seen == {threading.get_ident()}


def test_multiple_jobs_fan_out_across_threads():
    seen: set[int] = set()
    lock = threading.Lock()

    def worker(item: int) -> int:
        with lock:
            seen.add(threading.get_ident())
        time.sleep(0.03)
        return item

    run_parallel_ordered(list(range(8)), worker, jobs=4)
    assert len(seen) > 1


def test_on_result_fires_once_per_item_with_its_own_result():
    observed: list[tuple[int, int]] = []
    lock = threading.Lock()

    def record(item: int, result: int) -> None:
        with lock:
            observed.append((item, result))

    run_parallel_ordered([1, 2, 3], lambda i: i * 2, jobs=3, on_result=record)

    assert sorted(observed) == [(1, 2), (2, 4), (3, 6)]


def test_empty_input_is_handled_in_both_modes():
    assert run_parallel_ordered([], lambda i: i, jobs=1) == []
    assert run_parallel_ordered([], lambda i: i, jobs=4) == []


def test_worker_exception_propagates_to_the_caller():
    def worker(item: int) -> int:
        if item == 2:
            raise RuntimeError("probe blew up")
        return item

    with pytest.raises(RuntimeError, match="probe blew up"):
        run_parallel_ordered([1, 2, 3], worker, jobs=2)


# ------------------------------- CLI Helpers -------------------------------- #


def test_write_json_report_creates_missing_parent_directories(tmp_path: Path):
    target = tmp_path / "deep" / "nested" / "report.json"

    write_json_report(target, {"ok": True})

    assert json.loads(target.read_text(encoding="utf-8")) == {"ok": True}
    assert target.read_text(encoding="utf-8").endswith("\n")


def test_resolve_dir_argument_prefers_the_explicit_value(tmp_path: Path):
    explicit = tmp_path / "explicit"
    fallback = tmp_path / "fallback"

    assert resolve_dir_argument(explicit, fallback) == explicit.resolve()
    assert resolve_dir_argument(None, fallback) == fallback.resolve()


def test_resolve_dir_argument_expands_a_user_relative_path():
    resolved = resolve_dir_argument(Path("~/somewhere"), Path("/unused"))

    assert "~" not in str(resolved)
    assert resolved.is_absolute()


# --------------------------- Toolchain selection ---------------------------- #


def test_build_compiler_flags_injects_the_probe_defaults():
    flags = build_compiler_flags(ToolingConfig(compiler=None, compiler_flags=()))

    assert flags[0] == "-std=c++23"
    assert "-fsyntax-only" in flags


def test_build_compiler_flags_respects_a_configured_standard():
    config = ToolingConfig(compiler=None, compiler_flags=("-std=c++20", "-Wall"))

    flags = build_compiler_flags(config)

    assert "-std=c++23" not in flags
    assert flags[0] == "-std=c++20"
    assert flags.count("-fsyntax-only") == 1


def test_build_compiler_flags_does_not_duplicate_syntax_only():
    config = ToolingConfig(compiler=None, compiler_flags=("-std=c++23", "-fsyntax-only"))

    assert build_compiler_flags(config).count("-fsyntax-only") == 1


def test_load_tooling_config_degrades_instead_of_aborting_a_run(tmp_path: Path):
    """An unreadable registry must not abort verification — auto-detect instead."""

    config = load_tooling_config(tmp_path / "does-not-exist")

    assert config.compiler is None
    assert config.compiler_flags == ()


def test_load_tooling_config_degrades_on_a_malformed_registry(tmp_path: Path):
    (tmp_path / "profiles.toml").write_text("not = valid = toml", encoding="utf-8")

    config = load_tooling_config(tmp_path)

    assert config.compiler is None


def test_select_compiler_prefers_the_configured_override():
    config = ToolingConfig(compiler="g++-42", compiler_flags=())

    with (
        mock.patch("module_runtime.shutil.which", return_value="/opt/bin/g++-42"),
        mock.patch.object(module_runtime, "compiler_supports_std_headers", return_value=True),
    ):
        assert select_compiler(config) == "/opt/bin/g++-42"


def test_select_compiler_skips_candidates_without_bits_stdcxx():
    """A compiler on PATH that cannot include <bits/stdc++.h> is not usable here."""

    config = ToolingConfig(compiler=None, compiler_flags=())
    usable = "/usr/bin/g++-14"

    with (
        mock.patch("module_runtime.shutil.which", side_effect=lambda c: f"/usr/bin/{c}"),
        mock.patch.object(
            module_runtime,
            "compiler_supports_std_headers",
            side_effect=lambda resolved: resolved == usable,
        ),
    ):
        assert select_compiler(config) == usable


def test_select_compiler_falls_back_when_nothing_probes_clean():
    """With no usable candidate the configured name is returned unresolved."""

    config = ToolingConfig(compiler="g++-42", compiler_flags=())

    with mock.patch("module_runtime.shutil.which", return_value=None):
        assert select_compiler(config) == "g++-42"


def test_select_compiler_falls_back_to_plain_gpp_without_configuration():
    config = ToolingConfig(compiler=None, compiler_flags=())

    with mock.patch("module_runtime.shutil.which", return_value=None):
        assert select_compiler(config) == "g++"
