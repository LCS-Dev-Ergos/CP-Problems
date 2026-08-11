#!/usr/bin/env python3
"""Benchmark precompiled-header impact on CP round build times.

Runs three repeatable scenarios per configuration (PCH enabled vs disabled):
cold full build, single-file incremental, and post-reconfigure rebuild. The
script invokes the project's CMake configure + build through the shared
``_lib.process.run_capture`` layer and reports wall-time deltas, so the
team can decide whether the PCH cache is paying for itself on a given
toolchain.
"""

from __future__ import annotations

import argparse
import json
from collections.abc import Mapping, Sequence
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Literal

from _lib.cli import run_cli
from _lib.process import ProcessRequest, run_capture

BenchmarkRecommendation = Literal["disable_default_pch", "keep_default_pch"]

DEFAULT_ALGORITHMS_DIR = Path(__file__).resolve().parents[1]


class BenchmarkError(RuntimeError):
    """Raised when a benchmark command fails."""


@dataclass(frozen=True, slots=True)
class BenchmarkMetrics:
    """Normalized timing metrics for one PCH mode."""

    configure_sec: float
    cold_build_sec: float
    incremental_build_sec: float
    reconfigure_sec: float
    post_reconfigure_build_sec: float


@dataclass(frozen=True, slots=True)
class BenchmarkDelta:
    """Derived timing comparisons between PCH-on and PCH-off runs."""

    incremental_gain_sec: float
    cold_penalty_sec: float
    post_reconfigure_penalty_sec: float


@dataclass(frozen=True, slots=True)
class BenchmarkResult:
    """Full benchmark payload emitted to stdout/JSON output."""

    round_dir: Path
    target: str
    compiler: str
    threshold_sec: float
    pch_on: BenchmarkMetrics
    pch_off: BenchmarkMetrics
    delta: BenchmarkDelta
    recommendation: BenchmarkRecommendation

    def to_dict(self) -> dict[str, object]:
        """Serialize nested benchmark data into JSON-friendly primitives."""
        return {
            "round_dir": str(self.round_dir),
            "target": self.target,
            "compiler": self.compiler,
            "threshold_sec": self.threshold_sec,
            "pch_on": asdict(self.pch_on),
            "pch_off": asdict(self.pch_off),
            "delta": asdict(self.delta),
            "recommendation": self.recommendation,
        }


def _round_seconds(value: float) -> float:
    """Round benchmark timings to stable millisecond precision."""

    return round(value, 3)


def _run_command(
    argv: Sequence[str],
    cwd: Path,
    env_overrides: Mapping[str, str],
    verbose: bool,
) -> float:
    """Run a command and return wall-clock duration in seconds."""

    if verbose:
        print(f"[bench] {' '.join(argv)}")

    result = run_capture(
        ProcessRequest(
            argv=argv,
            cwd=cwd,
            env_overrides=env_overrides,
        )
    )
    if result.failed:
        raise BenchmarkError(
            "Command failed\n"
            f"cmd: {' '.join(argv)}\n"
            f"cwd: {cwd}\n"
            f"exit: {result.returncode}\n"
            f"stdout:\n{result.stdout}\n"
            f"stderr:\n{result.stderr}"
        )
    return result.duration_ms / 1000.0


def _discover_target(round_dir: Path) -> str:
    """Pick a default target from round sources when none is provided."""

    candidates = sorted(p for p in round_dir.glob("*.cpp") if "template" not in p.name.lower())
    if not candidates:
        raise BenchmarkError(f"No candidate .cpp targets found in {round_dir}")
    return candidates[0].stem


def _write_touch_marker(source_file: Path, marker: str) -> None:
    """Append a marker comment to force an incremental rebuild."""

    with source_file.open("a", encoding="utf-8") as fh:
        fh.write(f"\n// {marker}\n")


def _write_new_target(round_dir: Path, target_name: str) -> None:
    """Create a minimal benchmark source file for post-reconfigure timing."""

    source = round_dir / f"{target_name}.cpp"
    source.write_text(
        "#include <bits/stdc++.h>\n"
        "int main() {\n"
        "    std::ios::sync_with_stdio(false);\n"
        "    std::cin.tie(nullptr);\n"
        "    return 0;\n"
        "}\n",
        encoding="utf-8",
    )


