#!/usr/bin/env python3
"""Typed loader for ``templates/profiles.toml`` (uses :mod:`tomllib`).

Parses the central profile registry into immutable dataclasses exposing:
``config_defaults`` (base vs strict CP_* values), ``io_profiles`` (per
``CP_IO_PROFILE_*`` ``NEED_*`` lists + extra defines), feature headers
(``NEED_*`` to template headers), and scaffold recipes. Consumers
(``flattener_pipeline.macro_resolution``, ``gen_config``, ``gen_scaffold``)
reach the registry exclusively through this loader, so schema changes stay
in one place.
"""

from __future__ import annotations

import tomllib
from collections import OrderedDict
from collections.abc import Iterable, Mapping
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path
from typing import Any

SCRIPTS_DIR = Path(__file__).resolve().parent
TEMPLATES_DIR = SCRIPTS_DIR.parent / "templates"
DEFAULT_PROFILES_PATH = TEMPLATES_DIR / "profiles.toml"

NEED_IO = "NEED_IO"
NEED_FAST_IO = "NEED_FAST_IO"

# ``target`` is removed when any blocker is present. The same rules drive
# Python-side flattening and generated C++ profile normalization.
NEED_SHADOW_RULES: tuple[tuple[str, frozenset[str]], ...] = ((NEED_IO, frozenset({NEED_FAST_IO})),)


@dataclass(frozen=True, slots=True)
class IOProfile:
    """One named ``CP_IO_PROFILE_*`` expansion from ``profiles.toml``."""

    name: str
    needs: frozenset[str]
    defines: Mapping[str, int]

    @property
    def macro(self) -> str:
        """Return the public ``CP_IO_PROFILE_*`` macro name for this profile."""

        return f"CP_IO_PROFILE_{self.name.upper()}"


@dataclass(frozen=True, slots=True)
class ScaffoldProfile:
    """One generated ``templates/cpp/*.cpp`` scaffold recipe."""

    name: str
    description: str
    needs: frozenset[str]
    io_profile: str
    header_doc: bool
    strict: bool
    advanced: bool
    defines: Mapping[str, int]


@dataclass(frozen=True, slots=True)
class ConditionalHeaders:
    """Headers guarded by one preprocessor condition inside a feature block."""

    condition: str
    headers: tuple[str, ...]


@dataclass(frozen=True, slots=True)
class FeatureProfile:
    """One ``NEED_*`` feature and the template headers it includes."""

    name: str
    headers: tuple[str, ...]
    conditional_headers: tuple[ConditionalHeaders, ...]

    def all_headers(self) -> tuple[str, ...]:
        """Return direct and conditional headers in generated include order."""

        headers: list[str] = list(self.headers)
        for group in self.conditional_headers:
            headers.extend(group.headers)
        return tuple(headers)


@dataclass(frozen=True, slots=True)
class ToolingConfig:
    """Compiler selection for the probe/verification tooling."""

    compiler: str | None
    compiler_flags: tuple[str, ...]


@dataclass(frozen=True, slots=True)
class ConfigDefaults:
    """Base and strict-profile ``CP_*`` default values."""

    base: Mapping[str, int]
    strict_overrides: Mapping[str, int]


