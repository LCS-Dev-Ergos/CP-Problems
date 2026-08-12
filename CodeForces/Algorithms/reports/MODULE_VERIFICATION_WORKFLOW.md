# Module Verification Workflow

Date: 2026-05-24

This document defines the verification workflow for reusable algorithm modules
under `CodeForces/Algorithms`. It complements the template-level checks in
`scripts/module_tester.py`.

## 1. Two layers of validation

### Template layer

```bash
python3 scripts/module_tester.py
```

Purpose:

- validate `NEED_*` combinations
- catch template integration breakage
- smoke-check the compile surface of the starter stack

### Module layer

```bash
python3 scripts/module_verify.py
```

Or via the Makefile:

```bash
make check
```

Purpose:

- validate concrete reusable modules against their manifests
- compile and optionally run focused probes
- scale one imported module at a time

## 2. Manifest-driven design

Verification cases live under `verify/modules/`, grouped by category:

```text
verify/modules/
  data_structures/
    fenwick/
    segment_tree/
    range_query/
    union_find/
    sequence_structures/
  graph/
  strings/
```

Each module has a JSON manifest plus one or more `.cpp` probes.

## 3. Manifest format

```json
{
  "name": "ModuleName",
  "header": "modules/path/Header.hpp",
  "category": "data_structures",
  "tags": ["smoke"],
  "cases": [
    {
      "name": "runtime",
      "kind": "run",
      "source": "verify/modules/data_structures/ModuleName.runtime.cpp",
      "stdin": "5\n1 2 3 4 5\n",
      "expected_stdout": "15\n",
      "expected_exit_code": 0,
      "timeout_seconds": 5
    },
    {
      "name": "compile",
      "kind": "compile",
      "source": "verify/modules/data_structures/ModuleName.compile.cpp"
    }
  ]
}
```

Supported case fields:

| Field                | Required | Description                                                   |
| -------------------- | -------- | ------------------------------------------------------------- |
| `name`               | Yes      | Case identifier shown in output                               |
| `kind`               | Yes      | `compile` or `run`                                            |
| `source`             | Yes      | Repo-relative path to the `.cpp` probe                        |
| `stdin`              | No       | Input fed to the process on `run` cases                       |
| `expected_stdout`    | No       | Exact stdout expected; failure if mismatched                  |
| `expected_exit_code` | No       | Defaults to `0`                                               |
| `timeout_seconds`    | No       | Kills the process if exceeded (requires `timeout`/`gtimeout`) |

Safety rules:

- manifest filename must match `name`, e.g. `SlidingWindowAggregation.json`
- probe filenames must start with the module name
- `header` and `source` must be repo-relative paths inside `CodeForces/Algorithms`
- manifests are rejected if they try to escape the workspace via `..` or
  absolute paths

## 4. Running verification

**All modules:**

```bash
python3 scripts/module_verify.py
# or
make check
```

**Specific module(s) (repeatable flag):**

```bash
python3 scripts/module_verify.py --module LazySegTree
python3 scripts/module_verify.py --module LazySegTree --module MonoidSegmentTree
# or via Makefile pattern rule
make check-LazySegTree
```

**With JSON report output:**

```bash
python3 scripts/module_verify.py --report reports/test_report.json
# or
make check-report
```

## 5. Output format

A successful run looks like:

```text
==================================================
MODULE VERIFICATION HARNESS
==================================================
LazySegTree :: runtime     PASS 1.885s
LazySegTree :: compile     PASS 1.440s
...
==================================================
SUMMARY: 22/22 verification cases passed
==================================================
```

A failing case shows the mismatch and exits non-zero:

```text
LazySegTree :: runtime     FAIL 0.312s
  expected stdout : "15\n"
  actual stdout   : "14\n"
```

The JSON report (when `--report` is given) records per-case results including
elapsed time, stdout, stderr, and exit code, making it suitable for CI
consumption.

## 6. Why both `compile` and `runtime` cases

`compile` cases:

- validate that the module's public API integrates cleanly
- catch include/dependency regressions
- cheap smoke-check for template-heavy surfaces
- do not require `timeout`

`runtime` cases:

- validate actual behavior on a small but meaningful scenario
- catch logical bugs that a syntax-only check cannot see
- support expected-stdout matching and timeout enforcement

Recommended baseline per module: one `compile` case and one `runtime` case.

## 7. Recommended workflow for new imports

When importing a new module:

1. Adapt the header to local style and local core aliases.
2. Add one runtime probe covering normal behavior.
3. Add one compile-smoke probe for template/API shape.
4. Register both in a manifest under the appropriate `verify/modules/<category>/` path.
5. Run `module_verify.py` filtered to that module.

```bash
make check-NewModuleName
```

## 8. Design goals

- each module's verification is self-describing via its manifest
- both compile-only and runtime checks are supported
- machine-readable JSON reports for CI integration
- future imports (e.g. from Maspy) can be registered without touching any
  central registry file
- compatible with the existing local compiler and template setup
