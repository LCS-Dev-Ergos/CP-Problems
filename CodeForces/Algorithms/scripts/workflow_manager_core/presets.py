"""Centralized CMake preset profiles and the handlers that drive them.

The ``preset-conf`` / ``preset-build`` subcommands are the only part of the
CLI that bypasses cpp-tools and invokes ``cmake`` directly, and the only part
that writes the ``.statistics`` metadata the shell helpers read back. Both
concerns live here so the command registry stays a registry.

``_PRESET_PROFILES_BY_CONFIG`` is keyed by :class:`ConfigPreset` rather than
by raw strings so ``mypy`` catches a preset name that no longer exists, and
``_PRESET_PROFILES_BY_BUILD`` is derived from it — the two views cannot drift.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass

from .constants import BuildPreset, ConfigPreset
from .orchestration import WorkflowManager, run_external_step_with_policy
from .types import WorkflowError
from .utils import discover_algorithms_dir

__all__ = ["PresetProfile", "handle_preset_build", "handle_preset_conf"]


@dataclass(frozen=True, slots=True)
class PresetProfile:
    """Metadata describing one centralized compiler/build profile."""

    build_type: str
    compiler: str
    pch: str
    build_dir: str
    config_preset: str
    build_preset: str


_PRESET_PROFILES_BY_CONFIG: dict[ConfigPreset, PresetProfile] = {
    ConfigPreset.CP_DEBUG_GCC: PresetProfile(
        build_type="Debug",
        compiler="gcc",
        pch="ON",
        build_dir="build/gcc/debug",
        config_preset=ConfigPreset.CP_DEBUG_GCC.value,
        build_preset=BuildPreset.CP_BUILD_DEBUG_GCC.value,
    ),
    ConfigPreset.CP_RELEASE_GCC: PresetProfile(
        build_type="Release",
        compiler="gcc",
        pch="OFF",
        build_dir="build/gcc/release",
        config_preset=ConfigPreset.CP_RELEASE_GCC.value,
        build_preset=BuildPreset.CP_BUILD_RELEASE_GCC.value,
    ),
    ConfigPreset.CP_SANITIZE_GCC: PresetProfile(
        build_type="Sanitize",
        compiler="gcc",
        pch="OFF",
        build_dir="build/gcc/sanitize",
        config_preset=ConfigPreset.CP_SANITIZE_GCC.value,
        build_preset=BuildPreset.CP_BUILD_SANITIZE_GCC.value,
    ),
    ConfigPreset.CP_DEBUG_CLANG: PresetProfile(
        build_type="Debug",
        compiler="clang",
        pch="OFF",
        build_dir="build/clang/debug",
        config_preset=ConfigPreset.CP_DEBUG_CLANG.value,
        build_preset=BuildPreset.CP_BUILD_DEBUG_CLANG.value,
    ),
    ConfigPreset.CP_RELEASE_CLANG: PresetProfile(
        build_type="Release",
        compiler="clang",
        pch="OFF",
        build_dir="build/clang/release",
        config_preset=ConfigPreset.CP_RELEASE_CLANG.value,
        build_preset=BuildPreset.CP_BUILD_RELEASE_CLANG.value,
    ),
    ConfigPreset.CP_SANITIZE_CLANG: PresetProfile(
        build_type="Sanitize",
        compiler="clang",
        pch="OFF",
        build_dir="build/clang/sanitize",
        config_preset=ConfigPreset.CP_SANITIZE_CLANG.value,
        build_preset=BuildPreset.CP_BUILD_SANITIZE_CLANG.value,
    ),
}

_PRESET_PROFILES_BY_BUILD: dict[BuildPreset, PresetProfile] = {
    BuildPreset(profile.build_preset): profile for profile in _PRESET_PROFILES_BY_CONFIG.values()
}


def _preset_profile_from_config(preset: str) -> PresetProfile:
    """Return profile metadata for one configure preset name."""

    try:
        return _PRESET_PROFILES_BY_CONFIG[ConfigPreset(preset)]
    except (KeyError, ValueError) as exc:
        raise WorkflowError(f"unsupported configure preset: {preset}") from exc


def _preset_profile_from_build(preset: str) -> PresetProfile:
    """Return profile metadata for one build preset name."""

    try:
        return _PRESET_PROFILES_BY_BUILD[BuildPreset(preset)]
    except (KeyError, ValueError) as exc:
        raise WorkflowError(f"unsupported build preset: {preset}") from exc


def _write_profile_metadata(manager: WorkflowManager, profile: PresetProfile) -> None:
    """Persist active build profile metadata consumed by cpp-tools shell helpers."""

    stats_dir = manager.runner.cwd / ".statistics"
    stats_dir.mkdir(parents=True, exist_ok=True)
    (stats_dir / "last_config").write_text(
        f"{profile.build_type}:{profile.compiler}:{profile.pch}:{profile.build_dir}\n",
        encoding="utf-8",
    )
    (stats_dir / "active_build_dir").write_text(
        f"{profile.build_dir}\n",
        encoding="utf-8",
    )


def _sync_compile_commands_for_preset(
    manager: WorkflowManager,
    ns: argparse.Namespace,
    build_preset: str,
    *,
    jobs: int | None = None,
) -> None:
    """Refresh root compile_commands symlinks for the selected preset profile."""

    argv: list[str] = [
        "cmake",
        "--build",
        "--preset",
        build_preset,
        "--target",
        "symlink_clangd",
    ]
    if jobs:
        argv.extend(["-j", str(jobs)])
    run_external_step_with_policy(manager, ns, argv)


def handle_preset_conf(manager: WorkflowManager, ns: argparse.Namespace) -> None:
    """Run centralized CMake configure preset with proper environment wiring."""

    profile = _preset_profile_from_config(ns.preset)
    algorithms_dir = discover_algorithms_dir(ns.algorithms_dir)
    env = {"CP_ALGORITHMS_DIR": str(algorithms_dir)}

    argv: list[str] = ["cmake", "--preset", ns.preset]
    if ns.fresh:
        argv.append("--fresh")

    conf_result = run_external_step_with_policy(manager, ns, argv, env_overrides=env)
    if conf_result.returncode != 0:
        return

    _sync_compile_commands_for_preset(manager, ns, profile.build_preset)
    _write_profile_metadata(manager, profile)


def handle_preset_build(manager: WorkflowManager, ns: argparse.Namespace) -> None:
    """Run centralized CMake build preset with optional target/jobs."""

    profile = _preset_profile_from_build(ns.preset)
    argv: list[str] = ["cmake", "--build", "--preset", ns.preset]
    if ns.target:
        argv.extend(["--target", ns.target])
    if ns.jobs:
        argv.extend(["-j", str(ns.jobs)])

    build_result = run_external_step_with_policy(manager, ns, argv)
    if build_result.returncode != 0:
        return

    _sync_compile_commands_for_preset(manager, ns, ns.preset, jobs=ns.jobs)
    _write_profile_metadata(manager, profile)
