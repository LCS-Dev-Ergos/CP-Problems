"""Tests for the ``profile_registry`` TOML loader.

Validates the contract that downstream consumers (``flattener_pipeline``,
``gen_config``, ``gen_scaffold``) rely on: schema-version parsing,
base-vs-strict default selection, IO profile lookup with ``NEED_*``
and define lists, scaffold recipe materialization, and cache reset
semantics between tests.
"""

from __future__ import annotations

import importlib
import tempfile
import textwrap
import unittest
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parents[1]

import profile_registry  # noqa: E402


def _write_toml(body: str) -> Path:
    tmp = Path(tempfile.mkstemp(suffix=".toml")[1])
    tmp.write_text(textwrap.dedent(body), encoding="utf-8")
    return tmp


# Every required table, each with one harmless entry. Field-level validation
# tests append the malformed table they are actually about, so the check under
# test is the one that fires rather than the required-table gate.
_MINIMAL_VALID_TOML = """\
schema_version = 1

[defaults]
CP_OK = 1

[io_profile.simple]
needs = ["NEED_IO"]

[feature.NEED_IO]
headers = ["modules/IO.hpp"]

[scaffold.base]
needs = ["NEED_IO"]
io_profile = "simple"
"""


def _write_toml_with(extra: str) -> Path:
    """Write a schema-complete registry extended with ``extra``."""

    return _write_toml(_MINIMAL_VALID_TOML + textwrap.dedent(extra))


