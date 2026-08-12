"""Function-style flattener tests that drive the CLI through ``flatten_inproc``.

Each test takes the shared ``write_source`` + ``flatten_inproc`` fixtures
from ``conftest.py``, writes a small ``.cpp`` probe, and asserts on the
flattened output without spawning a separate Python interpreter — real
tracebacks survive failures and the per-test runtime drops sharply.

Acts as the canonical pattern for new flattener tests: prefer this style
over the subprocess scaffolding still present in ``test_flattener.py``.
"""

from __future__ import annotations

import textwrap
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Callable
    from pathlib import Path

    from .conftest import FlattenResult


def test_flattener_accepts_need_macro_with_value(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """``#define NEED_IO 1`` (value form) must enable the IO section."""

    source = write_source(
        "probe.cpp",
        textwrap.dedent(
            """\
            #define NEED_IO 1
            #include "templates/Base.hpp"
            auto main() -> int {
              INT(x);
              OUT(x);
              return 0;
            }
            """
        ),
    )

    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "safe"})

    assert result.returncode == 0, result.stderr
    assert "#define INT(" in result.stdout
    assert "#define OUT(" in result.stdout


def test_flattener_expands_base_include_with_trailing_comment(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """A trailing line comment after ``Base.hpp`` must not prevent inlining."""

    source = write_source(
        "probe.cpp",
        textwrap.dedent(
            """\
            #define NEED_CORE
            #include "templates/Base.hpp" // keep this comment style
            auto main() -> int { return 0; }
            """
        ),
    )

    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "safe"})

    assert result.returncode == 0, result.stderr
    assert '#include "templates/Base.hpp"' not in result.stdout
    assert "<bits/stdc++.h>" in result.stdout


def test_flattener_keeps_reverse_loop_macros_from_need_core(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """``NEED_CORE`` must keep the reverse-loop helper macros in the output."""

    source = write_source(
        "probe.cpp",
        textwrap.dedent(
            """\
            #define NEED_CORE
            #include "templates/Base.hpp"
            auto main() -> int { return 0; }
            """
        ),
    )

    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "safe"})

    assert result.returncode == 0, result.stderr
    assert "#define ROF(" in result.stdout
    assert "#define FORD(" in result.stdout


def test_flattener_fast_minimal_profile_selects_minimal_variant(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """``CP_IO_PROFILE_FAST_MINIMAL`` must resolve Fast_IO to the variant-0 body."""

    source = write_source(
        "probe.cpp",
        textwrap.dedent(
            """\
            #define CP_IO_PROFILE_FAST_MINIMAL
            #include "templates/Base.hpp"
            auto main() -> int { INT(x); OUT(x); return 0; }
            """
        ),
    )

    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "safe"})

    assert result.returncode == 0, result.stderr
    assert "std::fread(input_buffer, 1, BUFFER_SIZE, stdin)" in result.stdout
    assert "std::memmove" not in result.stdout
    assert "input_eof" not in result.stdout
    assert "input_eof" not in result.stdout
    assert "#if CP_FAST_IO_VARIANT" not in result.stdout
    # Fast I/O dispatch binds to fast_io::read.
    assert "fast_io::read" in result.stdout


def test_flattener_fast_extended_profile_does_not_set_minimal_variant(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """``CP_IO_PROFILE_FAST_EXTENDED`` must resolve Fast_IO to the variant-1 body."""

    source = write_source(
        "probe.cpp",
        textwrap.dedent(
            """\
            #define CP_IO_PROFILE_FAST_EXTENDED
            #include "templates/Base.hpp"
            auto main() -> int { INT(x); OUT(x); return 0; }
            """
        ),
    )

    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "safe"})

    assert result.returncode == 0, result.stderr
    # Variant 1 keeps the refill sentinel and drops the full-reload branch.
    assert "input_eof" in result.stdout
    assert "Full reload" not in result.stdout
    # The profile declares NEED_MOD_INT; Fast I/O discovers its protocol through
    # concepts rather than a second extension toggle.
    assert "ModInt" in result.stdout


def test_flattener_collision_fast_io_wins_over_simple(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """When both NEED_IO and NEED_FAST_IO are set, the fast backend wins."""

    source = write_source(
        "probe.cpp",
        textwrap.dedent(
            """\
            #define NEED_IO
            #define NEED_FAST_IO
            #include "templates/Base.hpp"
            auto main() -> int { return 0; }
            """
        ),
    )

    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "safe"})

    assert result.returncode == 0, result.stderr
    # cp_io must not be emitted alongside the fast backend.
    assert "namespace cp_io {" not in result.stdout
    assert "namespace fast_io {" in result.stdout


def test_flattener_rejects_unknown_need_macro(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """A misspelled template feature must fail at the architectural boundary."""

    source = write_source(
        "probe.cpp",
        '#define NEED_GRPAH\n#include "templates/Base.hpp"\nint main() {}\n',
    )
    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "safe"})

    assert result.returncode == 1
    assert "Unknown template feature macro" in result.stderr
    assert "NEED_GRPAH" in result.stderr
