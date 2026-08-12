"""Runtime regressions for safety-critical template fixes.

The normal template probes are syntax checks. These cases deliberately execute
the code paths whose old implementations compiled successfully but returned a
wrong result, invalidated a container invariant, or consumed only part of an
input token.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import textwrap
from pathlib import Path

import pytest

ALGORITHMS_DIR = Path(__file__).resolve().parents[2]
COMPILER = next(
    (
        resolved
        for name in ("g++-16", "g++-15", "g++-14", "g++-13", "g++")
        if (resolved := shutil.which(name))
    ),
    None,
)


@pytest.mark.skipif(COMPILER is None, reason="a GCC-class compiler is required")
def test_safety_critical_template_regressions(tmp_path: Path) -> None:
    """Execute the formerly silent safety and correctness regressions."""

    source = tmp_path / "template_regressions.cpp"
    binary = tmp_path / "template_regressions"
    source.write_text(
        textwrap.dedent(
            """\
            #define CP_USE_ADVANCED 1
            #define NEED_BIT_OPS
            #define NEED_CONTAINERS
            #define NEED_HASHING
            #define NEED_PBDS
            #define NEED_RANGE_REDUCERS
            #define CP_IO_PROFILE_FAST_EXTENDED
            #include "templates/Base.hpp"

            struct MeterTag;
            using Meter = cp::StrongType<I32, MeterTag>;
            CP_DECLARE_STRONG_TYPE(Distance, I32);
            static_assert(std::same_as<DistanceTag, Distance_tag>);

            template <class T>
            concept RvalueReferenceCast = requires(T value) {
              static_cast<const typename T::value_type&>(std::move(value));
            };
            static_assert(!RvalueReferenceCast<Meter>);

            int main() {
              Barrett barrett(4'294'967'295U);
              constexpr U32 a = 3'423'590'172U;
              constexpr U32 b = 1'889'393'798U;
              if (barrett.mul(a, b) != U32((U64(a) * b) % barrett.mod()))
                return 1;

              cp::hashing::FastHashSet<I64> values;
              values.insert(42);
              cp::hashing::set_seed(123456789);
              if (!values.contains(42))
                return 2;

              static_assert(std::same_as<ordered_multiset<I32>, OrderedMultiSet<I32>>);
              OrderedMultiSet<I32> multiset;
              multiset.insert(5);
              multiset.insert(5);
              multiset.insert(7);
              if (multiset.count(5) != 2 || multiset.order_of_key(7) != 2)
                return 3;
              if (!multiset.erase_one(5) || multiset.count(5) != 1)
                return 4;

              Vec<bool> bits = {true, true, false};
              if (SUM(bits) != 2)
                return 5;

              if (div_floor<I64>(7, -3) != -3 || div_ceil<I64>(7, -3) != -2)
                return 7;
              const auto [quotient, remainder] = divmod<I32>(Limits<I32>::max(), -2);
              if (quotient != -1'073'741'824 || remainder != -1)
                return 8;

              if (bit_ceil<I32>(7) != 8 || bit_floor<I32>(7) != 4)
                return 9;
              I32 bit_index_sum = 0;
              for (I32 index : BitRange<I32>(0b10110))
                bit_index_sum += index;
              if (bit_index_sum != 7)
                return 10;
              I32 subset_count = 0;
              for ([[maybe_unused]] I32 subset : SubsetRange<I32>(0b111))
                ++subset_count;
              if (subset_count != 8)
                return 11;

              const VecI32 source_values = {10, 20, 30};
              if (rearrange(source_values, VecI32{2, 0}) != VecI32({30, 10}))
                return 12;
              if (cumsum(VecI32{1, 2, 3}) != VecI32({0, 1, 3, 6}))
                return 13;
              if (cp::cast::try_narrow<I32>(2'147'483'648.0).has_value())
                return 14;

              F64 floating_value;
              I32 tail;
              fast_io::read(floating_value);
              fast_io::read(tail);
              if (!(floating_value > 0) || tail != 7)
                return 6;
              return 0;
            }
            """
        ),
        encoding="utf-8",
    )

    environment = os.environ.copy()
    environment["TMPDIR"] = str(tmp_path)
    compiled = subprocess.run(
        [
            COMPILER,
            "-std=c++23",
            "-O2",
            "-I",
            str(ALGORITHMS_DIR),
            str(source),
            "-o",
            str(binary),
        ],
        text=True,
        capture_output=True,
        timeout=60,
        check=False,
        env=environment,
    )
    assert compiled.returncode == 0, compiled.stderr

    long_float = "0." + ("0" * 100) + "1"
    executed = subprocess.run(
        [str(binary)],
        input=f"{long_float} 7\n",
        text=True,
        capture_output=True,
        timeout=30,
        check=False,
        env=environment,
    )
    assert executed.returncode == 0, executed.stderr


@pytest.mark.skipif(COMPILER is None, reason="a GCC-class compiler is required")
@pytest.mark.parametrize("variant", (0, 1))
def test_fast_io_integer_parser_handles_boundaries(tmp_path: Path, variant: int) -> None:
    """Parse integer limits when a token crosses the input-buffer boundary."""

    source = tmp_path / "fast_io_integer_boundaries.cpp"
    binary = tmp_path / "fast_io_integer_boundaries"
    source.write_text(
        textwrap.dedent(
            f"""\
            #define CP_FAST_IO_VARIANT {variant}
            #define NEED_FAST_IO
            #include "templates/Base.hpp"

            int main() {{
              I32 i32_min, i32_max;
              U32 u32_max;
              I64 i64_min, i64_max;
              U64 u64_max;
              fast_io::read(i32_max, i32_min, u32_max, i64_min, i64_max, u64_max);
              fast_io::writeln(i32_max, i32_min, u32_max, i64_min, i64_max, u64_max);
            }}
            """
        ),
        encoding="utf-8",
    )
    environment = os.environ.copy()
    environment["TMPDIR"] = str(tmp_path)
    compiled = subprocess.run(
        [
            COMPILER,
            "-std=c++23",
            "-O2",
            "-I",
            str(ALGORITHMS_DIR),
            str(source),
            "-o",
            str(binary),
        ],
        text=True,
        capture_output=True,
        timeout=60,
        check=False,
        env=environment,
    )
    assert compiled.returncode == 0, compiled.stderr

    values = (
        "2147483647 -2147483648 4294967295 -9223372036854775808 "
        "9223372036854775807 18446744073709551615"
    )
    padding = " " * ((1 << 20) - 4)
    executed = subprocess.run(
        [str(binary)],
        input=padding + values + "\n",
        text=True,
        capture_output=True,
        timeout=30,
        check=False,
        env=environment,
    )
    assert executed.returncode == 0, executed.stderr
    assert executed.stdout == values + "\n"

    for invalid in ("2147483648\n", "12x\n", "-\n"):
        rejected = subprocess.run(
            [str(binary)],
            input=invalid,
            text=True,
            capture_output=True,
            timeout=30,
            check=False,
            env=environment,
        )
        assert rejected.returncode != 0


@pytest.mark.skipif(COMPILER is None, reason="a GCC-class compiler is required")
@pytest.mark.parametrize(
    ("defines", "diagnostic"),
    [
        (
            "#define CP_STRICT_TEMPLATE_NEEDS\n#define NEED_IO\n#define NEED_FAST_IO\n",
            "Conflicting template features",
        ),
        ("#define CP_FAST_IO_VARIANT 9\n", "CP_FAST_IO_VARIANT must be"),
    ],
)
def test_invalid_template_configuration_fails_clearly(
    tmp_path: Path, defines: str, diagnostic: str
) -> None:
    """Reject invalid feature combinations with an actionable diagnostic."""

    source = tmp_path / "invalid_config.cpp"
    source.write_text(defines + '#include "templates/Base.hpp"\nint main() {}\n', encoding="utf-8")
    environment = os.environ.copy()
    environment["TMPDIR"] = str(tmp_path)
    compiled = subprocess.run(
        [
            COMPILER,
            "-std=c++23",
            "-I",
            str(ALGORITHMS_DIR),
            str(source),
            "-fsyntax-only",
        ],
        text=True,
        capture_output=True,
        timeout=60,
        check=False,
        env=environment,
    )
    assert compiled.returncode != 0
    assert diagnostic in compiled.stderr
