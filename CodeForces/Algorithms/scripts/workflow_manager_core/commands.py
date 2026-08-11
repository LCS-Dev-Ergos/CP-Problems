"""Command registry: the one place that says which subcommands exist.

Each entry pairs a name and help string with the three pieces the CLI needs:
how to declare its flags (``parsers``), how to turn the parsed namespace into
a cpp-tools argv (``argbuilders``), and what to run (a handler). Anything that
is not that wiring lives in a sibling module:

- ``parsers``     — argparse declarations, shared across subcommand families
- ``argbuilders`` — pure ``Namespace -> list[str]`` translations
- ``presets``     — the CMake-preset profile table and its two handlers
- ``workflows``   — multi-step recipes (``cycle``, ``doctor``)

The pre-hooks and the ``_cpp_handler`` factory stay here because they are the
dispatch mechanism itself rather than data about any one command.
"""

from __future__ import annotations

import argparse
from collections.abc import Callable, Iterable
from dataclasses import dataclass

from . import argbuilders as ab
from . import parsers as ps
from .orchestration import WorkflowManager, run_step_with_policy
from .presets import handle_preset_build, handle_preset_conf
from .types import WorkflowError
from .utils import find_existing_target_source
from .workflows import handle_cycle as _handle_cycle
from .workflows import handle_doctor as _handle_doctor

ParserConfigurator = Callable[[argparse.ArgumentParser], None]
CommandHandler = Callable[[WorkflowManager, argparse.Namespace], None]
ArgsBuilder = Callable[[argparse.Namespace], list[str]]
PreHook = Callable[[WorkflowManager, argparse.Namespace], bool]


@dataclass(frozen=True, slots=True)
class CommandSpec:
    name: str
    help: str
    handler: CommandHandler
    configure: ParserConfigurator | None = None


# ---------------------------------- Pre-hooks -------------------------------- #


def _skip_new_if_missing(manager: WorkflowManager, ns: argparse.Namespace) -> bool:
    """Skip `cppnew` when `--if-missing` is enabled and target exists."""

    target_file = find_existing_target_source(manager.runner.cwd, ns.name)
    if ns.if_missing and target_file is not None:
        manager.note(f"[workflow] {target_file.name} already exists, skipping cppnew")
        return True
    return False


def _require_deepclean_yes(_: WorkflowManager, ns: argparse.Namespace) -> bool:
    """Validate explicit confirmation for destructive deepclean."""

    if not ns.yes:
        raise WorkflowError("deepclean is destructive; pass --yes to confirm")
    return False


# --------------------------------- Dispatch ---------------------------------- #


def _cpp_handler(
    function: str,
    *,
    args_builder: ArgsBuilder = ab.no_args,
    pre_hook: PreHook | None = None,
    auto_confirm_deepclean: bool = False,
) -> CommandHandler:
    """Create a command handler delegating to one cpp-tools function."""

    def _handler(manager: WorkflowManager, ns: argparse.Namespace) -> None:
        """Execute optional pre-hook then run wrapped cpp-tools function."""
        if pre_hook is not None and pre_hook(manager, ns):
            return
        run_step_with_policy(
            manager,
            ns,
            function,
            args_builder(ns),
            auto_confirm_deepclean=auto_confirm_deepclean,
        )

    return _handler


# --------------------------------- Registry ---------------------------------- #