class ProfileRegistryTests(unittest.TestCase):
    def setUp(self) -> None:
        """Reset the profile registry cache before each test to avoid cross-test state."""
        profile_registry.reset_cache()

    def test_loads_default_profiles_toml(self) -> None:
        """Default profiles.toml should parse with expected schema version and known profiles."""
        registry = profile_registry.load_registry()
        self.assertEqual(registry.schema_version, 1)
        self.assertIn("simple", registry.io_profiles)
        self.assertIn("fast_minimal", registry.io_profiles)
        self.assertIn("fast_extended", registry.io_profiles)
        self.assertEqual(
            set(registry.all_scaffold_names()),
            {"base", "default", "pbds", "advanced"},
        )

    def test_fast_minimal_profile_selects_variant_zero(self) -> None:
        """fast_minimal shares the fast backend and only pins CP_FAST_IO_VARIANT."""
        registry = profile_registry.load_registry()
        profile = registry.io_profiles["fast_minimal"]
        self.assertEqual(set(profile.needs), {"NEED_FAST_IO"})
        self.assertEqual(dict(profile.defines), {"CP_FAST_IO_VARIANT": 0})

    def test_feature_manifest_exposes_need_to_headers_mapping(self) -> None:
        """Feature headers should be available without parsing Base.hpp."""
        registry = profile_registry.load_registry()
        mapping = registry.feature_headers()
        self.assertEqual(mapping["NEED_IO"][-1], "modules/IO.hpp")
        self.assertIn("core/ContainerAliases.hpp", mapping["NEED_IO"])
        self.assertIn("core/Types.hpp", mapping["NEED_CORE"])
        self.assertIn("modules/IntegerMath.hpp", mapping["NEED_CORE"])

    def test_expand_scaffold_unions_io_needs(self) -> None:
        """Expanding 'advanced' scaffold should include fast-I/O, ModInt, and bit-ops needs."""
        registry = profile_registry.load_registry()
        needs, defines = registry.expand_scaffold("advanced")
        self.assertIn("NEED_FAST_IO", needs)
        self.assertIn("NEED_MOD_INT", needs)
        self.assertIn("NEED_BIT_OPS", needs)
        self.assertEqual(defines.get("CP_FAST_IO_ENABLE_MODINT"), 1)
        self.assertEqual(defines.get("CP_USE_GLOBAL_STD_NAMESPACE"), 1)

    def test_scaffold_policy_is_loaded_from_registry(self) -> None:
        """Scaffold strictness and local defines should come from profiles.toml."""
        registry = profile_registry.load_registry()
        profile = registry.scaffolds["base"]
        self.assertTrue(profile.strict)
        self.assertFalse(profile.header_doc)
        self.assertFalse(profile.advanced)
        self.assertEqual(dict(profile.defines), {"CP_USE_GLOBAL_STD_NAMESPACE": 1})

    def test_config_defaults_apply_strict_overrides(self) -> None:
        """Strict profile should drop the global std namespace and the core math bundle."""
        registry = profile_registry.load_registry()
        relaxed = registry.config_defaults_as_dict(strict=False)
        strict = registry.config_defaults_as_dict(strict=True)
        self.assertEqual(relaxed["CP_USE_GLOBAL_STD_NAMESPACE"], 1)
        self.assertEqual(strict["CP_USE_GLOBAL_STD_NAMESPACE"], 0)
        self.assertEqual(relaxed["CP_CORE_ENABLE_MATH"], 1)
        self.assertEqual(strict["CP_CORE_ENABLE_MATH"], 0)

    def test_composite_io_is_enabled_in_every_profile(self) -> None:
        """Structured Vec/Pair/tuple I/O must survive the strict profile."""
        registry = profile_registry.load_registry()
        self.assertEqual(
            registry.config_defaults_as_dict(strict=False)["CP_IO_ENABLE_COMPOSITE"], 1
        )
        self.assertEqual(registry.config_defaults_as_dict(strict=True)["CP_IO_ENABLE_COMPOSITE"], 1)
        self.assertNotIn("CP_IO_ENABLE_COMPOSITE", registry.defaults.strict_overrides)

    def test_io_profile_to_needs_uses_macro_names(self) -> None:
        """IO profile mapping should translate CP_IO_PROFILE_SIMPLE into NEED_IO tuple."""
        registry = profile_registry.load_registry()
        mapping = registry.io_profile_to_needs()
        self.assertIn("CP_IO_PROFILE_SIMPLE", mapping)
        self.assertEqual(mapping["CP_IO_PROFILE_SIMPLE"], ("NEED_IO",))
        self.assertIn("CP_IO_PROFILE_FAST_MINIMAL", mapping)
        self.assertEqual(mapping["CP_IO_PROFILE_FAST_MINIMAL"], ("NEED_FAST_IO",))

    def test_expand_io_profiles_accepts_macro_names_and_defines(self) -> None:
        """IO expansion should be available from profile macro names, not only TOML keys."""
        registry = profile_registry.load_registry()

        needs, defines = registry.expand_io_profiles(["CP_IO_PROFILE_FAST_EXTENDED"])

        self.assertEqual(needs, {"NEED_FAST_IO", "NEED_MOD_INT"})
        self.assertEqual(defines["CP_FAST_IO_ENABLE_MODINT"], 1)
        self.assertEqual(defines["CP_FAST_IO_ENABLE_STRONG_TYPE"], 1)

    def test_rejects_multiple_enabled_io_profiles(self) -> None:
        """The single-profile rule should be enforced by the registry itself."""
        registry = profile_registry.load_registry()

        with self.assertRaisesRegex(ValueError, "at most one CP_IO_PROFILE"):
            registry.expand_io_profiles(["CP_IO_PROFILE_SIMPLE", "CP_IO_PROFILE_FAST_MINIMAL"])

    def test_normalize_needs_matches_generated_io_precedence(self) -> None:
        """The fast backend should shadow simple IO."""
        registry = profile_registry.load_registry()

        self.assertEqual(registry.normalize_needs({"NEED_IO"}), {"NEED_IO"})
        self.assertEqual(
            registry.normalize_needs({"NEED_IO", "NEED_FAST_IO"}),
            {"NEED_FAST_IO"},
        )

    def test_rejects_unsupported_schema_version(self) -> None:
        """profiles.toml with unsupported schema_version should raise ValueError."""
        path = _write_toml("schema_version = 0\n")
        try:
            with self.assertRaisesRegex(ValueError, "schema_version"):
                profile_registry.load_registry(str(path))
        finally:
            path.unlink(missing_ok=True)

    def test_rejects_registry_missing_required_tables(self) -> None:
        """A truncated profiles.toml must fail at load, naming what is missing.

        Every table used to be optional, so a file carrying only
        ``schema_version`` produced an empty registry that failed much later as
        an empty argparse ``choices`` tuple or a bare "Unable to derive NEED_*
        mapping".
        """

        path = _write_toml("schema_version = 1\n")
        try:
            with self.assertRaisesRegex(ValueError, "missing required non-empty table"):
                profile_registry.load_registry(str(path))
        finally:
            path.unlink(missing_ok=True)

    def test_missing_table_error_names_every_absent_table(self) -> None:
        """The message must list the absent tables, not just the first one."""

        path = _write_toml(
            """\
            schema_version = 1

            [defaults]
            CP_OK = 1
            """
        )
        try:
            with self.assertRaises(ValueError) as ctx:
                profile_registry.load_registry(str(path))
        finally:
            path.unlink(missing_ok=True)

        message = str(ctx.exception)
        for table in ("io_profile", "feature", "scaffold"):
            self.assertIn(table, message)
        self.assertNotIn("defaults", message.rsplit(":", maxsplit=1)[-1])

    def test_present_but_empty_table_is_rejected(self) -> None:
        """An empty ``[scaffold]`` is as unusable as an absent one."""

        path = _write_toml(
            """\
            schema_version = 1

            [defaults]
            CP_OK = 1

            [io_profile.simple]
            needs = ["NEED_IO"]

            [feature.NEED_IO]
            headers = ["modules/IO.hpp"]

            [scaffold]
            """
        )
        try:
            with self.assertRaisesRegex(ValueError, "missing required non-empty table"):
                profile_registry.load_registry(str(path))
        finally:
            path.unlink(missing_ok=True)

    def test_scalar_where_a_named_table_is_expected_is_rejected(self) -> None:
        """``[scaffold]`` holding scalars (a mistyped ``[scaffold.base]``) fails cleanly.

        Previously this reached ``_scaffold`` and raised ``AttributeError``,
        which reads as a crash rather than a configuration error.
        """

        path = _write_toml(
            """\
            schema_version = 1

            [defaults]
            CP_OK = 1

            [io_profile.simple]
            needs = ["NEED_IO"]

            [feature.NEED_IO]
            headers = ["modules/IO.hpp"]

            [scaffold]
            needs = ["NEED_IO"]
            io_profile = "simple"
            """
        )
        try:
            with self.assertRaisesRegex(ValueError, r"scaffold\.needs: expected a table"):
                profile_registry.load_registry(str(path))
        finally:
            path.unlink(missing_ok=True)

    def test_rejects_scaffold_referencing_unknown_io_profile(self) -> None:
        """Scaffold that references a non-existent io_profile should raise ValueError."""
        path = _write_toml_with(
            """
            [scaffold.broken]
            needs = ["NEED_MACROS"]
            io_profile = "does_not_exist"
            """
        )
        try:
            with self.assertRaisesRegex(ValueError, "io_profile"):
                profile_registry.load_registry(str(path))
        finally:
            path.unlink(missing_ok=True)

    def test_rejects_non_int_default(self) -> None:
        """Defaults section containing non-integer values should raise ValueError."""
        # Spelled out rather than composed: ``[defaults]`` already exists in the
        # shared fixture and TOML forbids redefining a table.
        path = _write_toml(
            """\
            schema_version = 1

            [defaults]
            CP_BAD = "not-an-int"

            [io_profile.simple]
            needs = ["NEED_IO"]

            [feature.NEED_IO]
            headers = ["modules/IO.hpp"]

            [scaffold.base]
            needs = ["NEED_IO"]
            io_profile = "simple"
            """
        )
        try:
            with self.assertRaisesRegex(ValueError, "expected int"):
                profile_registry.load_registry(str(path))
        finally:
            path.unlink(missing_ok=True)

    def test_rejects_non_list_needs(self) -> None:
        """io_profile with a string instead of a list for 'needs' should raise ValueError."""
        path = _write_toml_with(
            """
            [io_profile.broken]
            needs = "NEED_IO"
            """
        )
        try:
            with self.assertRaisesRegex(ValueError, "needs"):
                profile_registry.load_registry(str(path))
        finally:
            path.unlink(missing_ok=True)

    def test_rejects_non_need_feature_names(self) -> None:
        """Feature manifest table names should be explicit NEED_* macros."""
        path = _write_toml_with(
            """
            [feature.CP_BAD]
            headers = ["core/Macros.hpp"]
            """
        )
        try:
            with self.assertRaisesRegex(ValueError, "NEED_"):
                profile_registry.load_registry(str(path))
        finally:
            path.unlink(missing_ok=True)

    def test_rejects_non_bool_scaffold_flags(self) -> None:
        """Scaffold booleans should reject strings instead of using Python truthiness."""
        path = _write_toml_with(
            """
            [scaffold.broken]
            needs = ["NEED_MACROS"]
            io_profile = "simple"
            strict = "false"
            """
        )
        try:
            with self.assertRaisesRegex(ValueError, "expected bool"):
                profile_registry.load_registry(str(path))
        finally:
            path.unlink(missing_ok=True)


