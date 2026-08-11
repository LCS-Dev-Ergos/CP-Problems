"""Regression tests that pin scripts-tree contracts the team relies on.

Each class encodes a non-obvious invariant whose violation has historically
broken either the CLI surface or downstream tooling: importability of every
top-level script, the subprocess-routing rule that funnels all process
spawning through ``_lib/process.py``, the StrEnum-to-argparse mirroring
in ``constants``, the deprecation shim that keeps the old
``module_tester`` private names resolvable, and the cached-property
laziness used to avoid expensive cold imports.
"""

from __future__ import annotations

import argparse
import contextlib
import importlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path
from typing import ClassVar
from unittest import mock

SCRIPTS_DIR = Path(__file__).resolve().parents[1]
WORKFLOW_SCRIPT = SCRIPTS_DIR / "workflow_manager.py"
_MIGRATIONS_DIR = SCRIPTS_DIR / "migrations" / "archive"
MIGRATE_CMAKE_SCRIPT = _MIGRATIONS_DIR / "migrate_round_cmake.py"
MIGRATE_PRESETS_SCRIPT = _MIGRATIONS_DIR / "migrate_round_presets.py"
MIGRATE_TOOLCHAINS_SCRIPT = _MIGRATIONS_DIR / "migrate_round_toolchains.py"
MIGRATE_VSCODE_SCRIPT = _MIGRATIONS_DIR / "migrate_round_vscode.py"
ALGORITHMS_DIR = SCRIPTS_DIR.parent

import benchmark_pch  # noqa: E402
import module_tester  # noqa: E402
import module_verify  # noqa: E402
from workflow_manager_core.constants import (  # noqa: E402
    BUILD_TYPE_CHOICES,
    CMAKE_BUILD_PRESET_CHOICES,
    CMAKE_CONFIG_PRESET_CHOICES,
    COMPILER_CHOICES,
    BuildPreset,
    BuildType,
    CompilerChoice,
    ConfigPreset,
)
from workflow_manager_core.types import CommandResult  # noqa: E402