def get_command_specs() -> Iterable[CommandSpec]:
    """Return declarative command definitions for parser and dispatcher wiring."""

    return (
        CommandSpec("init", "run cppinit", _cpp_handler("cppinit")),
        CommandSpec(
            "contest",
            "run cppcontest <dir>",
            _cpp_handler("cppcontest", args_builder=lambda ns: [ns.directory]),
            ps.configure_contest,
        ),
        CommandSpec(
            "new",
            "run cppnew <name> [template]",
            _cpp_handler("cppnew", args_builder=ab.new_args, pre_hook=_skip_new_if_missing),
            ps.configure_new,
        ),
        CommandSpec(
            "batch",
            "run cppbatch",
            _cpp_handler("cppbatch", args_builder=ab.batch_args),
            ps.configure_batch,
        ),
        CommandSpec(
            "delete",
            "run cppdelete for one or more problems",
            _cpp_handler("cppdelete", args_builder=ab.delete_args),
            ps.configure_delete,
        ),
        CommandSpec(
            "conf",
            "run cppconf",
            _cpp_handler("cppconf", args_builder=ab.conf_args),
            ps.configure_conf,
        ),
        CommandSpec(
            "preset-conf",
            "configure with centralized CMake presets (cmake --preset ...)",
            handle_preset_conf,
            ps.configure_preset_conf,
        ),
        CommandSpec(
            "preset-build",
            "build with centralized CMake build presets (cmake --build --preset ...)",
            handle_preset_build,
            ps.configure_preset_build,
        ),
        CommandSpec(
            "build",
            "run cppbuild",
            _cpp_handler("cppbuild", args_builder=ab.build_args),
            ps.configure_target_optional,
        ),
        CommandSpec(
            "run",
            "run cpprun",
            _cpp_handler("cpprun", args_builder=ab.run_args),
            ps.configure_target_optional,
        ),
        CommandSpec(
            "go",
            "run cppgo",
            _cpp_handler("cppgo", args_builder=ab.go_args),
            ps.configure_go,
        ),
        CommandSpec(
            "forcego",
            "run cppforcego",
            _cpp_handler("cppgo", args_builder=ab.forcego_args),
            ps.configure_target_optional,
        ),
        CommandSpec(
            "judge",
            "run cppjudge",
            _cpp_handler("cppjudge", args_builder=ab.target_args),
            ps.configure_target_optional,
        ),
        CommandSpec(
            "test",
            "alias of judge for compatibility",
            _cpp_handler("cppjudge", args_builder=ab.target_args),
            ps.configure_target_optional,
        ),
        CommandSpec(
            "stress",
            "run cppstress",
            _cpp_handler("cppstress", args_builder=ab.stress_args),
            ps.configure_stress,
        ),
        CommandSpec(
            "submit",
            "run cppsubmit",
            _cpp_handler("cppsubmit", args_builder=ab.submit_args),
            ps.configure_submit,
        ),
        CommandSpec(
            "test-submit",
            "run cpptestsubmit",
            _cpp_handler("cpptestsubmit", args_builder=ab.test_submit_args),
            ps.configure_test_submit,
        ),
        CommandSpec(
            "full",
            "run cppfull",
            _cpp_handler("cppfull", args_builder=ab.full_args),
            ps.configure_full,
        ),
        CommandSpec("check", "run cppcheck", _cpp_handler("cppcheck")),
        CommandSpec("info", "run cppinfo", _cpp_handler("cppinfo")),
        CommandSpec("diag", "run cppdiag", _cpp_handler("cppdiag")),
        CommandSpec("help", "run cpphelp", _cpp_handler("cpphelp")),
        CommandSpec("clean", "run cppclean", _cpp_handler("cppclean")),
        CommandSpec(
            "deepclean",
            "run cppdeepclean (requires --yes)",
            _cpp_handler(
                "cppdeepclean",
                pre_hook=_require_deepclean_yes,
                auto_confirm_deepclean=True,
            ),
            ps.configure_deepclean,
        ),
        CommandSpec("stats", "run cppstats", _cpp_handler("cppstats")),
        CommandSpec("archive", "run cpparchive", _cpp_handler("cpparchive")),
        CommandSpec(
            "watch",
            "run cppwatch",
            _cpp_handler("cppwatch", args_builder=ab.target_args),
            ps.configure_target_optional,
        ),
        CommandSpec(
            "doctor",
            "run manager + cpp-tools diagnostics",
            _handle_doctor,
            ps.configure_doctor,
        ),
        CommandSpec(
            "cycle",
            "orchestrate a standard CP loop (new/go/judge/submit/test-submit)",
            _handle_cycle,
            ps.configure_cycle,
        ),
    )


def register_subcommands(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    """Attach all command specs to argparse subparsers."""

    for spec in get_command_specs():
        parser = subparsers.add_parser(spec.name, help=spec.help)
        if spec.configure is not None:
            spec.configure(parser)
        parser.set_defaults(handler=spec.handler)
