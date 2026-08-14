"""Template-header metadata and tree-shaking."""

from __future__ import annotations

import functools
import os
from pathlib import Path

from flattener_core.includes import parse_project_include_line
from flattener_core.lexer import extract_identifiers, strip_non_code
from flattener_core.symbols import extract_public_symbols

TYPES_HPP = "Types.hpp"
SCALAR_TYPES_HPP = "ScalarTypes.hpp"
CONTAINER_ALIASES_HPP = "ContainerAliases.hpp"
TYPE_TRAITS_HPP = "TypeTraits.hpp"
IDIOM_ALIASES_HPP = "IdiomAliases.hpp"
CONSTANTS_HPP = "Constants.hpp"
MACROS_HPP = "Macros.hpp"
INTEGER_MATH_HPP = "IntegerMath.hpp"
MINMAX_HPP = "MinMax.hpp"
RANDOM_HPP = "Random.hpp"
TIMER_HPP = "Timer.hpp"
IO_DEFS_HPP = "IO_Defs.hpp"
IO_HPP = "IO.hpp"
FAST_IO_HPP = "Fast_IO.hpp"
PBDS_HPP = "PBDS.hpp"
BIT_OPS_HPP = "Bit_Ops.hpp"
CONTAINERS_HPP = "Containers.hpp"
NDVEC_HPP = "NdVec.hpp"
SEARCH_HPP = "Search.hpp"
CONTAINER_ALGORITHMS_HPP = "ContainerAlgorithms.hpp"
MOD_INT_HPP = "Mod_Int.hpp"
CONCEPTS_HPP = "Concepts.hpp"
CORE_CONCEPTS_HPP = "CoreConcepts.hpp"
RANGE_STREAM_CONCEPTS_HPP = "RangeStreamConcepts.hpp"
CAST_HPP = "Cast.hpp"
STRONG_TYPE_HPP = "Strong_Type.hpp"
HASHING_HPP = "Hashing.hpp"
DEBUG_HPP = "Debug.hpp"

# fmt: off
OPTIONAL_HEADER_TRIGGER_TOKENS: dict[str, set[str]] = {
    SCALAR_TYPES_HPP: {
        "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "I128", "U128",
        "F32", "F64", "F80", "F128", "Size", "Diff", "Byte", "Limits",
        "HAS_INT128", "HAS_FLOAT128",
    },
    CONTAINER_ALIASES_HPP: {
        "String", "StringView", "Vec", "Vec2D", "Vec3D", "Vec4D",
        "Deque", "List", "Set", "MultiSet", "UnorderedSet", "Map", "MultiMap",
        "UnorderedMap", "Stack", "Queue", "PriorityQueue", "MinPriorityQueue",
        "Pair", "Tuple", "Optional", "Variant", "Function", "Span",
        "VecI8", "VecI16", "VecI32", "VecI64",  "VecU8", "VecU16",
        "VecU32", "VecU64", "VecF64", "VecF80","VecBool", "VecStr",
        "PairI32", "PairI64", "PairF80", "VecPair", "VecPairI32", "VecPairI64",
    },
    TYPE_TRAITS_HPP: {
        "RemoveCvrefT", "MakeUnsignedT", "remove_cvref_t", "make_unsigned_t",
        "as",
    },
    IDIOM_ALIASES_HPP: {
        "Same", "Int", "Float", "Signed", "Unsigned", "NonBoolInt", "Arithmetic",
        "Conditional", "Enum", "Predicate", "Hashable",
    },
    TYPES_HPP: {
        "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "I128", "U128",
        "F32", "F64", "F80", "F128",
        "Deque", "List", "Set", "MultiSet", "UnorderedSet", "Map", "MultiMap",
        "UnorderedMap", "Stack", "Queue", "PriorityQueue", "MinPriorityQueue",
        "Pair", "Tuple",
        "OrderedSet", "OrderedMultiSet", "OrderedMap", "GPHashTable",
        "ordered_set", "ordered_multiset", "ordered_map", "gp_hash_table",
        "HAS_INT128", "HAS_FLOAT128",
        "PBDS_AVAILABLE",
    },
    CONSTANTS_HPP: {
        "PI", "E", "PHI", "LN2", "EPS", "DEPS", "infinity", "INF32", "INF64",
        "LINF", "MOD", "MOD2",
        "MOD3",
    },
    MACROS_HPP: {
        "FOR", "FOR_R", "ALL", "RALL", "all",
        "rall", "sz", "len", "eb", "elif",
        "UNIQUE", "LB", "UB", "SUM", "MIN",
        "MAX", "fix", "YCombinator", "isz", "sum_range", "ROF", "FORD",
        "sz64", "sz32", "LoopT",
    },
    INTEGER_MATH_HPP: {
        "div_floor", "div_ceil", "mod_floor", "divmod", "power", "mod_pow",
        "floor_sqrt", "ceil_sqrt",
    },
    MINMAX_HPP: {
        "chmax", "chmin", "_min", "_max",
    },
    RANDOM_HPP: {
        "default_rng_seed", "rng", "reseed", "rnd",
    },
    TIMER_HPP: {
        "Stopwatch",
    },
    IO_HPP: {
        "fast_io", "IN", "OUT", "FLUSH", "INT", "LL", "ULL", "STR", "CHR",
        "DBL", "VEC", "VV", "YES", "NO", "Yes", "No",
    },
    FAST_IO_HPP: {
        "fast_io", "load_input", "read_integer", "read_char", "read_string",
        "write_integer", "write_char", "write_string", "flush_output", "IOFlusher",
        "IN", "OUT", "FLUSH", "INT", "LL", "ULL", "STR", "CHR", "DBL", "VEC", "VV",
        "YES", "NO", "Yes", "No",
    },
    PBDS_HPP: {
        "OrderedSet", "OrderedMultiSet", "OrderedMap", "GPHashTable",
        "ordered_set", "ordered_multiset", "ordered_map", "gp_hash_table",
        "tree_order_statistics_node_update", "find_by_order", "order_of_key",
    },
    BIT_OPS_HPP: {
        "popcount", "leading_zeros", "trailing_zeros", "bit_width", "bit_floor",
        "bit_ceil", "parity_sign", "kth_bit", "has_kth_bit", "bit_range",
        "subset_range",
    },
    CONTAINERS_HPP: {
        "binary_search", "binary_search_real", "argsort", "rearrange", "cumsum",
        "concat", "sum_as", "string_to_ints", "pop_val",
        "make_vec2", "make_vec3", "make_vec4",
    },
    NDVEC_HPP: {"make_vec2", "make_vec3", "make_vec4"},
    SEARCH_HPP: {"binary_search", "binary_search_real"},
    CONTAINER_ALGORITHMS_HPP: {
        "argsort", "rearrange", "cumsum", "concat", "sum_as", "string_to_ints", "pop_val",
    },
    MOD_INT_HPP: {
        "ModInt", "DynModInt", "MInt", "MInt2", "DMInt",
    },
    CONCEPTS_HPP: {
        "IndexLike", "Range", "SizedRange", "StreamReadable", "StreamWritable",
    },
    CORE_CONCEPTS_HPP: {
        "Same", "Int", "Float", "Signed", "Unsigned", "NonBoolInt", "Arithmetic",
        "Conditional", "Enum", "Predicate", "Hashable",
    },
    RANGE_STREAM_CONCEPTS_HPP: {
        "IndexLike", "Range", "SizedRange", "StreamReadable", "StreamWritable",
    },
    CAST_HPP: {
        "to_underlying", "enum_cast", "narrow", "try_narrow", "saturate", "narrow_as",
    },
    STRONG_TYPE_HPP: {
        "StrongType", "unwrap", "make_strong", "strong", "StrongInt",
        "CP_DECLARE_STRONG_TYPE",
    },
    HASHING_HPP: {
        "splitmix64", "SplitMixHash", "PairHash", "FastHashMap", "FastHashSet",
        "FastHashMap2", "hash_combine", "raw_hash",
    },
    DEBUG_HPP: {
        "debug", "debug_if", "debug_tree", "debug_tree_verbose",
        "debug_line", "my_assert", "COUNT_CALLS",
    },
}
# fmt: on

