PYTHON  := python3
ROOT    := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
SCRIPTS := $(ROOT)scripts

# Absolute path to this Algorithms directory, passed as --algorithms-dir to
# workflow_manager round targets.
ALGORITHMS_DIR := $(ROOT:/=)

# Round build targets require the round directory.
# Usage: make preset-conf-debug-gcc ROUND_DIR=/path/to/round
ROUND_DIR ?=

.PHONY: all configure verify headers scaffolds all-probes \
        configure-release configure-sanitize configure-timing \
        release-probes sanitize-probes timing-probes \
        ctest ctest-sanitize \
        check check-report clean help doctor \
        _check-round-dir \
        preset-conf-debug-gcc preset-conf-release-gcc preset-conf-sanitize-gcc \
        preset-conf-debug-clang preset-conf-release-clang preset-conf-sanitize-clang \
        preset-build-debug-gcc preset-build-release-gcc preset-build-sanitize-gcc \
        preset-build-debug-clang preset-build-release-clang preset-build-sanitize-clang

# ── Default ───────────────────────────────────────────────────────────────────
all: configure

# ── Workspace: compile_commands.json + probe families ─────────────────-───────
configure:
	cmake --preset algorithms-workspace-debug

verify:
	cmake --build --preset algorithms-workspace-verify

headers:
	cmake --build --preset algorithms-workspace-headers

scaffolds:
	cmake --build --preset algorithms-workspace-scaffolds

all-probes:
	cmake --build --preset algorithms-workspace-all-probes

# ── Diagnostic / Advanced probe builds ────────────────────────────────────────
configure-release:
	cmake --preset algorithms-workspace-release

configure-sanitize:
	cmake --preset algorithms-workspace-sanitize

configure-timing:
	cmake --preset algorithms-workspace-timing

release-probes: configure-release
	cmake --build --preset algorithms-workspace-release-all

sanitize-probes: configure-sanitize
	cmake --build --preset algorithms-workspace-sanitize-verify

timing-probes: configure-timing
	cmake --build --preset algorithms-workspace-timing-all

# ── CTest: run *.runtime.cpp probes (build them first) ──────────────────-─────
ctest: verify
	ctest --preset algorithms-workspace-tests

ctest-sanitize: sanitize-probes
	ctest --preset algorithms-workspace-tests-sanitize

# ── Module verification ───────────────────────────────────────────────────────
check:
	$(PYTHON) $(SCRIPTS)/module_verify.py

# Pattern rule: make check-<ModuleName>
check-%:
	$(PYTHON) $(SCRIPTS)/module_verify.py --module $*

check-report:
	$(PYTHON) $(SCRIPTS)/module_verify.py --report reports/test_report.json

# ── Diagnostics ───────────────────────────────────────────────────────-───────
doctor:
	$(PYTHON) $(SCRIPTS)/workflow_manager.py doctor

# ── Cleanup ───────────────────────────────────────────────────────────────────
clean:
	rm -rf build/workspace

# ── Round build targets (require ROUND_DIR=<path>) ────────────────────────-───
# These targets delegate to workflow_manager.py with --cwd set to the round
# directory. The Algorithms workspace is passed as --algorithms-dir so the
# centralized presets and toolchain files are resolved correctly.
#
# Example:
#   make preset-conf-debug-gcc ROUND_DIR=../Rounds/Round_1100_Div_1

_check-round-dir:
	@test -n "$(ROUND_DIR)" || \
	  (printf "Error: ROUND_DIR is required for round targets.\n" && \
	   printf "Usage: make <preset-target> ROUND_DIR=<path-to-round>\n" && false)

# Shorthand: $(call wm, <subcommand and args>)
wm = $(PYTHON) $(SCRIPTS)/workflow_manager.py --cwd "$(ROUND_DIR)" $(1)

preset-conf-debug-gcc: _check-round-dir
	$(call wm, preset-conf --preset cp-debug-gcc --algorithms-dir $(ALGORITHMS_DIR))

preset-conf-release-gcc: _check-round-dir
	$(call wm, preset-conf --preset cp-release-gcc --algorithms-dir $(ALGORITHMS_DIR))

