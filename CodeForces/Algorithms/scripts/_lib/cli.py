"""Shared entry-point wrapper for the standalone scripts.

``workflow_manager`` has always converted its domain errors into a message
plus an exit code; every other script in the tree let them escape as a
traceback. A malformed manifest or an unreadable ``profiles.toml`` is a
configuration problem, not a crash, and printing 20 lines of Python stack for
one is both unhelpful and indistinguishable from a real defect.

:func:`run_cli` is that conversion, factored out so each ``__main__`` block is
one line. It deliberately catches only the error classes this codebase raises
to signal bad input — ``ValueError`` (manifest/registry validation, and
``tomllib.TOMLDecodeError`` which subclasses it) and ``OSError`` (missing or
unreadable files). Anything else is a genuine bug and keeps its traceback.
"""

from __future__ import annotations

import os
import sys
from collections.abc import Callable

__all__ = ["EXIT_ERROR", "EXIT_INTERRUPTED", "EXIT_SIGPIPE", "run_cli"]

EXIT_ERROR = 1
EXIT_SIGPIPE = 141  # 128 + SIGPIPE, the shell convention.
EXIT_INTERRUPTED = 130  # 128 + SIGINT, matching workflow_manager.


def run_cli(
    main_fn: Callable[[], int | None],
    *,
    also_catch: tuple[type[Exception], ...] = (),
) -> int:
    """Run ``main_fn``, turning expected failures into a message + exit code.

    A ``main_fn`` returning ``None`` is treated as success, so entry points
    that simply print their output need no sentinel return.

    ``also_catch`` extends the handled set with a script's own domain error
    (e.g. ``BenchmarkError``) instead of forcing it to inherit from
    ``ValueError`` just to be caught here.

    ``SystemExit`` is intentionally not caught: argparse raises it for
    ``--help`` and for invalid arguments, and swallowing it would break both.
    """

    # Bound to a name first: mypy does not accept an unpacked tuple literal in
    # an ``except`` clause, but it does accept a typed tuple variable.
    handled: tuple[type[Exception], ...] = (OSError, ValueError, *also_catch)

    try:
        return main_fn() or 0
    except KeyboardInterrupt:
        print("Interrupted.", file=sys.stderr)
        return EXIT_INTERRUPTED
    except BrokenPipeError:
        # Piping into `head` closes the reader early. Rebind stdout to devnull
        # so the interpreter's shutdown flush cannot raise a second time and
        # print "Exception ignored" after we have already exited cleanly.
        devnull = os.open(os.devnull, os.O_WRONLY)
        os.dup2(devnull, sys.stdout.fileno())
        return EXIT_SIGPIPE
    except handled as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return EXIT_ERROR
