"""Focused audit tests for flattener correctness, safety, and robustness.

Covers the historically-tricky surfaces of the source flattener: include
resolution and path escapes, preprocessor folding under known macro
state, ``NEED_*`` parsing edge cases, mode-specific output shape
(safe / compact / submission / auto), and project-wide layout invariants
(``.clangd`` include paths, canonical-include enforcement across modules).

Most tests in here spawn a fresh Python interpreter via ``subprocess``;
new tests should prefer the lighter ``flatten_inproc`` pytest fixture
demonstrated in ``test_flattener_inproc.py``.
"""

from __future__ import annotations

import os
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parents[1]
FLATTENER_SCRIPT = SCRIPTS_DIR / "flattener.py"
ALGORITHMS_DIR = SCRIPTS_DIR.parent

from flattener_core.includes import (  # noqa: E402
    collect_transitive_template_deps,
    extract_prefix_before_base_include,
    inline_local_header,
    order_template_headers_by_dependencies,
    parse_project_include_line,
    resolve_project_include,
)
from flattener_core.lexer import extract_identifiers, strip_comments, strip_non_code  # noqa: E402
from flattener_core.preprocessor import fold_simple_preprocessor_conditionals  # noqa: E402
from flattener_pipeline.macros import extract_macro_values_from_source  # noqa: E402
from need_resolver import extract_need_macros_from_source, load_need_mapping  # noqa: E402


