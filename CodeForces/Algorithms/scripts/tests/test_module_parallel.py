"""Parallel-execution contract for ``module_tester`` and ``module_verify``.

Covers three invariants the threaded probe runner must satisfy:

- ``parse_jobs_argument`` rejects non-positive and non-numeric ``--jobs`` input;
- ``ModuleVerifier.run_all`` and ``ModuleTester._run_case_group`` actually
  fan out across multiple worker threads when ``jobs > 1``, and accumulate
  results in submission order;
- benchmark cases in ``ModuleTester`` stay sequential regardless of
  ``jobs`` so the per-case ``elapsed_seconds`` measurement is not
  corrupted by sibling contention.

The compile/run step is stubbed everywhere — no ``g++`` is spawned.
"""

from __future__ import annotations

import argparse
import threading
import time
import unittest
from pathlib import Path
from unittest import mock

SCRIPTS_DIR = Path(__file__).resolve().parents[1]

import module_runtime  # noqa: E402
import module_tester  # noqa: E402
import module_verify  # noqa: E402


class ParseJobsArgumentTests(unittest.TestCase):
    """Validator behavior for the shared ``--jobs`` CLI flag."""

    def test_positive_integer_passes(self) -> None:
        self.assertEqual(module_runtime.parse_jobs_argument("4"), 4)

    def test_zero_is_rejected(self) -> None:
        with self.assertRaises(argparse.ArgumentTypeError):
            module_runtime.parse_jobs_argument("0")

    def test_negative_is_rejected(self) -> None:
        with self.assertRaises(argparse.ArgumentTypeError):
            module_runtime.parse_jobs_argument("-2")

    def test_non_integer_is_rejected(self) -> None:
        with self.assertRaises(argparse.ArgumentTypeError):
            module_runtime.parse_jobs_argument("auto")


def _verification_record(case: module_verify.VerificationCase) -> module_verify.VerificationRecord:
    """Build a minimal successful ``VerificationRecord`` for ``case``."""

    return module_verify.VerificationRecord(
        module_name=case.module_name,
        header=case.header,
        category=case.category,
        tags=case.tags,
        case_name=case.name,
        kind=case.kind,
        source=str(case.source),
        success=True,
        elapsed_seconds=0.001,
    )


def _make_verification_cases(count: int) -> list[module_verify.VerificationCase]:
    """Create ``count`` lightweight in-memory verification cases."""

    return [
        module_verify.VerificationCase(
            module_name=f"M{i:02d}",
            header=f"modules/demo/M{i:02d}.hpp",
            category="demo",
            tags=(),
            name=f"case_{i:02d}",
            kind="compile",
            source=Path(f"/tmp/M{i:02d}.cpp"),
        )
        for i in range(count)
    ]


class ParallelVerifyExecutionTests(unittest.TestCase):
    """Cover ``ModuleVerifier.run_all`` thread-pool fan-out and ordering."""

    def _run_verifier_with_stub(
        self, *, jobs: int, sleep_seconds: float = 0.04, count: int = 8
    ) -> tuple[set[int], list[module_verify.VerificationRecord]]:
        """Stub ``discover_cases`` + ``run_case`` and return observed threads + results."""

        verifier = module_verify.ModuleVerifier(SCRIPTS_DIR, jobs=jobs)
        fake_cases = _make_verification_cases(count)
        threads_seen: set[int] = set()
        lock = threading.Lock()

        def fake_run_case(case: module_verify.VerificationCase) -> module_verify.VerificationRecord:
            with lock:
                threads_seen.add(threading.get_ident())
            time.sleep(sleep_seconds)
            return _verification_record(case)

        with (
            mock.patch.object(verifier, "discover_cases", return_value=fake_cases),
            mock.patch.object(verifier, "run_case", side_effect=fake_run_case),
            mock.patch("builtins.print"),
        ):
            verifier.run_all()
        return threads_seen, verifier.results

    def test_jobs_one_stays_on_caller_thread(self) -> None:
        threads, _ = self._run_verifier_with_stub(jobs=1, count=4)
        self.assertEqual(len(threads), 1)

    def test_jobs_many_spreads_across_threads(self) -> None:
        threads, _ = self._run_verifier_with_stub(jobs=4, count=8)
        self.assertGreater(len(threads), 1, "ThreadPoolExecutor should fan out probes")

    def test_results_preserve_submission_order(self) -> None:
        _, results = self._run_verifier_with_stub(jobs=4, count=8)
        self.assertEqual(
            [r.module_name for r in results],
            [f"M{i:02d}" for i in range(8)],
            "self.results must be ordered by enumeration index, not completion order",
        )


