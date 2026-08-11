#!/usr/bin/env python3
"""Top-level CLI entry point for the source flattener.

This script exists only so that ``python3 scripts/flattener.py ...`` keeps
working for external callers (cpp-tools wrappers, IDE tasks, the round
build scripts). All real logic lives in ``flattener_pipeline.cli:main``.
"""

from __future__ import annotations

from _lib.cli import run_cli
from flattener_pipeline.cli import main

if __name__ == "__main__":
    raise SystemExit(run_cli(main))
