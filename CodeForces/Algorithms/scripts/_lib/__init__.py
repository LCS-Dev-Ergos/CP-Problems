"""Cross-cutting infrastructure shared by Algorithms/scripts/.

Modules in this package are deliberately *low-level* and dependency-free
within the scripts tree (no flattener, workflow_manager, etc.). They model
the building blocks every script reaches for: subprocess execution, common
result types, and stable error categories.

External users should import from the public re-exports of the consuming
modules (e.g. ``workflow_manager_core``) rather than reaching into ``_lib``
directly — the leading underscore signals an internal package boundary.
"""

from __future__ import annotations

from .cli import EXIT_ERROR, EXIT_INTERRUPTED, EXIT_SIGPIPE, run_cli
from .process import (
    DEFAULT_PROCESS_TIMEOUT_SECONDS,
    ProcessRequest,
    ProcessResult,
    Runnable,
    run_capture,
)

__all__ = [
    "DEFAULT_PROCESS_TIMEOUT_SECONDS",
    "EXIT_ERROR",
    "EXIT_INTERRUPTED",
    "EXIT_SIGPIPE",
    "ProcessRequest",
    "ProcessResult",
    "Runnable",
    "run_capture",
    "run_cli",
]