def _benchmark_mode(
    round_dir: Path,
    target: str,
    *,
    compiler: str,
    pch_enabled: bool,
    algorithms_dir: Path,
    verbose: bool,
) -> BenchmarkMetrics:
    """Measure configure/build timings for one PCH mode."""

    env_overrides: dict[str, str] = {
        "CP_ALGORITHMS_DIR": str(algorithms_dir),
        "CCACHE_DISABLE": "1",
    }

    configure_preset = f"cp-debug-{compiler}"
    build_preset = f"cp-build-debug-{compiler}"

    mode = "on" if pch_enabled else "off"
    bench_target = f"__bench_new_target_{mode}"
    source_file = round_dir / f"{target}.cpp"
    source_backup = source_file.read_text(encoding="utf-8")
    bench_source = round_dir / f"{bench_target}.cpp"

    configure_argv = [
        "cmake",
        "--preset",
        configure_preset,
        "--fresh",
        f"-DCP_ENABLE_PCH={'ON' if pch_enabled else 'OFF'}",
        "-DCP_FORCE_PCH_REBUILD=OFF",
    ]
    cold_build_argv = [
        "cmake",
        "--build",
        "--preset",
        build_preset,
        "--target",
        target,
    ]

    try:
        configure_sec = _run_command(configure_argv, round_dir, env_overrides, verbose)
        cold_build_sec = _run_command(cold_build_argv, round_dir, env_overrides, verbose)

        _write_touch_marker(source_file, f"bench-incremental-{mode}")
        incremental_build_sec = _run_command(cold_build_argv, round_dir, env_overrides, verbose)

        _write_new_target(round_dir, bench_target)
        reconfigure_sec = _run_command(
            [
                "cmake",
                "--preset",
                configure_preset,
                f"-DCP_ENABLE_PCH={'ON' if pch_enabled else 'OFF'}",
                "-DCP_FORCE_PCH_REBUILD=OFF",
            ],
            round_dir,
            env_overrides,
            verbose,
        )
        post_reconfigure_build_sec = _run_command(
            [
                "cmake",
                "--build",
                "--preset",
                build_preset,
                "--target",
                bench_target,
            ],
            round_dir,
            env_overrides,
            verbose,
        )
    finally:
        source_file.write_text(source_backup, encoding="utf-8")
        if bench_source.exists():
            bench_source.unlink()

    return BenchmarkMetrics(
        configure_sec=_round_seconds(configure_sec),
        cold_build_sec=_round_seconds(cold_build_sec),
        incremental_build_sec=_round_seconds(incremental_build_sec),
        reconfigure_sec=_round_seconds(reconfigure_sec),
        post_reconfigure_build_sec=_round_seconds(post_reconfigure_build_sec),
    )


def _build_result(
    round_dir: Path,
    target: str,
    *,
    compiler: str,
    threshold_sec: float,
    on_metrics: BenchmarkMetrics,
    off_metrics: BenchmarkMetrics,
) -> BenchmarkResult:
    """Assemble benchmark output payload and recommendation."""

    delta = BenchmarkDelta(
        incremental_gain_sec=_round_seconds(
            off_metrics.incremental_build_sec - on_metrics.incremental_build_sec
        ),
        cold_penalty_sec=_round_seconds(on_metrics.cold_build_sec - off_metrics.cold_build_sec),
        post_reconfigure_penalty_sec=_round_seconds(
            on_metrics.post_reconfigure_build_sec - off_metrics.post_reconfigure_build_sec
        ),
    )

    recommendation: BenchmarkRecommendation = (
        "disable_default_pch" if delta.incremental_gain_sec < threshold_sec else "keep_default_pch"
    )

    return BenchmarkResult(
        round_dir=round_dir,
        target=target,
        compiler=compiler,
        threshold_sec=threshold_sec,
        pch_on=on_metrics,
        pch_off=off_metrics,
        delta=delta,
        recommendation=recommendation,
    )


def parse_args() -> argparse.Namespace:
    """Parse benchmark CLI arguments."""

    parser = argparse.ArgumentParser(
        description="Benchmark PCH value for CP round builds (Debug presets)."
    )
    parser.add_argument(
        "--round-dir",
        type=Path,
        required=True,
        help="absolute or relative path to a round directory containing CMakeLists.txt",
    )
    parser.add_argument(
        "--target",
        default=None,
        help="target/problem name to benchmark (defaults to first .cpp file)",
    )
    parser.add_argument(
        "--compiler",
        choices=("gcc", "clang"),
        default="gcc",
        help="debug preset compiler family",
    )
    parser.add_argument(
        "--algorithms-dir",
        type=Path,
        default=DEFAULT_ALGORITHMS_DIR,
        help="path to CodeForces/Algorithms directory",
    )
    parser.add_argument(
        "--threshold-sec",
        type=float,
        default=1.0,
        help="incremental gain threshold used for recommendation",
    )
    parser.add_argument(
        "--json-out",
        type=Path,
        default=None,
        help="optional path where JSON result is saved",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
    )
    return parser.parse_args()


def main() -> int:
    """Execute PCH benchmark workflow and emit JSON output."""

    ns = parse_args()
    round_dir = ns.round_dir.expanduser().resolve()
    algorithms_dir = ns.algorithms_dir.expanduser().resolve()

    if not round_dir.is_dir() or not (round_dir / "CMakeLists.txt").is_file():
        raise BenchmarkError(f"Invalid round dir: {round_dir}")
    if not algorithms_dir.is_dir() or not (algorithms_dir / "cmake").is_dir():
        raise BenchmarkError(f"Invalid algorithms dir: {algorithms_dir}")

    target = ns.target or _discover_target(round_dir)

    on_metrics = _benchmark_mode(
        round_dir,
        target,
        compiler=ns.compiler,
        pch_enabled=True,
        algorithms_dir=algorithms_dir,
        verbose=ns.verbose,
    )
    off_metrics = _benchmark_mode(
        round_dir,
        target,
        compiler=ns.compiler,
        pch_enabled=False,
        algorithms_dir=algorithms_dir,
        verbose=ns.verbose,
    )

    result = _build_result(
        round_dir,
        target,
        compiler=ns.compiler,
        threshold_sec=ns.threshold_sec,
        on_metrics=on_metrics,
        off_metrics=off_metrics,
    )
    payload = result.to_dict()

    print(json.dumps(payload, ensure_ascii=False, indent=2))
    if ns.json_out is not None:
        json_out = ns.json_out.expanduser()
        json_out.parent.mkdir(parents=True, exist_ok=True)
        json_out.write_text(
            json.dumps(payload, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        print(f"[bench] wrote {json_out}")

    return 0


if __name__ == "__main__":
    raise SystemExit(run_cli(main, also_catch=(BenchmarkError,)))
