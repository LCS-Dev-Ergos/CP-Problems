"""Extract public symbol surfaces from headers for tree-shaking decisions.

``extract_public_symbols`` walks a header at brace-depth 0 and harvests
class/struct/using/function names, deliberately ignoring namespace-internal
helpers and template specializations so the resulting set stays a stable
proxy for "what a downstream file might reference by name".

Macro names are harvested too, and at any brace depth: the preprocessor does
not respect scope, and a header whose entire public surface is macros (a
feature module exposing only ``SUM``/``MIN``/``MAX``, say) would otherwise
look symbol-less and be pruned away from a submission that uses it.

``collect_module_leaf_trigger_tokens`` builds the per-leaf token map used
by the umbrella-module pruner in ``includes.inline_local_header``.
"""

from __future__ import annotations

from pathlib import Path

from flattener_core.lexer import (
    CPP_KEYWORDS,
    FUNC_LIKE_RE,
    STRUCT_CLASS_RE,
    USING_RE,
    strip_non_code,
)
from flattener_core.macros import DEFINE_DIRECTIVE_RE

ModuleLeafTokenMap = dict[str, set[str]]


def extract_public_symbols(header_content: str) -> set[str]:
    """Extract top-level exported symbols from a header.

    Conservative: only declarations at brace depth 0 are considered, so
    namespace-internal helpers and template specializations are skipped.
    """

    content = strip_non_code(header_content)
    symbols: set[str] = set()
    depth = 0

    for raw in content.splitlines():
        line = raw.strip()

        # Macros are depth-independent: `#define` inside a namespace block still
        # exports a name any downstream file can spell.
        define_match = DEFINE_DIRECTIVE_RE.match(line)
        if define_match:
            symbols.add(define_match.group(1))

        if depth == 0 and line:
            match = STRUCT_CLASS_RE.match(line)
            if match:
                symbols.add(match.group(1))

            match = USING_RE.match(line)
            if match:
                symbols.add(match.group(1))

            match = FUNC_LIKE_RE.search(line)
            if match:
                name = match.group(1)
                if name not in CPP_KEYWORDS:
                    symbols.add(name)

        depth += raw.count("{")
        depth -= raw.count("}")
        depth = max(depth, 0)

    return symbols


def collect_module_leaf_trigger_tokens(project_root: Path) -> ModuleLeafTokenMap:
    """Build trigger token map for granular module headers (``modules/<domain>/*.hpp``)."""

    modules_root = project_root / "modules"
    if not modules_root.is_dir():
        return {}

    token_map: ModuleLeafTokenMap = {}
    for domain_dir in modules_root.iterdir():
        if not domain_dir.is_dir():
            continue

        for header in sorted(domain_dir.glob("*.hpp")):
            if header.name == "_Common.hpp":
                continue
            try:
                content = header.read_text(encoding="utf-8")
            except OSError:
                continue

            rel = header.relative_to(project_root).as_posix()
            tokens = extract_public_symbols(content)
            tokens.add(header.stem)
            token_map[rel] = tokens

    return token_map
