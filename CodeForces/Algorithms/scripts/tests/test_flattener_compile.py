"""Compile-the-submission regression tests for the flattener.

These guard the class of bug where a profile flattens *and prints* fine yet
the resulting submission does not compile (or silently drops a feature)
because a profile-resolved switch was never emitted into the output. The
``fast_extended`` profile is the canonical victim: its ModInt / StrongType
fast-I/O extensions are gated by ``#if CP_FAST_IO_ENABLE_*`` guards that
default off unless the resolved value is restated in the submission.

Unit-level macro/text assertions cannot catch this — only a real compile of
the flattened output can. The tests are skipped when no GCC-class compiler
that can consume ``<bits/stdc++.h>`` is available (e.g. bare Apple Clang).
"""

from __future__ import annotations

import shutil
import subprocess
import textwrap
from typing import TYPE_CHECKING

import pytest

if TYPE_CHECKING:
    from collections.abc import Callable
    from pathlib import Path

    from .conftest import FlattenResult

_COMPILER_CANDIDATES = ("g++-15", "g++-14", "g++-13", "g++")


def _bits_capable_compiler() -> str | None:
    """Return a compiler path that can compile ``<bits/stdc++.h>``, or ``None``."""

    for candidate in _COMPILER_CANDIDATES:
        resolved = shutil.which(candidate)
        if not resolved:
            continue
        try:
            probe = subprocess.run(
                [resolved, "-std=gnu++20", "-fsyntax-only", "-x", "c++", "-"],
                input="#include <bits/stdc++.h>\nint main(){}\n",
                text=True,
                capture_output=True,
                timeout=30,
                check=False,
            )
        except (OSError, subprocess.TimeoutExpired):
            continue
        if probe.returncode == 0:
            return resolved
    return None


_COMPILER = _bits_capable_compiler()
_needs_compiler = pytest.mark.skipif(
    _COMPILER is None, reason="no GCC-class compiler with <bits/stdc++.h> available"
)


def _compile_submission(source: str) -> subprocess.CompletedProcess[str]:
    """Syntax-check a flattened submission standalone (as an online judge would)."""

    assert _COMPILER is not None
    return subprocess.run(
        [_COMPILER, "-std=gnu++20", "-fsyntax-only", "-x", "c++", "-"],
        input=source,
        text=True,
        capture_output=True,
        timeout=60,
        check=False,
    )


_ADVANCED_MODINT_SOURCE = textwrap.dedent(
    """\
    #ifndef CP_USE_ADVANCED
      #define CP_USE_ADVANCED 1
    #endif
    #define NEED_MACROS
    #define NEED_TYPES
    #define CP_IO_PROFILE_FAST_EXTENDED
    #include "templates/Base.hpp"
    void solve() {
      INT(n);
      MInt acc = 0;
      FOR(n) { MInt x; IN(x); acc = acc + x; }
      OUT(acc);
    }
    auto main() -> int { INT(T); FOR(T) solve(); return 0; }
    """
)


_NDVEC_SOURCE = textwrap.dedent(
    """\
    #define NEED_MACROS
    #define NEED_NDVEC
    #define NEED_TYPES
    #define CP_IO_PROFILE_SIMPLE
    #include "templates/Base.hpp"
    void solve() {
      INT(n, m);
      auto g = make_vec2<I64>(n, m, 0);
      g[0][0] = 7;
      OUT(g[0][0] + (I64)g.size());
    }
    auto main() -> int { INT(T); FOR(T) solve(); return 0; }
    """
)

# Exercises the de-coupled advanced concept paths: Random (rnd), Hashing
# (FastHashMap), Containers (binary_search) must still find their concepts in
# core/IdiomAliases.hpp without reaching into advanced/.
_ADVANCED_DECOUPLE_SOURCE = textwrap.dedent(
    """\
    #ifndef CP_USE_ADVANCED
      #define CP_USE_ADVANCED 1
    #endif
    #define NEED_MACROS
    #define NEED_RANDOM
    #define NEED_HASHING
    #define NEED_CONTAINERS
    #define NEED_TYPES
    #define CP_IO_PROFILE_FAST_EXTENDED
    #include "templates/Base.hpp"
    void solve() {
      INT(n);
      cp::hashing::FastHashMap<I64, I64> mp; mp[3] = 9;
      I64 r = rnd<I64>(1, 1);
      I64 bs = binary_search([&](I64 x) { return x * x <= n; }, 0, n + 1);
      OUT(mp[3] + r + bs);
    }
    auto main() -> int { INT(T); FOR(T) solve(); return 0; }
    """
)


