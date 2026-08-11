"""Structural invariants of the scripts tree, enforced rather than documented.

Two things the audit found were true by accident rather than by construction:
the package graph is acyclic with dependencies pointing consistently downward,
and the zsh allowlist happens to match what the CLI dispatches. Neither had a
check, so either could rot silently.

The layers, from the bottom:

    _lib/                       process + CLI plumbing, no domain knowledge
    flattener_core/             the preprocessor engine, a pure leaf
    profile_registry            the profiles.toml loader
    need_resolver, module_runtime   shared domain libraries
    flattener_pipeline/, workflow_manager_core/   orchestration
    *.py entry points           thin argv wrappers

An import may point down or sideways within a layer, never up.
"""

from __future__ import annotations

import ast
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parents[1]

# Lower number == lower layer. A module may import from its own rank or below.
_LAYER_RANK: dict[str, int] = {
    "_lib": 0,
    "flattener_core": 1,
    "profile_registry": 2,
    "need_resolver": 3,
    "module_runtime": 3,
    "flattener_pipeline": 4,
    "workflow_manager_core": 4,
}


def _internal_modules() -> dict[str, Path]:
    """Map top-level internal module/package names to a representative path."""

    modules: dict[str, Path] = {}
    for path in SCRIPTS_DIR.glob("*.py"):
        modules[path.stem] = path
    for path in SCRIPTS_DIR.iterdir():
        if path.is_dir() and (path / "__init__.py").is_file():
            modules[path.name] = path
    return modules


def _source_files() -> list[Path]:
    """Every checked source file: the tree minus tests, archives, and caches."""

    return [
        path
        for path in SCRIPTS_DIR.rglob("*.py")
        if "__pycache__" not in path.parts
        and "migrations" not in path.parts
        and "tests" not in path.parts
    ]


def _imported_top_level_names(path: Path) -> set[str]:
    """Return the top-level module names ``path`` imports, resolving relatives."""

    tree = ast.parse(path.read_text(encoding="utf-8"))
    package = path.parent.name if path.parent != SCRIPTS_DIR else ""

    names: set[str] = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.ImportFrom):
            if node.level:
                names.add(package)
            elif node.module:
                names.add(node.module.split(".")[0])
        elif isinstance(node, ast.Import):
            for alias in node.names:
                names.add(alias.name.split(".")[0])
    return names


def _module_rank(path: Path) -> int | None:
    """Return the layer rank of the file's owning module, if it has one."""

    owner = path.parent.name if path.parent != SCRIPTS_DIR else path.stem
    return _LAYER_RANK.get(owner)


def test_imports_never_point_up_a_layer():
    """An engine module must not reach back into its orchestrator."""

    internal = set(_internal_modules())
    violations: list[str] = []

    for path in _source_files():
        rank = _module_rank(path)
        if rank is None:
            continue  # entry points sit above everything and may import freely
        for name in _imported_top_level_names(path):
            if name not in internal:
                continue
            target_rank = _LAYER_RANK.get(name)
            if target_rank is not None and target_rank > rank:
                violations.append(
                    f"{path.relative_to(SCRIPTS_DIR)} (layer {rank}) imports "
                    f"{name} (layer {target_rank})"
                )

    assert violations == [], "upward imports break the engine/orchestration split:\n" + "\n".join(
        violations
    )


def test_flattener_core_is_a_pure_leaf():
    """The preprocessor engine must not depend on any other internal module."""

    allowed = {"flattener_core"}
    internal = set(_internal_modules())
    violations: list[str] = []

    for path in (SCRIPTS_DIR / "flattener_core").glob("*.py"):
        for name in _imported_top_level_names(path):
            if name in internal and name not in allowed:
                violations.append(f"{path.name} imports {name}")

    assert violations == [], "flattener_core must stay dependency-free:\n" + "\n".join(violations)


def test_lib_carries_no_domain_knowledge():
    """``_lib`` is plumbing: it must not know about the flattener or workflows."""

    internal = set(_internal_modules())
    violations: list[str] = []

    for path in (SCRIPTS_DIR / "_lib").glob("*.py"):
        for name in _imported_top_level_names(path):
            if name in internal and name != "_lib":
                violations.append(f"_lib/{path.name} imports {name}")

    assert violations == [], "_lib must stay domain-free:\n" + "\n".join(violations)


def test_package_graph_has_no_cycles():
    """A cycle would make the layering unenforceable and imports order-dependent."""

    internal = set(_internal_modules())
    graph: dict[str, set[str]] = {name: set() for name in internal}

    for path in _source_files():
        owner = path.parent.name if path.parent != SCRIPTS_DIR else path.stem
        if owner not in graph:
            continue
        graph[owner] |= {n for n in _imported_top_level_names(path) if n in internal} - {owner}

    state: dict[str, int] = dict.fromkeys(graph, 0)
    cycles: list[str] = []

    def visit(node: str, stack: list[str]) -> None:
        state[node] = 1
        stack.append(node)
        for neighbor in sorted(graph[node]):
            if state[neighbor] == 1:
                cycles.append(" -> ".join([*stack[stack.index(neighbor) :], neighbor]))
            elif state[neighbor] == 0:
                visit(neighbor, stack)
        stack.pop()
        state[node] = 2

    for node in sorted(graph):
        if state[node] == 0:
            visit(node, [])

    assert cycles == [], "import cycles detected:\n" + "\n".join(cycles)