@dataclass(frozen=True, slots=True)
class ProfileRegistry:
    """Validated in-memory representation of ``templates/profiles.toml``."""

    schema_version: int
    io_profiles: Mapping[str, IOProfile]
    features: Mapping[str, FeatureProfile]
    scaffolds: Mapping[str, ScaffoldProfile]
    defaults: ConfigDefaults
    tooling: ToolingConfig

    def expand_io(self, name: str) -> tuple[frozenset[str], dict[str, int]]:
        """Return the direct ``NEED_*`` and ``CP_*`` defines for one IO profile."""

        profile = self.io_profiles[name]
        return profile.needs, dict(profile.defines)

    def need_shadow_rules(self) -> tuple[tuple[str, frozenset[str]], ...]:
        """Return canonical ``NEED_*`` precedence rules used by C++ and Python."""

        return NEED_SHADOW_RULES

    def io_profile_macros(self) -> tuple[str, ...]:
        """Return all public ``CP_IO_PROFILE_*`` macro names in stable order."""

        return tuple(sorted(profile.macro for profile in self.io_profiles.values()))

    def io_profile_name_from_macro(self, macro: str) -> str | None:
        """Translate a public ``CP_IO_PROFILE_*`` macro to its registry key."""

        for name, profile in self.io_profiles.items():
            if profile.macro == macro:
                return name
        return None

    def normalize_needs(self, needs: Iterable[str]) -> frozenset[str]:
        """Apply canonical shadowing rules to an arbitrary set of ``NEED_*`` macros."""

        values = set(needs)
        for target, blockers in self.need_shadow_rules():
            if target in values and values & blockers:
                values.discard(target)
        return frozenset(values)

    def expand_io_profiles(
        self, enabled_profiles: Iterable[str]
    ) -> tuple[frozenset[str], dict[str, int]]:
        """Expand enabled IO profiles into normalized needs and extra defines.

        Entries may be TOML profile keys (``fast_extended``) or public macro
        names (``CP_IO_PROFILE_FAST_EXTENDED``). Unknown names are ignored so
        callers can pass the raw set of macros seen in a source file.
        """

        names: list[str] = []
        for profile in enabled_profiles:
            name = self.io_profile_name_from_macro(profile) or profile
            if name not in self.io_profiles:
                continue
            if name not in names:
                names.append(name)
        if len(names) > 1:
            selected = ", ".join(self.io_profiles[name].macro for name in names)
            raise ValueError(f"choose at most one CP_IO_PROFILE_* macro: {selected}")

        needs: set[str] = set()
        defines: dict[str, int] = {}
        for name in names:
            profile_needs, profile_defines = self.expand_io(name)
            needs.update(profile_needs)
            defines.update(profile_defines)
        return self.normalize_needs(needs), defines

    def expand_scaffold(self, name: str) -> tuple[frozenset[str], dict[str, int]]:
        """Return the effective needs and defines implied by a scaffold profile."""

        scaffold = self.scaffolds[name]
        io_needs, io_defines = self.expand_io(scaffold.io_profile)
        defines = dict(io_defines)
        defines.update(scaffold.defines)
        return self.normalize_needs(scaffold.needs | io_needs), defines

    def feature_headers(self) -> OrderedDict[str, list[str]]:
        """Return the canonical ``NEED_*`` to header-list mapping for tooling."""

        mapping: OrderedDict[str, list[str]] = OrderedDict()
        for name, feature in self.features.items():
            mapping[name] = list(feature.all_headers())
        return mapping

    def all_scaffold_names(self) -> tuple[str, ...]:
        """Return scaffold names in registry order."""

        return tuple(self.scaffolds.keys())

    def config_defaults_as_dict(self, *, strict: bool) -> dict[str, int]:
        """Return base defaults with strict overrides applied when requested."""

        values = dict(self.defaults.base)
        if strict:
            values.update(self.defaults.strict_overrides)
        return values

    def io_profile_to_needs(self) -> dict[str, tuple[str, ...]]:
        """Return public IO-profile macro names mapped to normalized need tuples."""

        return {
            profile.macro: tuple(sorted(self.normalize_needs(profile.needs)))
            for profile in self.io_profiles.values()
        }


def _int_map(payload: object, *, where: str) -> dict[str, int]:
    """Validate and return a TOML table containing string keys and int values."""

    if not isinstance(payload, dict):
        raise ValueError(f"{where}: expected a table")
    out: dict[str, int] = {}
    for k, v in payload.items():
        if not isinstance(k, str) or isinstance(v, bool) or not isinstance(v, int):
            raise ValueError(f"{where}.{k}: expected int")
        out[k] = v
    return out


def _bool_field(
    payload: Mapping[str, Any],
    key: str,
    *,
    default: bool,
    where: str,
) -> bool:
    """Read one optional boolean field without coercing non-bool TOML values."""

    if key not in payload:
        return default
    value = payload[key]
    if not isinstance(value, bool):
        raise ValueError(f"{where}.{key}: expected bool")
    return value


def _tooling(payload: object) -> ToolingConfig:
    """Build a validated :class:`ToolingConfig` from the ``[tooling]`` table."""

    if payload is None:
        return ToolingConfig(compiler=None, compiler_flags=())
    if not isinstance(payload, dict):
        raise ValueError("tooling: expected a table")

    compiler = payload.get("compiler", "")
    if not isinstance(compiler, str):
        raise ValueError("tooling.compiler: expected string")

    return ToolingConfig(
        compiler=compiler.strip() or None,
        compiler_flags=_str_list(payload.get("compiler_flags", []), where="tooling.compiler_flags"),
    )


def _io(name: str, payload: dict[str, Any]) -> IOProfile:
    """Build a validated :class:`IOProfile` from one TOML table."""

    needs = payload.get("needs", [])
    if not isinstance(needs, list) or not all(isinstance(n, str) for n in needs):
        raise ValueError(f"io_profile.{name}.needs must be list[str]")
    return IOProfile(
        name=name,
        needs=frozenset(needs),
        defines=_int_map(payload.get("defines", {}) or {}, where=f"io_profile.{name}.defines"),
    )


def _str_list(payload: object, *, where: str) -> tuple[str, ...]:
    """Validate and return a TOML list containing only strings."""

    if not isinstance(payload, list) or not all(isinstance(v, str) for v in payload):
        raise ValueError(f"{where}: expected list[str]")
    return tuple(payload)


def _conditional_headers(payload: object, *, where: str) -> tuple[ConditionalHeaders, ...]:
    """Validate conditional feature-header groups from one TOML feature table."""

    if payload is None:
        return ()
    if not isinstance(payload, list):
        raise ValueError(f"{where}: expected list[table]")

    groups: list[ConditionalHeaders] = []
    for idx, entry in enumerate(payload):
        entry_where = f"{where}[{idx}]"
        if not isinstance(entry, dict):
            raise ValueError(f"{entry_where}: expected table")
        condition = entry.get("if", "")
        if not isinstance(condition, str) or not condition:
            raise ValueError(f"{entry_where}.if: expected non-empty string")
        groups.append(
            ConditionalHeaders(
                condition=condition,
                headers=_str_list(entry.get("headers", []), where=f"{entry_where}.headers"),
            )
        )
    return tuple(groups)