class FlattenerAuditTests(unittest.TestCase):
    def _project_header_graph(self) -> dict[str, list[str]]:
        """Build a dependency graph of all project .hpp headers by scanning includes.

        Only project-local .hpp targets are kept; the debug.h stub is allowed
        because it is resolved outside the Algorithms tree at runtime.
        """
        root = ALGORITHMS_DIR
        headers = sorted((root / "templates").rglob("*.hpp"))
        headers += sorted((root / "modules").glob("**/*.hpp"))
        graph: dict[str, list[str]] = {}

        for header in headers:
            content = header.read_text(encoding="utf-8")
            masked_lines = strip_non_code(content).splitlines()
            rel_header = header.relative_to(root).as_posix()
            edges: list[str] = []

            for idx, line in enumerate(content.splitlines()):
                masked_line = masked_lines[idx] if idx < len(masked_lines) else ""
                include_name = parse_project_include_line(line, masked_line=masked_line)
                if not include_name:
                    continue

                target = resolve_project_include(root, header, include_name)
                if target is None:
                    # debug.h is intentionally local-only and supplied outside
                    # the Algorithms tree when LOCAL debugging is enabled.
                    self.assertEqual(
                        (rel_header, include_name), ("templates/core/Debug.hpp", "debug.h")
                    )
                    continue

                if target.suffix == ".hpp" and target != header.resolve():
                    edges.append(target.relative_to(root).as_posix())

            graph[rel_header] = edges

        return graph

    def test_template_headers_use_canonical_non_relative_includes(self) -> None:
        """``templates/*.hpp`` must use project-rooted include paths.

        Modules deliberately use ``"../_Common.hpp"`` style references for
        per-domain umbrella headers and sibling/cross-domain links, so this
        check is scoped to ``templates/`` where canonical paths are
        actually the convention.
        """

        root = ALGORITHMS_DIR
        headers = sorted((root / "templates").rglob("*.hpp"))
        relative_includes: list[tuple[str, str]] = []

        for header in headers:
            content = header.read_text(encoding="utf-8")
            masked_lines = strip_non_code(content).splitlines()
            for idx, line in enumerate(content.splitlines()):
                include_name = parse_project_include_line(
                    line,
                    masked_line=masked_lines[idx] if idx < len(masked_lines) else "",
                )
                if include_name and include_name.startswith("../"):
                    relative_includes.append((header.relative_to(root).as_posix(), include_name))

        self.assertEqual(relative_includes, [])

    def test_project_header_include_graph_is_acyclic(self) -> None:
        graph = self._project_header_graph()
        visiting: set[str] = set()
        visited: set[str] = set()
        stack: list[str] = []
        cycles: list[list[str]] = []

        def dfs(node: str) -> None:
            if node in visiting:
                cycles.append([*stack[stack.index(node) :], node])
                return
            if node in visited:
                return

            visiting.add(node)
            stack.append(node)
            for child in graph.get(node, []):
                dfs(child)
            stack.pop()
            visiting.remove(node)
            visited.add(node)

        for node in graph:
            dfs(node)

        self.assertEqual(cycles, [])

    def test_extract_need_macros_expands_io_profile_simple(self) -> None:
        source = textwrap.dedent(
            """\
            #define CP_IO_PROFILE_SIMPLE
            #include "templates/Base.hpp"
            """
        )
        known = {"NEED_CORE", "NEED_IO", "NEED_FAST_IO", "NEED_MOD_INT", "NEED_TYPE_SAFETY"}

        found = extract_need_macros_from_source(source, known)

        self.assertEqual(found, {"NEED_IO"})

    def test_extract_need_macros_expands_io_profile_fast_extended(self) -> None:
        source = textwrap.dedent(
            """\
            #define CP_IO_PROFILE_FAST_EXTENDED 1
            #undef CP_IO_PROFILE_FAST_EXTENDED
            #define CP_IO_PROFILE_FAST_EXTENDED 1
            #include "templates/Base.hpp"
            """
        )
        known = {"NEED_CORE", "NEED_IO", "NEED_FAST_IO", "NEED_MOD_INT"}

        found = extract_need_macros_from_source(source, known)

        self.assertEqual(found, {"NEED_FAST_IO", "NEED_MOD_INT"})

    def test_extract_need_macros_expands_io_profile_fast_minimal(self) -> None:
        source = textwrap.dedent(
            """\
            #define CP_IO_PROFILE_FAST_MINIMAL
            #include "templates/Base.hpp"
            """
        )
        known = {"NEED_CORE", "NEED_IO", "NEED_FAST_IO", "NEED_MOD_INT"}

        found = extract_need_macros_from_source(source, known)

        self.assertEqual(found, {"NEED_FAST_IO"})

    def test_extract_need_macros_handles_values_undef_and_comments(self) -> None:
        source = textwrap.dedent(
            """\
            /*
            #define NEED_IO
            */
            #if 0
            #define NEED_IO
            #endif
            #define NEED_IO 1
            #define NEED_FAST_IO 1 // inline comment
            #undef NEED_IO
            #include "templates/Base.hpp"
            #define NEED_IO
            """
        )
        known = {"NEED_CORE", "NEED_IO", "NEED_FAST_IO"}

        found = extract_need_macros_from_source(source, known)

        self.assertEqual(found, {"NEED_FAST_IO"})

    def test_load_need_mapping_ignores_else_branch_of_need_guard(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            base = Path(tmpdir) / "Base.hpp"
            base.write_text(
                textwrap.dedent(
                    """\
                    #ifdef NEED_X
                    #include "A.hpp"
                    #else
                    #include "B.hpp"
                    #endif
                    """
                ),
                encoding="utf-8",
            )

            mapping = load_need_mapping(base)
            self.assertEqual(mapping.get("NEED_X"), ["A.hpp"])

    def test_inline_header_tracks_function_like_macros_before_nested_includes(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            templates = root / "templates"
            templates.mkdir()
            (templates / "A.hpp").write_text(
                textwrap.dedent(
                    """\
                    #define CP_IMPL(...) impl(__VA_ARGS__)
                    #include "B.hpp"
                    """
                ),
                encoding="utf-8",
            )
            (templates / "B.hpp").write_text(
                textwrap.dedent(
                    """\
                    #ifndef CP_IMPL
                      #define CP_IMPL(...) fallback(__VA_ARGS__)
                    #endif
                    int value = 0;
                    """
                ),
                encoding="utf-8",
            )

            output = inline_local_header(root, templates / "A.hpp", set())

            self.assertNotIn("fallback", output)
            self.assertIn("int value = 0;", output)

    def test_template_headers_are_topologically_ordered_without_expanding_closure(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            templates = root / "templates"
            templates.mkdir()
            a = templates / "A.hpp"
            b = templates / "B.hpp"
            internal = templates / "Internal.hpp"

            a.write_text('#include "B.hpp"\n#include "Internal.hpp"\n', encoding="utf-8")
            b.write_text("using B = int;\n", encoding="utf-8")
            internal.write_text("using Internal = int;\n", encoding="utf-8")

            ordered = order_template_headers_by_dependencies(root, [a, b])

            self.assertEqual([path.name for path in ordered], ["B.hpp", "A.hpp"])

    def test_module_dependency_collection_does_not_reenter_preamble_fallback(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            templates = root / "templates"
            modules = root / "modules" / "data_structures"
            templates.mkdir(parents=True)
            modules.mkdir(parents=True)

            (templates / "Preamble.hpp").write_text('#include "StdHeaders.hpp"\n', encoding="utf-8")
            (templates / "StdHeaders.hpp").write_text(
                textwrap.dedent(
                    """\
                    #if CP_USE_BITS_HEADER
                      #include <bits/stdc++.h>
                    #endif
                    #if !CP_USE_BITS_HEADER
                      #include "PortableStdHeaders.hpp"
                    #endif
                    """
                ),
                encoding="utf-8",
            )
            (templates / "PortableStdHeaders.hpp").write_text(
                "#include <vector>\n", encoding="utf-8"
            )
            (templates / "Types.hpp").write_text('#include "Preamble.hpp"\n', encoding="utf-8")
            (modules / "_Common.hpp").write_text(
                '#include "../../templates/Types.hpp"\n', encoding="utf-8"
            )
            start = modules / "SegmentTree.hpp"
            start.write_text('#include "_Common.hpp"\n', encoding="utf-8")

            deps = collect_transitive_template_deps(
                root,
                start,
                exclude_template_files={"Preamble.hpp"},
                macro_values={"CP_USE_BITS_HEADER": 1},
            )

            self.assertEqual([dep.name for dep in deps], ["Types.hpp"])

    def test_extract_need_macros_ignores_directives_inside_raw_string(self) -> None:
        source = textwrap.dedent(
            """\
            const char* payload = R"RAW(
            #define NEED_IO
            #include "templates/Base.hpp"
            )RAW";
            #define NEED_FAST_IO 1
            #include "templates/Base.hpp"
            """
        )
        known = {"NEED_CORE", "NEED_IO", "NEED_FAST_IO"}

        found = extract_need_macros_from_source(source, known)

        self.assertEqual(found, {"NEED_FAST_IO"})

    def test_lexer_preserves_comment_markers_inside_raw_strings(self) -> None:
        source = 'auto s = R"(example /* not a comment */ )"; /* real comment */ int x;'

        stripped = strip_comments(source)

        self.assertIn('R"(example /* not a comment */ )"', stripped)
        self.assertNotIn("real comment", stripped)

    def test_prefixed_string_literals_do_not_leak_prefix_identifiers(self) -> None:
        source = "auto s = u8\"ModInt\"; auto c = L'x'; int value = 0;"

        identifiers = extract_identifiers(source)

        self.assertNotIn("u8", identifiers)
        self.assertNotIn("L", identifiers)
        self.assertNotIn("ModInt", identifiers)
        self.assertIn("value", identifiers)

    def test_extract_macro_values_stops_at_base_include(self) -> None:
        source = textwrap.dedent(
            """\
            /* #define CP_ENABLE_LEGACY_IO_VEC_MACROS 1 */
            #define CP_ENABLE_LEGACY_IO_VEC_MACROS 0
            #include "templates/Base.hpp"
            #define CP_ENABLE_LEGACY_IO_VEC_MACROS 1
            """
        )
        prefix = extract_prefix_before_base_include(source)
        values = extract_macro_values_from_source(prefix, strict_profile_enabled=False)

        self.assertEqual(values.get("CP_ENABLE_LEGACY_IO_VEC_MACROS"), 0)

    def test_extract_macro_values_expands_io_profile_fast_extended(self) -> None:
        source = textwrap.dedent(
            """\
            #define CP_IO_PROFILE_FAST_EXTENDED
            #include "templates/Base.hpp"
            """
        )
        prefix = extract_prefix_before_base_include(source)
        values = extract_macro_values_from_source(prefix, strict_profile_enabled=False)

        self.assertEqual(values.get("NEED_FAST_IO"), 1)
        self.assertEqual(values.get("NEED_MOD_INT"), 1)
        self.assertEqual(values.get("CP_USE_ADVANCED"), 1)
        self.assertEqual(values.get("CP_FAST_IO_ENABLE_MODINT"), 1)
        self.assertEqual(values.get("CP_FAST_IO_ENABLE_STRONG_TYPE"), 1)

    def test_extract_macro_values_expands_io_profile_fast_minimal(self) -> None:
        source = textwrap.dedent(
            """\
            #define CP_IO_PROFILE_FAST_MINIMAL
            #include "templates/Base.hpp"
            """
        )
        prefix = extract_prefix_before_base_include(source)
        values = extract_macro_values_from_source(prefix, strict_profile_enabled=False)

        self.assertEqual(values.get("NEED_FAST_IO"), 1)
        self.assertEqual(values.get("CP_FAST_IO_VARIANT"), 0)
        self.assertNotIn("NEED_MOD_INT", values)

    def test_fold_simple_conditionals_supports_numeric_literals(self) -> None:
        content = textwrap.dedent(
            """\
            #if 0
            DROP_A
            #endif
            #if 1
            KEEP_B
            #endif
            #if !0
            KEEP_C
            #endif
            #if !1
            DROP_D
            #endif
            """
        )

        folded = fold_simple_preprocessor_conditionals(content, {})

        self.assertIn("KEEP_B", folded)
        self.assertIn("KEEP_C", folded)
        self.assertNotIn("DROP_A", folded)
        self.assertNotIn("DROP_D", folded)
        self.assertNotIn("#if", folded)
        self.assertNotIn("#endif", folded)

    def test_fold_unknown_conditional_does_not_leak_macro_state(self) -> None:
        content = textwrap.dedent(
            """\
            #if FOO + BAR
            #define X 1
            #endif
            #if X
            int yes;
            #endif
            """
        )

        folded = fold_simple_preprocessor_conditionals(content, {})

        self.assertIn("#if FOO + BAR", folded)
        self.assertIn("#if X", folded)
        self.assertIn("#define X 1", folded)
        self.assertIn("int yes;", folded)
        self.assertNotIn("\n#endif\nint yes;\n", folded)

    def test_resolve_project_include_rejects_parent_escape(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            project_root = tmp / "project"
            project_root.mkdir()
            inside = project_root / "inside.hpp"
            inside.write_text("// in\n", encoding="utf-8")
            including_file = project_root / "sub" / "main.hpp"
            including_file.parent.mkdir()
            including_file.write_text('#include "../inside.hpp"\n', encoding="utf-8")

            outside = tmp / "outside.hpp"
            outside.write_text("// out\n", encoding="utf-8")

            resolved_inside = resolve_project_include(project_root, including_file, "../inside.hpp")
            resolved_escape = resolve_project_include(
                project_root, including_file, "../../outside.hpp"
            )

            self.assertEqual(resolved_inside, inside.resolve())
            self.assertIsNone(resolved_escape)

    # NOTE: test_flattener_accepts_need_macro_with_value migrated to
    # tests/test_flattener_inproc.py (in-process pytest fixture variant).

    def test_strict_simple_io_profile_avoids_core_alias_bloat(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define CP_TEMPLATE_PROFILE_STRICT
                    #define CP_IO_PROFILE_SIMPLE
                    #include "templates/Base.hpp"
                    auto main() -> int {
                      INT(x);
                      OUT(x);
                      return 0;
                    }
                    """
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "compact"},
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            self.assertIn("/* Lightweight I/O Utilities */", result.stdout)
            self.assertNotIn("/* Container and Utility Aliases */", result.stdout)
            self.assertNotIn("/* Mathematical Constants and Infinity Values */", result.stdout)
            self.assertNotIn("/* Mathematical Utilities */", result.stdout)

    # NOTE: test_flattener_expands_base_include_with_trailing_comment migrated
    # to tests/test_flattener_inproc.py (in-process pytest fixture variant).

    def test_flattener_preserves_global_std_namespace_opt_in(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define CP_TEMPLATE_PROFILE_STRICT
                    #ifndef CP_USE_GLOBAL_STD_NAMESPACE
                      #define CP_USE_GLOBAL_STD_NAMESPACE 1
                    #endif
                    #define NEED_CORE
                    #include "templates/Base.hpp"
                    auto main() -> int { return min(1, 2); }
                    """
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "safe"},
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            self.assertIn("using namespace std;", result.stdout)

    def test_flattener_places_global_std_namespace_before_solver_section(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define CP_TEMPLATE_PROFILE_STRICT
                    #ifndef CP_USE_GLOBAL_STD_NAMESPACE
                      #define CP_USE_GLOBAL_STD_NAMESPACE 1
                    #endif
                    #define NEED_CORE
                    #define NEED_IO
                    #include "templates/Base.hpp"

                    //===----------------------------------------------------------------------===//
                    /* Main Solver Function */

                    auto main() -> int { return min(1, 2); }
                    """
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "safe"},
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            self.assertLess(
                result.stdout.index("/* Lightweight I/O Utilities */"),
                result.stdout.index("using namespace std;"),
            )
            self.assertLess(
                result.stdout.index("/* Main Solver Function */"),
                result.stdout.index("using namespace std;"),
            )
            self.assertLess(
                result.stdout.index("using namespace std;"),
                result.stdout.index("auto main()"),
            )

    # NOTE: test_flattener_keeps_reverse_loop_macros_from_need_core migrated
    # to tests/test_flattener_inproc.py (in-process pytest fixture variant).

    def test_flattener_output_avoids_triple_newlines(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define NEED_CORE
                    #define NEED_IO
                    #include "templates/Base.hpp"
                    auto main() -> int { return 0; }
                    """
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "safe"},
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            self.assertNotIn("\n\n\n", result.stdout)

    def test_flattener_mode_safe_keeps_unreferenced_need_headers(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define NEED_IO
                    #include "templates/Base.hpp"
                    auto main() -> int { return 0; }
                    """
                ),
                encoding="utf-8",
            )

            compact = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "compact"},
            )
            safe = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "safe"},
            )

            self.assertEqual(compact.returncode, 0, msg=compact.stderr)
            self.assertEqual(safe.returncode, 0, msg=safe.stderr)
            self.assertNotIn("#define OUT(", compact.stdout)
            self.assertIn("#define OUT(", safe.stdout)

    def test_flattener_mode_auto_fallbacks_to_safe_on_compact_failure(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define NEED_IO
                    #include "templates/Base.hpp"
                    auto main() -> int { return 0; }
                    """
                ),
                encoding="utf-8",
            )

            fake_compiler = Path(tmpdir) / "fake_compiler.sh"
            fake_compiler.write_text(
                textwrap.dedent(
                    """\
                    #!/bin/sh
                    grep -q "#define OUT(" && exit 0
                    exit 1
                    """
                ),
                encoding="utf-8",
            )
            fake_compiler.chmod(0o755)

            result = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={
                    **os.environ,
                    "CP_FLATTENER_MODE": "auto",
                    "CP_FLATTENER_VALIDATION_COMPILER": str(fake_compiler),
                },
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            self.assertIn("#define OUT(", result.stdout)

    def test_nested_template_section_header_has_blank_line_separator(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define NEED_CORE
                    #define NEED_MATH
                    #include "templates/Base.hpp"
                    auto main() -> int { return 0; }
                    """
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "safe"},
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            banner = (
                "//===----------------------------------------------------------------------===//"
            )
            # Every nested section banner must be preceded by exactly one blank line.
            for section in (
                "/* Core Concept Vocabulary */",
                "/* Integer Mathematical Utilities */",
            ):
                self.assertIn(f"\n\n{banner}\n{section}", result.stdout)
                self.assertNotIn(f"\n\n\n{banner}\n{section}", result.stdout)

    def test_default_submission_headers_use_bits_without_portable_fallback(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define CP_IO_PROFILE_SIMPLE
                    #include "templates/Base.hpp"
                    auto main() -> int { INT(x); OUT(x); return 0; }
                    """
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "compact"},
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            self.assertIn("#include <bits/stdc++.h>", result.stdout)
            self.assertNotIn("Portable Standard Library Includes", result.stdout)
            self.assertNotIn("PortableStdHeaders.hpp", result.stdout)

    def test_flattener_mode_submission_forces_bits_and_strips_comments(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define CP_USE_BITS_HEADER 0
                    #define CP_IO_PROFILE_SIMPLE
                    #include "templates/Base.hpp"
                    // user comment
                    auto main() -> int {
                      INT(x); /* inline comment */
                      OUT(x);
                      return 0;
                    }
                    """
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "submission"},
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            self.assertIn("#include <bits/stdc++.h>", result.stdout)
            self.assertNotIn("Portable Standard Library Includes", result.stdout)
            self.assertNotIn("user comment", result.stdout)
            self.assertNotIn("inline comment", result.stdout)
            self.assertNotIn("\n\n\n", result.stdout)

    # Removed: test_algorithms_clangd_include_paths_are_configured.
    # The .clangd file no longer carries explicit -I paths for templates/
    # and modules/; include resolution now flows through CMake-generated
    # compile_commands.json, so asserting on .clangd text is obsolete.

    def test_fast_io_internal_define_not_emitted_when_need_fast_io(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define NEED_CORE
                    #define NEED_FAST_IO
                    #include "templates/Base.hpp"
                    auto main() -> int { return 0; }
                    """
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "safe"},
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            self.assertNotIn("#define CP_FAST_IO_NAMESPACE_DEFINED", result.stdout)
            self.assertIn("namespace fast_io {", result.stdout)

    def test_fast_io_macro_state_applies_to_io_header_when_included(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            src = Path(tmpdir) / "probe.cpp"
            src.write_text(
                textwrap.dedent(
                    """\
                    #define NEED_CORE
                    #define NEED_FAST_IO
                    #include "templates/Base.hpp"
                    #include "templates/IO.hpp"
                    auto main() -> int { return 0; }
                    """
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(FLATTENER_SCRIPT), str(src)],
                capture_output=True,
                text=True,
                check=False,
                env={**os.environ, "CP_FLATTENER_MODE": "safe"},
            )

            self.assertEqual(result.returncode, 0, msg=result.stderr)
            self.assertNotIn("#define CP_FAST_IO_NAMESPACE_DEFINED", result.stdout)
            self.assertIn(
                "#define CP_IO_IMPL_READ(...) fast_io::read(__VA_ARGS__)",
                result.stdout,
            )

    def test_collect_transitive_template_deps_skips_elif_after_active_if(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir).resolve()
            templates = root / "templates"
            templates.mkdir()
            (templates / "A.hpp").write_text(
                textwrap.dedent(
                    """\
                    #if 1
                      #include "B.hpp"
                    #elif 1
                      #include "C.hpp"
                    #endif
                    """
                ),
                encoding="utf-8",
            )
            (templates / "B.hpp").write_text("// b\n", encoding="utf-8")
            (templates / "C.hpp").write_text("// c\n", encoding="utf-8")
            deps = collect_transitive_template_deps(root, templates / "A.hpp")
            self.assertEqual([dep.name for dep in deps], ["B.hpp"])

    def test_collect_transitive_template_deps_keeps_elif_inactive_when_if_inactive(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir).resolve()
            templates = root / "templates"
            templates.mkdir()
            (templates / "A.hpp").write_text(
                textwrap.dedent(
                    """\
                    #if 0
                      #include "B.hpp"
                    #elif 1
                      #include "C.hpp"
                    #endif
                    """
                ),
                encoding="utf-8",
            )
            (templates / "B.hpp").write_text("// b\n", encoding="utf-8")
            (templates / "C.hpp").write_text("// c\n", encoding="utf-8")
            deps = collect_transitive_template_deps(root, templates / "A.hpp")
            self.assertEqual([dep.name for dep in deps], [])

    def test_inline_local_header_does_not_inline_elif_after_active_if(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir).resolve()
            templates = root / "templates"
            templates.mkdir()
            (templates / "A.hpp").write_text(
                textwrap.dedent(
                    """\
                    #if 1
                    #include "B.hpp"
                    #elif 1
                    #include "C.hpp"
                    #endif
                    """
                ),
                encoding="utf-8",
            )
            (templates / "B.hpp").write_text("int b = 1;\n", encoding="utf-8")
            (templates / "C.hpp").write_text("int c = 1;\n", encoding="utf-8")
            output = inline_local_header(root, templates / "A.hpp", set())
            self.assertIn("int b = 1;", output)
            self.assertNotIn("int c = 1;", output)
            # The taken ``#if`` arm wins, so the folder removes the dead ``#elif``
            # arm entirely (including its dangling include) rather than leaking it.
            self.assertNotIn('#include "C.hpp"', output)
            self.assertNotIn("#elif", output)

    def test_inline_local_header_keeps_elif_inactive_for_includes_when_if_inactive(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir).resolve()
            templates = root / "templates"
            templates.mkdir()
            (templates / "A.hpp").write_text(
                textwrap.dedent(
                    """\
                    #if 0
                    #include "B.hpp"
                    #elif 1
                    #include "C.hpp"
                    #endif
                    """
                ),
                encoding="utf-8",
            )
            (templates / "B.hpp").write_text("int b = 1;\n", encoding="utf-8")
            (templates / "C.hpp").write_text("int c = 1;\n", encoding="utf-8")
            output = inline_local_header(root, templates / "A.hpp", set())
            self.assertNotIn("int b = 1;", output)
            self.assertNotIn("int c = 1;", output)
            self.assertIn('#include "C.hpp"', output)


if __name__ == "__main__":
    unittest.main()