# The advanced layer (cp::StrongType, cp::cast::*) is gated by CP_USE_ADVANCED in
# Base.hpp, not by a NEED_* feature. The flattener must reproduce that include
# gate so these symbols survive even outside fast-I/O (here: legacy
# NEED_TYPE_SAFETY synonym + simple I/O + a direct cp::cast::narrow call).
_ADVANCED_NO_FASTIO_SOURCE = textwrap.dedent(
    """\
    #define NEED_TYPE_SAFETY
    #define NEED_MACROS
    #define NEED_TYPES
    #define CP_IO_PROFILE_SIMPLE
    #include "templates/Base.hpp"
    CP_DECLARE_STRONG_TYPE(Idx, I64);
    void solve() {
      INT(n);
      Idx a{n};
      I64 narrowed = cp::cast::narrow<I32>(a.get());
      OUT((I64)a.get() + narrowed);
    }
    auto main() -> int { INT(T); FOR(T) solve(); return 0; }
    """
)


@_needs_compiler
def test_advanced_layer_without_fastio_submission_compiles(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """CP_USE_ADVANCED must pull the advanced layer even without fast I/O."""

    source = write_source("probe.cpp", _ADVANCED_NO_FASTIO_SOURCE)
    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "submission"})

    assert result.returncode == 0, result.stderr
    assert 'include "templates/' not in result.stdout
    assert 'include "modules/' not in result.stdout

    compiled = _compile_submission(result.stdout)
    assert compiled.returncode == 0, compiled.stderr


@_needs_compiler
def test_ndvec_submission_compiles(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """NEED_NDVEC must inline the nd-vector builders into a compiling submission."""

    source = write_source("probe.cpp", _NDVEC_SOURCE)
    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "submission"})

    assert result.returncode == 0, result.stderr
    assert 'include "templates/' not in result.stdout
    assert 'include "modules/' not in result.stdout
    assert "make_vec2" in result.stdout  # NEED_NDVEC resolves to Containers.hpp

    compiled = _compile_submission(result.stdout)
    assert compiled.returncode == 0, compiled.stderr


@_needs_compiler
def test_advanced_decoupled_modules_submission_compiles(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """Random/Hashing/Containers must compile in advanced mode via IdiomAliases."""

    source = write_source("probe.cpp", _ADVANCED_DECOUPLE_SOURCE)
    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "submission"})

    assert result.returncode == 0, result.stderr
    assert 'include "templates/' not in result.stdout
    assert 'include "modules/' not in result.stdout

    compiled = _compile_submission(result.stdout)
    assert compiled.returncode == 0, compiled.stderr


@_needs_compiler
def test_fast_extended_modint_submission_compiles(
    clean_cp_env: None,
    write_source: Callable[[str, str], Path],
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """A ModInt + fast-I/O submission must compile after flattening."""

    source = write_source("probe.cpp", _ADVANCED_MODINT_SOURCE)
    result = flatten_inproc(source, env={"CP_FLATTENER_MODE": "submission"})

    assert result.returncode == 0, result.stderr
    # No project-local include may leak into a judge submission.
    assert 'include "templates/' not in result.stdout
    assert 'include "modules/' not in result.stdout
    # The flattener must not warn about a surviving project include.
    assert "survived flattening" not in result.stderr

    compiled = _compile_submission(result.stdout)
    assert compiled.returncode == 0, compiled.stderr


@_needs_compiler
@pytest.mark.parametrize("scaffold", ["base", "default", "pbds", "advanced"])
def test_scaffold_submissions_compile(
    scaffold: str,
    clean_cp_env: None,
    algorithms_dir: Path,
    flatten_inproc: Callable[..., FlattenResult],
) -> None:
    """Every committed scaffold must flatten to a compiling submission."""

    scaffold_path = algorithms_dir / "templates" / "cpp" / f"{scaffold}.cpp"
    result = flatten_inproc(scaffold_path, env={"CP_FLATTENER_MODE": "submission"})

    assert result.returncode == 0, result.stderr
    assert 'include "templates/' not in result.stdout
    assert 'include "modules/' not in result.stdout

    compiled = _compile_submission(result.stdout)
    assert compiled.returncode == 0, compiled.stderr