HEADER_DEPENDENCIES: dict[str, set[str]] = {}

MODULE_SECTION_SEPARATOR = (
    "//=====----- [ Modules ] ------------------------------------------------=====//\n"
)


def _templates_dir() -> Path:
    """Return the centralized templates directory (sibling of scripts parent)."""

    return Path(__file__).resolve().parents[2] / "templates"


def _augment_tables_from_headers() -> None:
    """Union the hand-maintained tables with symbols/deps extracted from real headers."""

    templates_dir = _templates_dir()
    if not templates_dir.is_dir():
        return

    for header_path in templates_dir.rglob("*.hpp"):
        name = header_path.name
        try:
            content = header_path.read_text(encoding="utf-8")
        except OSError:
            continue

        symbols = extract_public_symbols(content)
        if symbols:
            OPTIONAL_HEADER_TRIGGER_TOKENS.setdefault(name, set()).update(symbols)

        deps: set[str] = set()
        masked_lines = strip_non_code(content).splitlines()
        for idx, line in enumerate(content.splitlines()):
            masked = masked_lines[idx] if idx < len(masked_lines) else ""
            include_name = parse_project_include_line(line, masked_line=masked)
            if not include_name:
                continue
            base = Path(include_name).name
            if base.endswith(".hpp") and base != name:
                deps.add(base)
        if deps:
            HEADER_DEPENDENCIES.setdefault(name, set()).update(deps)


@functools.cache
def ensure_augmented_tables() -> None:
    """Idempotently merge real-header data into the hand-maintained tables.

    Called lazily on the first table-consumer to avoid the cost of walking
    ``templates/`` for every importer (e.g. ``workflow_manager.py`` start-up).
    ``@functools.cache`` makes this a thread-safe run-once shim.
    """

    _augment_tables_from_headers()


def prune_template_headers(files_to_include: list[Path], source_content: str) -> list[Path]:
    """Conservative tree-shaking for template headers.

    Keeps only headers whose symbols/macros appear in user code, while
    honoring hard dependencies between template headers.
    """

    return prune_template_headers_with_policy(
        files_to_include,
        source_content,
        enable_pruning=True,
    )


def prune_template_headers_with_policy(
    files_to_include: list[Path],
    source_content: str,
    *,
    enable_pruning: bool,
) -> list[Path]:
    """Tree-shaking for template headers with explicit policy control."""

    if not enable_pruning or os.environ.get("CP_FLATTENER_DISABLE_PRUNING", "") == "1":
        return files_to_include

    ensure_augmented_tables()

    used_identifiers = extract_identifiers(source_content)
    file_lookup = {path.name: path for path in files_to_include}
    selected_names = set(file_lookup.keys())

    for header, trigger_tokens in OPTIONAL_HEADER_TRIGGER_TOKENS.items():
        if header in selected_names and not (used_identifiers & trigger_tokens):
            selected_names.remove(header)

    changed = True
    while changed:
        changed = False
        for header in tuple(selected_names):
            for dep in HEADER_DEPENDENCIES.get(header, ()):
                if dep in file_lookup and dep not in selected_names:
                    selected_names.add(dep)
                    changed = True

    return [path for path in files_to_include if path.name in selected_names]