def _feature(name: str, payload: dict[str, Any]) -> FeatureProfile:
    """Build a validated :class:`FeatureProfile` from one TOML table."""

    if not name.startswith("NEED_"):
        raise ValueError(f"feature.{name}: expected NEED_* table name")
    return FeatureProfile(
        name=name,
        headers=_str_list(payload.get("headers", []), where=f"feature.{name}.headers"),
        conditional_headers=_conditional_headers(
            payload.get("conditional_headers"),
            where=f"feature.{name}.conditional_headers",
        ),
    )


def _scaffold(name: str, payload: dict[str, Any], known_io: frozenset[str]) -> ScaffoldProfile:
    """Build a validated :class:`ScaffoldProfile` from one TOML table."""

    needs = payload.get("needs", [])
    io_profile = payload.get("io_profile", "")
    if not isinstance(needs, list) or not all(isinstance(n, str) for n in needs):
        raise ValueError(f"scaffold.{name}.needs must be list[str]")
    if not isinstance(io_profile, str) or io_profile not in known_io:
        raise ValueError(f"scaffold.{name}.io_profile must reference a known io_profile")
    return ScaffoldProfile(
        name=name,
        description=str(payload.get("description", "")),
        needs=frozenset(needs),
        io_profile=io_profile,
        header_doc=_bool_field(payload, "header_doc", default=False, where=f"scaffold.{name}"),
        strict=_bool_field(payload, "strict", default=False, where=f"scaffold.{name}"),
        advanced=_bool_field(payload, "advanced", default=False, where=f"scaffold.{name}"),
        defines=_int_map(payload.get("defines", {}) or {}, where=f"scaffold.{name}.defines"),
    )


# Tables without which the registry cannot answer the questions its consumers
# ask. Every one of them was previously optional, so a truncated profiles.toml
# loaded "successfully" into an empty registry and surfaced three hops later as
# an empty argparse ``choices`` tuple or a bare "Unable to derive NEED_*
# mapping". Fail here instead, naming the file and the missing table.
_REQUIRED_TABLES: tuple[str, ...] = ("defaults", "io_profile", "feature", "scaffold")


def _sub_tables(payload: dict[str, Any], key: str) -> dict[str, dict[str, Any]]:
    """Return ``payload[key]`` as a table-of-tables, rejecting other shapes.

    Writing ``[scaffold]`` where ``[scaffold.base]`` was meant leaves a table
    whose values are plain scalars/lists. Without this guard the per-entry
    builders reach for ``.get`` on those values and raise ``AttributeError``,
    which reads like a tool crash rather than a config error.
    """

    table = payload.get(key) or {}
    if not isinstance(table, dict):
        raise ValueError(f"{key}: expected a table of named entries")
    for name, entry in table.items():
        if not isinstance(entry, dict):
            raise ValueError(f"{key}.{name}: expected a table (got {type(entry).__name__})")
    return table


def _build(payload: dict[str, Any], *, source: str | None = None) -> ProfileRegistry:
    """Build a validated registry from a parsed TOML payload."""

    schema = payload.get("schema_version", 0)
    if schema != 1:
        raise ValueError(f"unsupported schema_version: {schema}")

    where = f" in {source}" if source else ""
    missing = [name for name in _REQUIRED_TABLES if not payload.get(name)]
    if missing:
        raise ValueError(
            f"profiles.toml is missing required non-empty table(s){where}: {', '.join(missing)}"
        )

    defaults_payload = dict(payload.get("defaults", {}) or {})
    strict = defaults_payload.pop("strict_overrides", {}) or {}
    defaults = ConfigDefaults(
        base=_int_map(defaults_payload, where="defaults"),
        strict_overrides=_int_map(strict, where="defaults.strict_overrides"),
    )

    io_profiles = {n: _io(n, b) for n, b in _sub_tables(payload, "io_profile").items()}
    known_io = frozenset(io_profiles.keys())

    features = {n: _feature(n, b) for n, b in _sub_tables(payload, "feature").items()}

    scaffolds = {n: _scaffold(n, b, known_io) for n, b in _sub_tables(payload, "scaffold").items()}

    return ProfileRegistry(
        schema_version=schema,
        io_profiles=io_profiles,
        features=features,
        scaffolds=scaffolds,
        defaults=defaults,
        tooling=_tooling(payload.get("tooling")),
    )


@lru_cache(maxsize=4)
def load_registry(path: str | None = None) -> ProfileRegistry:
    """Load and cache a profile registry from ``path`` or the default TOML file."""

    resolved = Path(path) if path else DEFAULT_PROFILES_PATH
    with open(resolved, "rb") as f:
        return _build(tomllib.load(f), source=str(resolved))


def reset_cache() -> None:
    """Clear the cached registry, primarily for tests and generators."""

    load_registry.cache_clear()