class GeneratedArtefactStalenessTests(unittest.TestCase):
    """Regenerate the committed outputs and assert nothing drifts from the spec."""

    def setUp(self) -> None:
        """Reset cache and resolve the templates directory for every staleness test."""
        profile_registry.reset_cache()
        self.templates_dir = SCRIPTS_DIR.parent / "templates"

    def _backup_and_run(self, generator_module_name: str, output_paths: list[Path]) -> None:
        """Import a generator, run it, and assert no drift from committed files."""

        module = importlib.import_module(generator_module_name)
        backups = {p: p.read_text(encoding="utf-8") for p in output_paths if p.is_file()}
        try:
            rc = module.main()
            self.assertEqual(rc, 0)
            for path in output_paths:
                self.assertTrue(path.is_file(), f"{path} missing after regenerate")
                current = path.read_text(encoding="utf-8")
                self.assertEqual(
                    current,
                    backups.get(path, current),
                    f"{path} drifted from profiles.toml; run scripts/{generator_module_name}.py",
                )
        finally:
            for path, content in backups.items():
                path.write_text(content, encoding="utf-8")

    def test_gen_config_outputs_match_committed_files(self) -> None:
        """gen_config.py should not alter committed generated config headers."""
        self._backup_and_run(
            "gen_config",
            [
                self.templates_dir / "core" / "Config_defaults.hpp",
                self.templates_dir / "Base_profiles.hpp",
                self.templates_dir / "Base_features.hpp",
            ],
        )

    def test_gen_scaffold_outputs_match_committed_files(self) -> None:
        """gen_scaffold.py should not alter any committed cpp/ starter files."""
        registry = profile_registry.load_registry()
        scaffold_paths = [
            self.templates_dir / "cpp" / f"{name}.cpp" for name in registry.all_scaffold_names()
        ]
        self._backup_and_run("gen_scaffold", scaffold_paths)


if __name__ == "__main__":
    unittest.main()