class ScriptRegressionTests(unittest.TestCase):
    def _run_workflow(
        self, *args: str, env: dict[str, str] | None = None
    ) -> subprocess.CompletedProcess[str]:
        """Run workflow_manager.py with optional environment overrides."""

        return subprocess.run(
            [sys.executable, str(WORKFLOW_SCRIPT), *args],
            capture_output=True,
            text=True,
            check=False,
            env=env,
        )

    @unittest.skipIf(shutil.which("zsh") is None, "zsh is required for runtime smoke")
    def test_cpp_timeout_with_stderr_does_not_traceback(self) -> None:
        """Timeout on cpp-tools command should return clean error output."""

        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            cp_tools = tmp / "competitive_stub.sh"
            cp_tools.write_text(
                textwrap.dedent(
                    """\
                    cpphelp() {
                      print -- "stub-output-before-timeout"
                      print -u2 -- "stub-stderr-before-timeout"
                      sleep 2
                    }
                    """
                ),
                encoding="utf-8",
            )

            result = self._run_workflow(
                "--cp-tools-script",
                str(cp_tools),
                "--cwd",
                str(tmp),
                "--timeout",
                "1",
                "help",
            )

            self.assertEqual(result.returncode, 124)
            self.assertIn("Command timed out.", result.stderr)
            self.assertNotIn("Traceback", result.stderr + result.stdout)

    @unittest.skipIf(shutil.which("zsh") is None, "zsh is required for runtime smoke")
    def test_external_timeout_with_stderr_does_not_traceback(self) -> None:
        """Timeout on external command path should avoid traceback crashes."""

        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            cp_tools = tmp / "competitive_stub.sh"
            cp_tools.write_text("cpphelp() { :; }\n", encoding="utf-8")

            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            fake_cmake = fake_bin / "cmake"
            fake_cmake.write_text(
                textwrap.dedent(
                    """\
                    #!/usr/bin/env bash
                    echo "cmake-stdout-before-timeout"
                    echo "cmake-stderr-before-timeout" >&2
                    sleep 2
                    """
                ),
                encoding="utf-8",
            )
            fake_cmake.chmod(0o755)

            env = dict(os.environ)
            env["PATH"] = f"{fake_bin}:{env['PATH']}"

            result = self._run_workflow(
                "--cp-tools-script",
                str(cp_tools),
                "--cwd",
                str(tmp),
                "--timeout",
                "1",
                "preset-conf",
                "--preset",
                "cp-debug-gcc",
                "--algorithms-dir",
                str(ALGORITHMS_DIR),
                env=env,
            )

            self.assertEqual(result.returncode, 124)
            self.assertIn("Command timed out.", result.stderr)
            self.assertNotIn("Traceback", result.stderr + result.stdout)

    # Removed: test_build_template_without_argument_uses_default_output.
    # ``build_template.sh`` was retired from the scripts tree; the test was
    # permanently skipped and added no coverage.

    def test_benchmark_json_out_creates_parent_directories(self) -> None:
        """Benchmark JSON output should create missing parent directories."""

        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            round_dir = tmp / "Round_Test"
            round_dir.mkdir()
            (round_dir / "CMakeLists.txt").write_text(
                "cmake_minimum_required(VERSION 3.20)\nproject(test)\n",
                encoding="utf-8",
            )
            (round_dir / "problem_A.cpp").write_text(
                "int main() { return 0; }\n",
                encoding="utf-8",
            )

            json_out = tmp / "reports" / "pch" / "result.json"
            ns = argparse.Namespace(
                round_dir=round_dir,
                target="problem_A",
                compiler="gcc",
                algorithms_dir=ALGORITHMS_DIR,
                threshold_sec=1.0,
                json_out=json_out,
                verbose=False,
            )

            fake_on = benchmark_pch.BenchmarkMetrics(
                configure_sec=1.0,
                cold_build_sec=2.0,
                incremental_build_sec=1.0,
                reconfigure_sec=1.0,
                post_reconfigure_build_sec=2.0,
            )
            fake_off = benchmark_pch.BenchmarkMetrics(
                configure_sec=1.0,
                cold_build_sec=1.5,
                incremental_build_sec=1.2,
                reconfigure_sec=1.0,
                post_reconfigure_build_sec=1.6,
            )

            with (
                mock.patch.object(benchmark_pch, "parse_args", return_value=ns),
                mock.patch.object(
                    benchmark_pch,
                    "_benchmark_mode",
                    side_effect=[fake_on, fake_off],
                ),
                contextlib.redirect_stdout(io.StringIO()),
                contextlib.redirect_stderr(io.StringIO()),
            ):
                rc = benchmark_pch.main()

            self.assertEqual(rc, 0)
            self.assertTrue(json_out.is_file())
            payload = json.loads(json_out.read_text(encoding="utf-8"))
            self.assertEqual(payload["round_dir"], str(round_dir.resolve()))
            self.assertEqual(payload["pch_on"]["incremental_build_sec"], 1.0)
            self.assertEqual(payload["delta"]["incremental_gain_sec"], 0.2)
            self.assertEqual(payload["recommendation"], "disable_default_pch")

    @unittest.skipIf(shutil.which("zsh") is None, "zsh is required for runtime smoke")
    def test_workflow_doctor_marks_cppdiag_failure_as_non_fatal(self) -> None:
        """Non-strict doctor should preserve cppdiag failure details without failing the workflow."""

        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            cp_tools = tmp / "competitive_stub.sh"
            cp_tools.write_text(
                textwrap.dedent(
                    """\
                    cpphelp() {
                      print -- "help-valid"
                    }
                    cppinfo() {
                      print -- "info-valid"
                    }
                    cppdiag() {
                      print -u2 -- "diag-failed"
                      return 7
                    }
                    cppcheck() {
                      print -- "check-valid"
                    }
                    """
                ),
                encoding="utf-8",
            )

            result = self._run_workflow(
                "--json",
                "--cp-tools-script",
                str(cp_tools),
                "--cwd",
                str(tmp),
                "doctor",
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            payload = json.loads(result.stdout)
            self.assertEqual(payload["status"], "valid")
            diag_step = next(step for step in payload["steps"] if step["function"] == "cppdiag")
            self.assertEqual(diag_step["returncode"], 7)
            self.assertTrue(diag_step["non_fatal"])
            self.assertIn("diag-failed", diag_step["stderr"])

    def test_module_tester_fails_when_zero_tests_execute(self) -> None:
        """Zero executed tests should be treated as failure."""

        tester = module_tester.ModuleTester(ALGORITHMS_DIR / "templates")
        tester.need_macros = []
        tester.need_mapping = {}

        with contextlib.redirect_stdout(io.StringIO()):
            success = tester.run_all_tests()

        self.assertFalse(success)
        self.assertEqual(tester.test_results, [])

    @unittest.skipIf(shutil.which("zsh") is None, "zsh is required for runtime smoke")
    def test_workflow_new_if_missing_skips_existing_cc_source(self) -> None:
        """`new --if-missing` should honor existing .cc sources, not only .cpp."""

        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            (tmp / "problem_A.cc").write_text("int main() { return 0; }\n", encoding="utf-8")
            cp_tools = tmp / "competitive_stub.sh"
            cp_tools.write_text(
                textwrap.dedent(
                    """\
                    cppnew() {
                      print -- "cppnew-called"
                    }
                    """
                ),
                encoding="utf-8",
            )

            result = self._run_workflow(
                "--json",
                "--cp-tools-script",
                str(cp_tools),
                "--cwd",
                str(tmp),
                "new",
                "problem_A",
                "--if-missing",
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            payload = json.loads(result.stdout)
            self.assertEqual(payload["steps"], [])
            self.assertTrue(
                any(
                    "problem_A.cc already exists, skipping cppnew" in note
                    for note in payload["notes"]
                )
            )

    @unittest.skipIf(shutil.which("zsh") is None, "zsh is required for runtime smoke")
    def test_workflow_cycle_new_if_missing_skips_existing_cxx_source(self) -> None:
        """`cycle --new-if-missing` should honor existing .cxx sources."""

        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            (tmp / "problem_A.cxx").write_text("int main() { return 0; }\n", encoding="utf-8")
            cp_tools = tmp / "competitive_stub.sh"
            cp_tools.write_text(
                textwrap.dedent(
                    """\
                    cppnew() {
                      print -- "cppnew-called"
                    }
                    """
                ),
                encoding="utf-8",
            )

            result = self._run_workflow(
                "--json",
                "--cp-tools-script",
                str(cp_tools),
                "--cwd",
                str(tmp),
                "cycle",
                "problem_A",
                "--new-if-missing",
                "--skip-go",
                "--skip-judge",
                "--skip-submit",
                "--skip-submit-test",
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            payload = json.loads(result.stdout)
            self.assertEqual(payload["steps"], [])
            self.assertTrue(
                any(
                    "problem_A.cxx already exists, skipping cppnew" in note
                    for note in payload["notes"]
                )
            )

    def test_migrate_round_vscode_links_scripts_directory(self) -> None:
        """VS Code migration should also centralize the shared scripts directory."""

        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            rounds_root = tmp / "Rounds"
            round_dir = rounds_root / "Round_Test"
            algorithms_root = tmp / "Algorithms"
            central_vscode = algorithms_root / ".vscode"

            round_dir.mkdir(parents=True)
            (round_dir / "CMakeLists.txt").write_text(
                "cmake_minimum_required(VERSION 3.20)\n", encoding="utf-8"
            )
            central_vscode.mkdir(parents=True)
            (central_vscode / "scripts").mkdir()
            (central_vscode / "scripts" / "cp_sanitize_task.sh").write_text(
                "#!/bin/sh\n", encoding="utf-8"
            )
            for name in ("settings.json", "tasks.json", "launch.json", "c_cpp_properties.json"):
                (central_vscode / name).write_text("{}\n", encoding="utf-8")

            result = subprocess.run(
                [
                    sys.executable,
                    str(MIGRATE_VSCODE_SCRIPT),
                    "--rounds-root",
                    str(rounds_root),
                    "--algorithms-root",
                    str(algorithms_root),
                ],
                capture_output=True,
                text=True,
                check=False,
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            scripts_link = round_dir / ".vscode" / "scripts"
            self.assertTrue(scripts_link.is_symlink())
            self.assertEqual(
                scripts_link.resolve(),
                (central_vscode / "scripts").resolve(),
            )

    def test_migrate_round_vscode_skips_directory_symlink_rounds(self) -> None:
        """Migration should skip rounds already using a directory symlink to central .vscode."""

        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            rounds_root = tmp / "Rounds"
            round_dir = rounds_root / "Round_Test"
            algorithms_root = tmp / "Algorithms"
            central_vscode = algorithms_root / ".vscode"

            round_dir.mkdir(parents=True)
            (round_dir / "CMakeLists.txt").write_text(
                "cmake_minimum_required(VERSION 3.20)\n", encoding="utf-8"
            )
            central_vscode.mkdir(parents=True)
            (central_vscode / "scripts").mkdir()
            (central_vscode / "scripts" / "cp_sanitize_task.sh").write_text(
                "#!/bin/sh\n", encoding="utf-8"
            )
            original_settings = '{\n  "marker": true\n}\n'
            (central_vscode / "settings.json").write_text(original_settings, encoding="utf-8")
            for name in ("tasks.json", "launch.json", "c_cpp_properties.json"):
                (central_vscode / name).write_text("{}\n", encoding="utf-8")

            (round_dir / ".vscode").symlink_to(
                os.path.relpath(central_vscode, start=round_dir),
                target_is_directory=True,
            )

            result = subprocess.run(
                [
                    sys.executable,
                    str(MIGRATE_VSCODE_SCRIPT),
                    "--rounds-root",
                    str(rounds_root),
                    "--algorithms-root",
                    str(algorithms_root),
                ],
                capture_output=True,
                text=True,
                check=False,
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            self.assertIn("already-linked-dir", result.stdout)
            self.assertEqual(
                (central_vscode / "settings.json").read_text(encoding="utf-8"),
                original_settings,
            )

    def test_migrate_round_cmake_creates_backup_before_rewrite(self) -> None:
        """CMake migration should preserve the previous file when backups are enabled."""

        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            rounds_root = tmp / "Rounds"
            round_dir = rounds_root / "Round_Test"
            round_dir.mkdir(parents=True)
            cmake_path = round_dir / "CMakeLists.txt"
            original = "cmake_minimum_required(VERSION 3.20)\nproject(old_round)\n"
            cmake_path.write_text(original, encoding="utf-8")

            result = subprocess.run(
                [
                    sys.executable,
                    str(MIGRATE_CMAKE_SCRIPT),
                    "--rounds-root",
                    str(rounds_root),
                ],
                capture_output=True,
                text=True,
                check=False,
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            backups = list(round_dir.glob("CMakeLists.txt.bak-centralized-*"))
            self.assertEqual(len(backups), 1)
            self.assertEqual(backups[0].read_text(encoding="utf-8"), original)
            self.assertEqual(
                cmake_path.read_text(encoding="utf-8"),
                (
                    "# Thin round-level CMakeLists: delegates all build logic to centralized modules.\n"
                    "cmake_minimum_required(VERSION 3.20)\n"
                    "project(competitive_programming LANGUAGES CXX)\n\n"
                    'set(CP_ROUND_BOOTSTRAP "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/cmake/CPRoundBootstrap.cmake")\n'
                    'if(NOT EXISTS "${CP_ROUND_BOOTSTRAP}")\n'
                    "  message(FATAL_ERROR\n"
                    '    "Central build bootstrap not found: ${CP_ROUND_BOOTSTRAP}\\n"\n'
                    '    "Run cppinit (or recreate algorithms/cmake symlink) to restore shared build modules.")\n'
                    "endif()\n\n"
                    'include("${CP_ROUND_BOOTSTRAP}")\n'
                ),
            )

    def test_migrate_round_presets_creates_backup_before_rewrite(self) -> None:
        """Preset migration should preserve the previous file when backups are enabled."""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            rounds_root = tmp / "Rounds"
            round_dir = rounds_root / "Round_Test"
            round_dir.mkdir(parents=True)
            (round_dir / "CMakeLists.txt").write_text(
                "cmake_minimum_required(VERSION 3.20)\n", encoding="utf-8"
            )
            presets_path = round_dir / "CMakePresets.json"
            original = '{\n  "version": 3\n}\n'
            presets_path.write_text(original, encoding="utf-8")

            result = subprocess.run(
                [
                    sys.executable,
                    str(MIGRATE_PRESETS_SCRIPT),
                    "--rounds-root",
                    str(rounds_root),
                ],
                capture_output=True,
                text=True,
                check=False,
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            backups = list(round_dir.glob("CMakePresets.json.bak-centralized-*"))
            self.assertEqual(len(backups), 1)
            self.assertEqual(backups[0].read_text(encoding="utf-8"), original)
            self.assertEqual(
                presets_path.read_text(encoding="utf-8"),
                (
                    "{\n"
                    '  "version": 6,\n'
                    '  "include": [\n'
                    '    "algorithms/cmake/CPRoundPresets.json"\n'
                    "  ]\n"
                    "}\n"
                ),
            )

    def test_migrate_round_toolchains_respects_explicit_round_selection(self) -> None:
        """Toolchain migration should only affect explicitly selected rounds."""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            rounds_root = tmp / "Rounds"
            central_root = tmp / "Algorithms"
            central_root.mkdir(parents=True)
            for name in ("gcc-toolchain.cmake", "clang-toolchain.cmake"):
                (central_root / name).write_text("# central\n", encoding="utf-8")

            selected_round = rounds_root / "Round_Selected"
            untouched_round = rounds_root / "Round_Untouched"
            for round_dir in (selected_round, untouched_round):
                round_dir.mkdir(parents=True)
                (round_dir / "CMakeLists.txt").write_text(
                    "cmake_minimum_required(VERSION 3.20)\n", encoding="utf-8"
                )
                for name in ("gcc-toolchain.cmake", "clang-toolchain.cmake"):
                    (round_dir / name).write_text(f"# local {round_dir.name}\n", encoding="utf-8")

            result = subprocess.run(
                [
                    sys.executable,
                    str(MIGRATE_TOOLCHAINS_SCRIPT),
                    "--rounds-root",
                    str(rounds_root),
                    "--central-root",
                    str(central_root),
                    "--round",
                    "Round_Selected",
                ],
                capture_output=True,
                text=True,
                check=False,
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            for name in ("gcc-toolchain.cmake", "clang-toolchain.cmake"):
                self.assertFalse((selected_round / name).exists())
                self.assertEqual(
                    len(list(selected_round.glob(f"{name}.bak-centralized-*"))),
                    1,
                )
                self.assertTrue((untouched_round / name).is_file())


class ModuleRuntimeOwnershipTests(unittest.TestCase):
    """``module_runtime`` owns the shared toolchain helpers outright.

    The ``module_tester.__getattr__`` compatibility shim that used to
    re-export them under their old underscore names has been removed; this
    guard keeps callers from drifting back to the private spelling.
    """

    def test_no_internal_caller_imports_private_module_tester_names(self) -> None:
        """The codebase must not regress to importing module_tester._* internally."""

        scripts_dir = Path(__file__).resolve().parents[1]
        offending: list[str] = []
        for path in scripts_dir.rglob("*.py"):
            if "__pycache__" in path.parts or "migrations/archive" in path.as_posix():
                continue
            if path.resolve() == Path(__file__).resolve():
                continue
            text = path.read_text(encoding="utf-8")
            if "from module_tester import _" in text:
                offending.append(str(path.relative_to(scripts_dir)))
        self.assertEqual(offending, [], msg="should import from module_runtime instead")

    def test_legacy_underscore_names_are_gone(self) -> None:
        """The retired shim must not resurface: old names raise AttributeError."""

        module_tester = importlib.import_module("module_tester")

        for legacy in (
            "_load_template_config",
            "_select_compiler",
            "_build_compiler_flags",
            "_compiler_supports_std_headers",
        ):
            with self.subTest(name=legacy), self.assertRaises(AttributeError):
                getattr(module_tester, legacy)


class StrEnumChoicesTests(unittest.TestCase):
    """Argparse-facing tuples in ``constants`` must mirror their ``StrEnum`` source.

    Every CLI-choices tuple (``BUILD_TYPE_CHOICES``, ``COMPILER_CHOICES``,
    ``TOGGLE_CHOICES``, ``PCH_CHOICES``, ``CMAKE_*_PRESET_CHOICES``) is
    derived from a ``StrEnum`` so mypy and argparse agree on the legal
    surface. If someone adds a new enum member without rebuilding the tuple
    the contract breaks silently — these tests catch that drift.
    """

    def test_build_type_choices_match_strenum(self) -> None:
        self.assertEqual(BUILD_TYPE_CHOICES, tuple(e.value for e in BuildType))

    def test_compiler_choices_match_strenum(self) -> None:
        self.assertEqual(COMPILER_CHOICES, tuple(e.value for e in CompilerChoice))

    def test_config_preset_choices_match_strenum(self) -> None:
        self.assertEqual(CMAKE_CONFIG_PRESET_CHOICES, tuple(e.value for e in ConfigPreset))

    def test_build_preset_choices_match_strenum(self) -> None:
        self.assertEqual(CMAKE_BUILD_PRESET_CHOICES, tuple(e.value for e in BuildPreset))

    def test_strenum_members_compare_equal_to_their_string_values(self) -> None:
        """``StrEnum`` substitutability is what keeps argparse happy."""

        self.assertEqual(BuildType.DEBUG, "Debug")
        self.assertEqual(str(BuildType.DEBUG), "Debug")


class CachedPropertyLazinessTests(unittest.TestCase):
    """``ModuleVerifier.compiler`` must defer toolchain probing to first access."""

    def test_compiler_attribute_not_computed_until_accessed(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            (root / "templates").mkdir(parents=True)
            verifier = module_verify.ModuleVerifier(root)
            # Pre-access: compiler must not yet live on the instance dict.
            self.assertNotIn("compiler", verifier.__dict__)
            _ = verifier.compiler
            self.assertIn("compiler", verifier.__dict__)


class CommandResultSelfReturnTypeTests(unittest.TestCase):
    """``CommandResult.as_non_fatal`` returns the same subclass it was called on."""

    def test_as_non_fatal_preserves_instance_class(self) -> None:
        original = CommandResult(
            function="cppdiag",
            args=(),
            cwd="/tmp",
            returncode=7,
            duration_ms=100,
            stdout="",
            stderr="diag-failed",
        )
        copied = original.as_non_fatal()
        self.assertIsInstance(copied, CommandResult)
        self.assertTrue(copied.non_fatal)
        self.assertEqual(copied.returncode, 7)


class SubprocessLayerEnforcementTests(unittest.TestCase):
    """Only ``_lib/process.py`` may call ``subprocess.run`` directly.

    Every other script in the tree must route through ``_lib.process.run_capture``
    so timeout handling, text decoding, and elapsed-time accounting stay
    uniform. This test scans the working copy and rejects any new direct
    ``subprocess.run`` call that slips past code review.
    """

    _ALLOWED_DIRECT_SUBPROCESS: ClassVar[set[str]] = {
        "_lib/process.py",
        # syntax_check still calls git rev-parse / git diff directly; both are
        # synchronous one-shots where check=True is the desired behavior and
        # routing them through run_capture would obscure the intent.
        "syntax_check.py",
    }

    def test_only_lib_process_uses_raw_subprocess_run(self) -> None:
        """Every other script must go through ``_lib.process.run_capture``."""

        scripts_dir = Path(__file__).resolve().parents[1]
        offenders: list[str] = []
        for path in scripts_dir.rglob("*.py"):
            posix = path.relative_to(scripts_dir).as_posix()
            if posix.startswith(("tests/", "migrations/archive/", "__pycache__")):
                continue
            if posix in self._ALLOWED_DIRECT_SUBPROCESS:
                continue
            if "subprocess.run" in path.read_text(encoding="utf-8"):
                offenders.append(posix)
        self.assertEqual(
            offenders,
            [],
            msg="route these subprocess.run calls through _lib.process.run_capture",
        )


class ScriptsImportabilityTests(unittest.TestCase):
    """Guard against unparseable top-level scripts shipping unnoticed."""

    EXPECTED_MODULES = (
        "benchmark_pch",
        "flattener",
        "gen_config",
        "gen_scaffold",
        "module_runtime",
        "module_tester",
        "module_verify",
        "need_resolver",
        "profile_registry",
        "strip_index_casts",
        "syntax_check",
        "workflow_manager",
    )

    def test_every_top_level_script_imports_cleanly(self) -> None:
        """All shipped scripts must be importable without side effects failing."""

        for name in self.EXPECTED_MODULES:
            with self.subTest(module=name):
                module = importlib.import_module(name)
                self.assertTrue(hasattr(module, "__file__"))


class FlattenerLazyAugmentationTests(unittest.TestCase):
    """Verify the templates/ rglob no longer fires on plain import."""

    def test_headers_import_does_not_eagerly_walk_templates(self) -> None:
        """Importing flattener_core.headers must not augment tables."""

        headers = importlib.import_module("flattener_core.headers")
        importlib.reload(headers)
        # ensure_augmented_tables is wrapped in @functools.cache; cache_info()
        # reports 0 hits / 0 misses immediately after import.
        self.assertEqual(
            headers.ensure_augmented_tables.cache_info().misses,
            0,
            "Importing flattener_core.headers should not trigger augmentation",
        )

    def test_augmentation_runs_on_first_pruning_call(self) -> None:
        """Calling ``prune_template_headers_with_policy`` should fire augmentation."""

        headers = importlib.import_module("flattener_core.headers")
        importlib.reload(headers)
        self.assertEqual(headers.ensure_augmented_tables.cache_info().misses, 0)
        headers.prune_template_headers_with_policy([], "", enable_pruning=True)
        self.assertEqual(
            headers.ensure_augmented_tables.cache_info().misses,
            1,
            "First pruning call must execute the augmentation body exactly once",
        )


class SyntaxCheckGitToplevelTests(unittest.TestCase):
    """Ensure syntax_check uses git rev-parse, with safe fall-back."""

    def test_git_toplevel_resolves_to_a_directory(self) -> None:
        """In the current repo the lookup must succeed and point to a real dir."""

        syntax_check = importlib.import_module("syntax_check")
        syntax_check._git_toplevel.cache_clear()
        top = syntax_check._git_toplevel()
        self.assertTrue(top.is_dir(), msg=f"toplevel {top} is not a directory")

    def test_git_toplevel_falls_back_to_root_when_git_missing(self) -> None:
        """When ``git`` raises OSError the fallback should be ``ROOT``."""

        syntax_check = importlib.import_module("syntax_check")
        syntax_check._git_toplevel.cache_clear()
        with mock.patch.object(
            syntax_check.subprocess,
            "run",
            side_effect=OSError("git missing"),
        ):
            self.assertEqual(syntax_check._git_toplevel(), syntax_check.ROOT)
        syntax_check._git_toplevel.cache_clear()


if __name__ == "__main__":
    unittest.main()