class ParallelTesterExecutionTests(unittest.TestCase):
    """Cover ``ModuleTester._run_case_group`` fan-out and benchmark sequentiality."""

    def _build_tester(self, *, jobs: int) -> module_tester.ModuleTester:
        """Construct a tester without touching the compiler/template discovery."""

        tester = module_tester.ModuleTester.__new__(module_tester.ModuleTester)
        tester.templates_dir = SCRIPTS_DIR
        tester.test_results = []
        tester.config = module_runtime.ToolingConfig(compiler=None, compiler_flags=())
        tester.compiler = "g++"
        tester.compiler_flags = ()
        tester.need_mapping = {"NEED_CORE": []}
        tester.need_macros = ["NEED_CORE"]
        tester._compiler_invocation = None  # type: ignore[assignment]
        tester.jobs = max(1, jobs)
        tester._print_lock = threading.Lock()
        return tester

    def _stub_compile(self, threads_seen: set[int], lock: threading.Lock):
        """Return a fake ``_compile_case`` that records its calling thread."""

        def fake(case: module_tester.ModuleTestCase) -> module_runtime.CompilationOutcome:
            with lock:
                threads_seen.add(threading.get_ident())
            time.sleep(0.04)
            return module_runtime.CompilationOutcome(success=True, elapsed_seconds=0.01)

        return fake

    def test_parallel_group_uses_thread_pool(self) -> None:
        tester = self._build_tester(jobs=4)
        cases = [
            module_tester.ModuleTestCase(
                kind="combination", description=f"combo-{i}", macros=("NEED_CORE",)
            )
            for i in range(6)
        ]
        threads_seen: set[int] = set()
        lock = threading.Lock()

        with (
            mock.patch.object(
                tester, "_compile_case", side_effect=self._stub_compile(threads_seen, lock)
            ),
            mock.patch("builtins.print"),
        ):
            tester._run_case_group(title="t", cases=cases, parallel=True)

        self.assertGreater(len(threads_seen), 1)
        self.assertEqual(
            [r.description for r in tester.test_results], [c.description for c in cases]
        )

    def test_benchmark_group_stays_sequential(self) -> None:
        """``parallel=False`` must keep timing measurements uncontended."""

        tester = self._build_tester(jobs=8)
        cases = [
            module_tester.ModuleTestCase(
                kind="benchmark", description=f"bench-{i}", macros=("NEED_CORE",)
            )
            for i in range(4)
        ]
        threads_seen: set[int] = set()
        lock = threading.Lock()

        with (
            mock.patch.object(
                tester, "_compile_case", side_effect=self._stub_compile(threads_seen, lock)
            ),
            mock.patch("builtins.print"),
        ):
            tester._run_case_group(title="t", cases=cases, parallel=False)

        self.assertEqual(
            len(threads_seen),
            1,
            "benchmarks must run on a single thread to keep elapsed_seconds clean",
        )

    def test_test_performance_benchmarks_overrides_jobs(self) -> None:
        """The benchmark entry point must pass ``parallel=False`` even with ``jobs > 1``."""

        tester = self._build_tester(jobs=4)
        captured_kwargs: dict[str, object] = {}

        def fake_group(**kwargs: object) -> None:
            captured_kwargs.update(kwargs)

        with mock.patch.object(tester, "_run_case_group", side_effect=fake_group):
            tester.test_performance_benchmarks()

        self.assertIs(captured_kwargs.get("parallel"), False)


if __name__ == "__main__":
    unittest.main()