preset-conf-sanitize-gcc: _check-round-dir
	$(call wm, preset-conf --preset cp-sanitize-gcc --algorithms-dir $(ALGORITHMS_DIR))

preset-conf-debug-clang: _check-round-dir
	$(call wm, preset-conf --preset cp-debug-clang --algorithms-dir $(ALGORITHMS_DIR))

preset-conf-release-clang: _check-round-dir
	$(call wm, preset-conf --preset cp-release-clang --algorithms-dir $(ALGORITHMS_DIR))

preset-conf-sanitize-clang: _check-round-dir
	$(call wm, preset-conf --preset cp-sanitize-clang --algorithms-dir $(ALGORITHMS_DIR))

preset-build-debug-gcc: _check-round-dir
	$(call wm, preset-build --preset cp-build-debug-gcc)

preset-build-release-gcc: _check-round-dir
	$(call wm, preset-build --preset cp-build-release-gcc)

preset-build-sanitize-gcc: _check-round-dir
	$(call wm, preset-build --preset cp-build-sanitize-gcc)

preset-build-debug-clang: _check-round-dir
	$(call wm, preset-build --preset cp-build-debug-clang)

preset-build-release-clang: _check-round-dir
	$(call wm, preset-build --preset cp-build-release-clang)

preset-build-sanitize-clang: _check-round-dir
	$(call wm, preset-build --preset cp-build-sanitize-clang)

# ── Help ──────────────────────────────────────────────────────────────────────
help:
	@printf "\nWorkspace targets (Debug, default):\n"
	@printf "  make configure              Configure workspace (generates compile_commands.json)\n"
	@printf "  make verify                 Build module verify probes\n"
	@printf "  make headers                Build header probe targets\n"
	@printf "  make scaffolds              Build template scaffold targets\n"
	@printf "  make all-probes             Build all probe families\n"
	@printf "  make check                  Run all module verification cases\n"
	@printf "  make check-<Module>         Run verification for a specific module\n"
	@printf "  make check-report           Run all verifications, write reports/test_report.json\n"
	@printf "  make clean                  Remove workspace build directory\n"
	@printf "\nAdvanced probe builds:\n"
	@printf "  make release-probes         Build all probes with -O2 (catches UB under optimization)\n"
	@printf "  make sanitize-probes        Build verify probes with ASan + UBSan\n"
	@printf "  make timing-probes          Build all probes with -ftime-trace/-ftime-report\n"
	@printf "  make ctest                  Run runtime probes via ctest (after building)\n"
	@printf "  make ctest-sanitize         Run runtime probes under sanitizers\n"
	@printf "\nDiagnostics:\n"
	@printf "  make doctor                 Run cpp-tools environment diagnostics\n"
	@printf "\nRound build targets (pass ROUND_DIR=<path>):\n"
	@printf "  make preset-conf-debug-gcc ROUND_DIR=<path>\n"
	@printf "  make preset-conf-release-gcc ROUND_DIR=<path>\n"
	@printf "  make preset-conf-sanitize-gcc ROUND_DIR=<path>\n"
	@printf "  make preset-conf-debug-clang ROUND_DIR=<path>\n"
	@printf "  make preset-conf-release-clang ROUND_DIR=<path>\n"
	@printf "  make preset-conf-sanitize-clang ROUND_DIR=<path>\n"
	@printf "  make preset-build-debug-gcc ROUND_DIR=<path>\n"
	@printf "  make preset-build-release-gcc ROUND_DIR=<path>\n"
	@printf "  make preset-build-sanitize-gcc ROUND_DIR=<path>\n"
	@printf "  make preset-build-debug-clang ROUND_DIR=<path>\n"
	@printf "  make preset-build-release-clang ROUND_DIR=<path>\n"
	@printf "  make preset-build-sanitize-clang ROUND_DIR=<path>\n"
	@printf "\n  Example:\n"
	@printf "    make preset-conf-debug-gcc ROUND_DIR=../Rounds/Round_1100_Div_1\n\n"
