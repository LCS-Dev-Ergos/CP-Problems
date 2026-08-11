# Contributing to `Algorithms/scripts/`

This directory hosts the Python tooling that wraps the centralized CP build
(flattener, workflow manager, module tester/verifier, generators). The code
here is **not** packaged or pip-installed — scripts run directly via
`python3 scripts/<name>.py` or, for top-level workflows, through the
[`Makefile`](./Makefile).

## Required toolchain

- **Python ≥ 3.11** (uses `tomllib`, structural pattern matching, PEP 604
  unions, `StrEnum`, `typing.Self`).
- **ruff** and **mypy**, pinned in [`requirements-dev.txt`](./requirements-dev.txt):

  ```sh
  pip install -r requirements-dev.txt
  ```

  The ruff bound is mirrored by `required-version` in `pyproject.toml`, so a
  mismatched version fails fast instead of reporting findings for rules the
  ignore-list predates. Bump both together, in a commit that re-runs
  `make check`.
- **zsh** + the user's local `cpp-tools` install for the workflow manager
  runtime path (`competitive.sh`).

## CI gate

```sh
make check          # Runs lint + typecheck + test
```

Each of these is also available as a standalone target. Any of them failing
must block the PR.

## Coding conventions

- **Type annotations everywhere** that mypy can verify in strict mode.
  Use `from __future__ import annotations` to keep forward references cheap.
- **Dataclasses with `frozen=True, slots=True`** for value objects; default
  to immutable.
- **`enum.StrEnum`** for closed sets of CLI values (`BuildType`, `Compiler`,
  preset names, …). Never hand-roll `("foo", "bar")` tuples for argparse
  choices.
- **`@functools.cache` / `@functools.cached_property`** for values that are
  expensive once and stable thereafter (e.g. discovered toolchains, git
  toplevel lookups).
- **Subprocess calls go through `scripts/_lib/process.run_capture`**.
  Direct `subprocess.run` is forbidden outside `_lib/` (enforced by a
  `grep`-style test).
- **Entry points wrap `main` in `scripts/_lib/cli.run_cli`**, which turns
  `OSError`/`ValueError` into one stderr line plus exit 1, and leaves anything
  else with its traceback. Never catch bare `Exception` in a `__main__` block:
  a defect should look like a defect.
- **Path safety**: every path coming from a manifest or CLI argument must be
  resolved and `Path.relative_to`-checked against the expected root.

## Layering

Imports point down or sideways, never up. `tests/test_layering.py` enforces
this, so a violation fails `make check` rather than being caught in review:

```
_lib/                                    process + CLI plumbing, no domain knowledge
flattener_core/                          preprocessor engine, a pure leaf
profile_registry                         the profiles.toml loader
need_resolver · module_runtime           shared domain libraries
flattener_pipeline/ · workflow_manager_core/   orchestration
*.py at the top level                    thin argv wrappers
```

Two related invariants have their own tests: `flattener_core` imports nothing
internal at all, and `_lib` stays free of domain knowledge. The top-level
libraries (`profile_registry`, `need_resolver`, `module_runtime`) sit next to
the entry points for import-path reasons — scripts run as
`python3 scripts/<name>.py` — not because they are scripts. None of them has a
`__main__`.

## Workflow

1. `make lint` early and often.
2. New CLI flag or behavior? Add a focused unittest in `tests/` —
   `test_scripts_regressions.py` is the canonical home for cross-script
   regressions.
3. Before pushing: `make check`. If `make typecheck` complains about
   stdlib stubs, document the `# type: ignore[<code>]` reason inline.

## File map

See [reports/Audits/2026-05-26_Scripts_Audit.md](../reports/Audits/2026-05-26_Scripts_Audit.md)
§5 for an up-to-date inventory.
