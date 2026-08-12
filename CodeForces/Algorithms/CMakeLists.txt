# ============================================================================ #
# -------------------- CP ALGORITHMS BUILD CONFIGURATION --------------------- #
# ============================================================================ #
#
# Description:
#   Workspace-level build configuration for the CP Algorithms library.
#   Generates compile_commands.json consumed by clangd and other language
#   servers, providing accurate IDE diagnostics across all public headers,
#   module verification sources, and template scaffolds.
#
#   This file is NOT used for contest round builds. Those are driven by
#   cmake/CPRoundBootstrap.cmake, included from each round's CMakeLists.txt.
#
# Probe Families (all EXCLUDE_FROM_ALL — nothing built by default):
#   Header Probes   - One translation unit per public header; ensures clangd
#                     can resolve every include path and symbol.
#   Verify Probes   - Builds verify/modules/*.cpp smoke tests that exercise
#                     module APIs without external test frameworks.
#   Scaffold Probes - Registers templates/cpp/*.cpp contest template sources
#                     so the language server indexes them.
#
# Aggregate Targets:
#   cp_algorithms_verify_probes      - All module verification probes.
#   cp_algorithms_header_probes      - All header probe targets.
#   cp_algorithms_template_scaffolds - All template scaffold targets.
#   cp_algorithms_all_probes         - All probe families at once.
#
# Options:
#   CP_ALGORITHMS_ENABLE_VERIFY_PROBES      (default: ON)
#   CP_ALGORITHMS_ENABLE_HEADER_PROBES      (default: ON)
#   CP_ALGORITHMS_ENABLE_TEMPLATE_SCAFFOLDS (default: ON)
#   CP_ALGORITHMS_ENABLE_PROBE_PCH          (default: ON)
#   CP_ALGORITHMS_ENABLE_CCACHE             (default: ON)
#   CP_ALGORITHMS_ENABLE_TIMING             (default: OFF)
#   CP_ALGORITHMS_ENABLE_PROBE_SANITIZERS   (default: OFF)
#   CP_ALGORITHMS_ENABLE_CTEST              (default: ON)
#   CP_ALGORITHMS_WERROR                    (default: OFF)
#   CP_ALGORITHMS_ENABLE_UNITY_BUILD        (default: ON)
#   CP_ALGORITHMS_ENABLE_FAST_LINKER        (default: ON)
#
# ============================================================================ #

cmake_minimum_required(VERSION 3.20)
project(CPAlgorithmsWorkspace LANGUAGES CXX)

# ----------------------------- Global Settings ------------------------------ #
# compile_commands.json must be ON so clangd can find every translation unit.
# Extensions OFF ensures the standard is enforced without GNU-specific relaxations.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ----------------------------- Feature Options ------------------------------ #
# Each probe family can be disabled independently to speed up configure when
# only a subset is needed (e.g. disable header probes to iterate on verify only).
option(CP_ALGORITHMS_ENABLE_VERIFY_PROBES "Register module verification probes as build targets." ON)
option(CP_ALGORITHMS_ENABLE_HEADER_PROBES "Generate one probe translation unit per public header for clangd coverage." ON)
option(CP_ALGORITHMS_ENABLE_TEMPLATE_SCAFFOLDS "Register template scaffold sources as build targets." ON)
option(CP_ALGORITHMS_ENABLE_PROBE_PCH "Enable shared precompiled headers for workspace probes." ON)
option(CP_ALGORITHMS_ENABLE_CCACHE "Use ccache as compiler launcher if available." ON)
option(CP_ALGORITHMS_ENABLE_TIMING "Enable compilation timing reports for probe targets." OFF)
option(CP_ALGORITHMS_ENABLE_PROBE_SANITIZERS "Compile probes with AddressSanitizer + UBSan (probe PCH disabled if mismatched)." OFF)
option(CP_ALGORITHMS_ENABLE_CTEST "Register runtime probes (*.runtime.cpp) as CTest tests." ON)
option(CP_ALGORITHMS_WERROR "Treat warnings as errors for all probe targets." OFF)
option(CP_ALGORITHMS_ENABLE_UNITY_BUILD "Bundle header probes into a single unity TU for faster compilation." ON)
option(CP_ALGORITHMS_ENABLE_FAST_LINKER "Link runtime probes with lld when available (faster than ld64)." ON)

if(CP_ALGORITHMS_ENABLE_CTEST)
  enable_testing()
endif()

# ----------------------------- Directory Roots ------------------------------ #
# All paths in this file are derived from these three variables so that the
# workspace is fully relocatable.
set(CP_ALGORITHMS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(CP_ALGORITHMS_PROBE_DIR "${CMAKE_CURRENT_BINARY_DIR}/clangd_probes")
set(CP_ALGORITHMS_RUNTIME_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin/probes")

# ---------------------------- ccache (optional) ----------------------------- #
# Re-uses the same probe pattern as cmake/CPRoundConfig.cmake. We skip ccache
# silently when the cache directory is read-only (sandboxed CI, locked $HOME).
if(CP_ALGORITHMS_ENABLE_CCACHE AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
  find_program(CP_ALGORITHMS_CCACHE_PROGRAM ccache)
  if(CP_ALGORITHMS_CCACHE_PROGRAM)
    set(_cp_alg_ccache_dir "$ENV{CCACHE_DIR}")
    if(NOT _cp_alg_ccache_dir AND DEFINED ENV{HOME} AND NOT "$ENV{HOME}" STREQUAL "")
      set(_cp_alg_ccache_dir "$ENV{HOME}/.cache/ccache")
    endif()
    set(_cp_alg_ccache_ok TRUE)
    if(_cp_alg_ccache_dir)
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_cp_alg_ccache_dir}/tmp"
        RESULT_VARIABLE _cp_alg_ccache_mkdir_rc
      )
      if(NOT _cp_alg_ccache_mkdir_rc EQUAL 0)
        set(_cp_alg_ccache_ok FALSE)
      endif()
    endif()
    if(_cp_alg_ccache_ok)
      set(CMAKE_CXX_COMPILER_LAUNCHER "${CP_ALGORITHMS_CCACHE_PROGRAM}")
      message(STATUS "ccache enabled for workspace probes: ${CP_ALGORITHMS_CCACHE_PROGRAM}")
    endif()
    unset(_cp_alg_ccache_dir)
    unset(_cp_alg_ccache_ok)
    unset(_cp_alg_ccache_mkdir_rc)
  endif()
endif()

# ----------------------- Workspace Interface Library ------------------------ #
# All probe executables link against this INTERFACE target so they inherit the
# same include paths and flags that a real contest solution would see.
# The library itself produces no binary; it is purely a compile-settings carrier.
add_library(cp_algorithms_workspace INTERFACE)
target_compile_features(cp_algorithms_workspace INTERFACE cxx_std_23)
target_include_directories(
  cp_algorithms_workspace
  INTERFACE
    "${CP_ALGORITHMS_ROOT}"
    "${CP_ALGORITHMS_ROOT}/templates"
    "${CP_ALGORITHMS_ROOT}/modules"
    "${CP_ALGORITHMS_ROOT}/libs"
    "${CP_ALGORITHMS_ROOT}/verify"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(cp_algorithms_workspace INTERFACE -Wall -Wextra -Wpedantic)
  if(CP_ALGORITHMS_WERROR)
    target_compile_options(cp_algorithms_workspace INTERFACE -Werror)
  endif()
endif()

# Neutralize the contest-focused pragmas baked into templates/core/Compiler.hpp.
# Without this, GCC PCH ends up with `#pragma GCC target(...)` and aggressive
# optimize() directives that conflict with probe TUs (which compile under
# different flags), producing "target specific option mismatch" link errors.
# The workspace is purely diagnostic — it doesn't need contest tuning.
target_compile_definitions(
  cp_algorithms_workspace
  INTERFACE
    CP_ENABLE_AGGRESSIVE_OPTIMIZATIONS=0
    CP_ENABLE_ARCH_TARGET_PRAGMAS=0
)

# ---------------------- Sanitizer Plumbing for Probes ----------------------- #
# Applied via the INTERFACE library so every probe (compile-only OBJECT
# libraries and runtime executables) sees the same flags. This is critical for
# PCH REUSE_FROM, which requires byte-identical command-line flags between
# donor and consumers.
set(CP_ALGORITHMS_PROBE_SANITIZER_LINK_FLAGS "")
if(CP_ALGORITHMS_ENABLE_PROBE_SANITIZERS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(_cp_alg_san_compile -g -O1 -fno-omit-frame-pointer -fno-sanitize-recover=all)
    set(_cp_alg_san_link "")
    list(APPEND _cp_alg_san_compile -fsanitize=address,undefined)
    list(APPEND _cp_alg_san_link    -fsanitize=address,undefined)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT APPLE)
      list(APPEND _cp_alg_san_compile -fsanitize=leak)
      list(APPEND _cp_alg_san_link    -fsanitize=leak)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND NOT APPLE)
      list(APPEND _cp_alg_san_compile -fsanitize=leak)
      list(APPEND _cp_alg_san_link    -fsanitize=leak)
    endif()
    target_compile_options(cp_algorithms_workspace INTERFACE ${_cp_alg_san_compile})
    set(CP_ALGORITHMS_PROBE_SANITIZER_LINK_FLAGS "${_cp_alg_san_link}")
    message(STATUS "Probe sanitizers enabled: ${_cp_alg_san_compile}")
    unset(_cp_alg_san_compile)
    unset(_cp_alg_san_link)
  else()
    message(WARNING "CP_ALGORITHMS_ENABLE_PROBE_SANITIZERS requested but compiler is not Clang/GCC; ignoring.")
  endif()
endif()

# ---------------------- Fast Linker for Runtime Probes ---------------------- #
# Probe runtime executables only — lld typically beats ld64 by 2-3x for small
# C++ binaries. Apple Clang on macOS doesn't always know how to find Homebrew
# lld, so we restrict the substitution to LLVM Clang where lld ships alongside.
set(CP_ALGORITHMS_FAST_LINKER_FLAGS "")
if(CP_ALGORITHMS_ENABLE_FAST_LINKER AND CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  find_program(CP_ALGORITHMS_LLD_PROGRAM
    NAMES ld.lld lld
    PATHS /opt/homebrew/opt/llvm/bin /opt/homebrew/bin /usr/local/opt/llvm/bin /usr/bin
  )
  if(CP_ALGORITHMS_LLD_PROGRAM)
    set(CP_ALGORITHMS_FAST_LINKER_FLAGS "-fuse-ld=lld")
    message(STATUS "Fast linker enabled for runtime probes: ${CP_ALGORITHMS_LLD_PROGRAM}")
  endif()
endif()

# ---------------------- Compilation Timing (optional) ----------------------- #
# Reuses the round-level timing module — its flags (-ftime-report / -ftime-trace)
# are applied via add_compile_options(), which propagates to probe targets too.
if(CP_ALGORITHMS_ENABLE_TIMING)
  if(NOT DEFINED CP_TIMING_CONFIGS)
    set(CP_TIMING_CONFIGS "Debug;Sanitize;Release" CACHE STRING "Configs that receive timing flags")
  endif()
  set(CP_ENABLE_TIMING ON CACHE BOOL "Enable detailed GCC/Clang compilation timing reports" FORCE)
  include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPRoundFeatureTiming.cmake")
endif()

# ----------------------- Shared PCH for Probe Targets ----------------------- #
# A single donor target compiles the workspace PCH once; every probe reuses it.
# The PCH header lives at libs/PCH_Workspace.h — a thin stdlib-only wrapper
# kept separate from "libs/PCH.h" and "libs/PCH_Wrapper.h" because the contest PCHs
# embed `#pragma GCC optimize(...)` and `#pragma GCC target(...)` that conflict
# with CMake's -include-pch injection (see PCH_Workspace.h header comment).
set(CP_ALGORITHMS_PROBE_PCH_TARGET "")
set(CP_ALGORITHMS_PROBE_PCH_STATUS "disabled")
set(CP_ALGORITHMS_PROBE_PCH_HEADER "${CP_ALGORITHMS_ROOT}/libs/PCH_Workspace.h")
if(CP_ALGORITHMS_ENABLE_PROBE_PCH AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.16" AND NOT CP_ALGORITHMS_ENABLE_PROBE_SANITIZERS)
  if(EXISTS "${CP_ALGORITHMS_PROBE_PCH_HEADER}")
    set(CP_ALGORITHMS_PROBE_PCH_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/cp_algorithms_probe_pch.cpp")
    file(WRITE "${CP_ALGORITHMS_PROBE_PCH_SOURCE}" "int cp_algorithms_probe_pch_anchor = 0;\n")
    add_library(cp_algorithms_probe_pch STATIC EXCLUDE_FROM_ALL "${CP_ALGORITHMS_PROBE_PCH_SOURCE}")
    target_link_libraries(cp_algorithms_probe_pch PRIVATE cp_algorithms_workspace)
    target_precompile_headers(cp_algorithms_probe_pch PRIVATE "${CP_ALGORITHMS_PROBE_PCH_HEADER}")
    set_target_properties(cp_algorithms_probe_pch PROPERTIES FOLDER "probes")
    set(CP_ALGORITHMS_PROBE_PCH_TARGET cp_algorithms_probe_pch)
    set(CP_ALGORITHMS_PROBE_PCH_STATUS "enabled (libs/PCH_Workspace.h)")
  else()
    set(CP_ALGORITHMS_PROBE_PCH_STATUS "header not found at libs/PCH_Workspace.h")
  endif()
elseif(CP_ALGORITHMS_ENABLE_PROBE_PCH AND CP_ALGORITHMS_ENABLE_PROBE_SANITIZERS)
  set(CP_ALGORITHMS_PROBE_PCH_STATUS "disabled (sanitizers active)")
elseif(CP_ALGORITHMS_ENABLE_PROBE_PCH)
  set(CP_ALGORITHMS_PROBE_PCH_STATUS "unsupported (CMake < 3.16)")
endif()

# ----------------------------- Helper Functions ----------------------------- #
# cp_algorithms_sanitize_identifier: converts an arbitrary path or string into
# a valid CMake/C identifier by replacing non-alphanumeric characters.
function(cp_algorithms_sanitize_identifier out_var raw_value)
  string(MAKE_C_IDENTIFIER "${raw_value}" sanitized_value)
  set(${out_var} "${sanitized_value}" PARENT_SCOPE)
endfunction()

# cp_algorithms_register_probe: creates a single probe target and appends its
# name to the given global property list.
#   - .runtime.cpp sources become executables (need a main, produce a binary).
#   - all other sources become OBJECT libraries (compile-only smoke tests).
# PCH is applied via REUSE_FROM if a donor target was configured above.
function(cp_algorithms_register_probe property_name target_stem source_path)
  cp_algorithms_sanitize_identifier(target_name "${target_stem}")
  get_filename_component(source_name "${source_path}" NAME)
  string(REGEX MATCH "\\.runtime\\.cpp$" is_runtime_probe "${source_name}")

  if(is_runtime_probe)
    add_executable("${target_name}" EXCLUDE_FROM_ALL "${source_path}")
    target_link_libraries("${target_name}" PRIVATE cp_algorithms_workspace)
    set_target_properties(
      "${target_name}"
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CP_ALGORITHMS_RUNTIME_DIR}"
        FOLDER "probes"
    )
    if(CP_ALGORITHMS_PROBE_SANITIZER_LINK_FLAGS)
      target_link_options("${target_name}" PRIVATE ${CP_ALGORITHMS_PROBE_SANITIZER_LINK_FLAGS})
    endif()
    if(CP_ALGORITHMS_FAST_LINKER_FLAGS)
      target_link_options("${target_name}" PRIVATE ${CP_ALGORITHMS_FAST_LINKER_FLAGS})
    endif()
    if(CP_ALGORITHMS_ENABLE_CTEST)
      add_test(
        NAME "${target_name}"
        COMMAND "$<TARGET_FILE:${target_name}>"
      )
      set_tests_properties("${target_name}" PROPERTIES LABELS "probe;runtime")
    endif()
  else()
    add_library("${target_name}" OBJECT EXCLUDE_FROM_ALL "${source_path}")
    target_link_libraries("${target_name}" PRIVATE cp_algorithms_workspace)
    set_target_properties("${target_name}" PROPERTIES FOLDER "probes")
  endif()

  if(CP_ALGORITHMS_PROBE_PCH_TARGET)
    target_precompile_headers("${target_name}" REUSE_FROM "${CP_ALGORITHMS_PROBE_PCH_TARGET}")
    add_dependencies("${target_name}" "${CP_ALGORITHMS_PROBE_PCH_TARGET}")
  endif()

  set_property(GLOBAL APPEND PROPERTY "${property_name}" "${target_name}")
endfunction()

# cp_algorithms_make_header_probe_source: synthesizes a minimal .cpp that
# includes one public header. The probe TU is OBJECT-only (no linkage), so a
# main() is not required — and omitting it makes the sources unity-buildable.
# Returns the generated probe source path through out_var.
function(cp_algorithms_make_header_probe_source out_var header_path)
  file(RELATIVE_PATH relative_header "${CP_ALGORITHMS_ROOT}" "${header_path}")
  set(probe_source "${CP_ALGORITHMS_PROBE_DIR}/${relative_header}.cpp")
  get_filename_component(probe_directory "${probe_source}" DIRECTORY)
  file(MAKE_DIRECTORY "${probe_directory}")
  file(
    WRITE "${probe_source}"
    "// Auto-generated CP Algorithms header probe.\n"
    "// OBJECT-library translation unit; no linkage, no main() required.\n"
    "#include \"${relative_header}\"\n"
  )
  set(${out_var} "${probe_source}" PARENT_SCOPE)
endfunction()

# cp_algorithms_register_unity_object_target: creates a single OBJECT library
# whose sources are bundled into one (or a few) unity translation units. Used
# for families where the source files are pure includes — no file-scope
# symbols that would collide when concatenated.
function(cp_algorithms_register_unity_object_target target_name property_name sources_var)
  add_library("${target_name}" OBJECT EXCLUDE_FROM_ALL ${${sources_var}})
  target_link_libraries("${target_name}" PRIVATE cp_algorithms_workspace)
  set_target_properties("${target_name}" PROPERTIES
    FOLDER "probes"
    UNITY_BUILD ON
    UNITY_BUILD_MODE BATCH
    UNITY_BUILD_BATCH_SIZE 16
  )
  if(CP_ALGORITHMS_PROBE_PCH_TARGET)
    target_precompile_headers("${target_name}" REUSE_FROM "${CP_ALGORITHMS_PROBE_PCH_TARGET}")
    add_dependencies("${target_name}" "${CP_ALGORITHMS_PROBE_PCH_TARGET}")
  endif()
  set_property(GLOBAL APPEND PROPERTY "${property_name}" "${target_name}")
endfunction()

# ------------------------- Template Scaffold Probes ------------------------- #
# Registers each contest template source so the language server indexes it with
# real target context. GLOB (not GLOB_RECURSE) keeps discovery flat — templates
# live at one level under templates/cpp/.
if(CP_ALGORITHMS_ENABLE_TEMPLATE_SCAFFOLDS)
  file(GLOB cp_algorithms_template_scaffolds CONFIGURE_DEPENDS "${CP_ALGORITHMS_ROOT}/templates/cpp/*.cpp")
  foreach(scaffold_source IN LISTS cp_algorithms_template_scaffolds)
    file(RELATIVE_PATH relative_scaffold "${CP_ALGORITHMS_ROOT}" "${scaffold_source}")
    cp_algorithms_register_probe(
      "CP_ALGORITHMS_SCAFFOLD_TARGETS"
      "cp_algorithms_scaffold_${relative_scaffold}"
      "${scaffold_source}"
    )
  endforeach()
endif()

# ------------------------ Module Verification Probes ------------------------ #
# Discovers all .cpp files under "verify/modules/" recursively. The manifest-
# driven distinction between .runtime.cpp (executable) and .compile.cpp (OBJECT
# library) is handled inside cp_algorithms_register_probe.
if(CP_ALGORITHMS_ENABLE_VERIFY_PROBES)
  file(GLOB_RECURSE cp_algorithms_verify_sources CONFIGURE_DEPENDS "${CP_ALGORITHMS_ROOT}/verify/modules/*.cpp")
  foreach(verify_source IN LISTS cp_algorithms_verify_sources)
    file(RELATIVE_PATH relative_verify "${CP_ALGORITHMS_ROOT}" "${verify_source}")
    cp_algorithms_register_probe(
      "CP_ALGORITHMS_VERIFY_TARGETS"
      "cp_algorithms_verify_${relative_verify}"
      "${verify_source}"
    )
  endforeach()
endif()

# ------------------------------ Header Probes ------------------------------- #
# Generates one synthetic .cpp per public header and registers it as a probe.
# The generated files land in CP_ALGORITHMS_PROBE_DIR (binary dir) so the
# source tree is not polluted. These probes give clangd accurate per-header
# compile flags without relying on heuristic include-path guessing.
if(CP_ALGORITHMS_ENABLE_HEADER_PROBES)
  file(GLOB_RECURSE cp_algorithms_public_headers CONFIGURE_DEPENDS
    "${CP_ALGORITHMS_ROOT}/templates/*.hpp"
    "${CP_ALGORITHMS_ROOT}/modules/*.hpp"
    "${CP_ALGORITHMS_ROOT}/libs/*.h"
    "${CP_ALGORITHMS_ROOT}/libs/*.hpp"
  )
  list(FILTER cp_algorithms_public_headers EXCLUDE REGEX
    "/templates/modules/(IO_Composite|IO_Defs)\\.hpp$"
  )

  set(cp_algorithms_header_probe_sources "")
  foreach(header_path IN LISTS cp_algorithms_public_headers)
    cp_algorithms_make_header_probe_source(_cp_alg_probe_source "${header_path}")
    list(APPEND cp_algorithms_header_probe_sources "${_cp_alg_probe_source}")
  endforeach()
  unset(_cp_alg_probe_source)

  if(CP_ALGORITHMS_ENABLE_UNITY_BUILD AND cp_algorithms_header_probe_sources)
    # Single unity OBJECT library: CMake bundles up to UNITY_BUILD_BATCH_SIZE
    # probe sources per generated TU, slashing parser overhead vs. 150+ TUs.
    cp_algorithms_register_unity_object_target(
      cp_algorithms_header_unity
      "CP_ALGORITHMS_HEADER_TARGETS"
      cp_algorithms_header_probe_sources
    )
  else()
    foreach(probe_source IN LISTS cp_algorithms_header_probe_sources)
      file(RELATIVE_PATH _cp_alg_rel_probe "${CP_ALGORITHMS_PROBE_DIR}" "${probe_source}")
      cp_algorithms_register_probe(
        "CP_ALGORITHMS_HEADER_TARGETS"
        "cp_algorithms_header_${_cp_alg_rel_probe}"
        "${probe_source}"
      )
    endforeach()
    unset(_cp_alg_rel_probe)
  endif()
endif()

# ------------------- Per-Family Aggregate Custom Targets -------------------- #
# Each family gets a named custom target that depends on all its probe targets.
# These are the targets exposed via build presets (algorithms-workspace-verify,
# algorithms-workspace-headers, algorithms-workspace-scaffolds).
get_property(cp_algorithms_verify_targets GLOBAL PROPERTY CP_ALGORITHMS_VERIFY_TARGETS)
if(cp_algorithms_verify_targets)
  add_custom_target(cp_algorithms_verify_probes DEPENDS ${cp_algorithms_verify_targets})
  set_target_properties(cp_algorithms_verify_probes PROPERTIES FOLDER "probes")
endif()

get_property(cp_algorithms_header_targets GLOBAL PROPERTY CP_ALGORITHMS_HEADER_TARGETS)
if(cp_algorithms_header_targets)
  add_custom_target(cp_algorithms_header_probes DEPENDS ${cp_algorithms_header_targets})
  set_target_properties(cp_algorithms_header_probes PROPERTIES FOLDER "probes")
endif()

get_property(cp_algorithms_scaffold_targets GLOBAL PROPERTY CP_ALGORITHMS_SCAFFOLD_TARGETS)
if(cp_algorithms_scaffold_targets)
  add_custom_target(cp_algorithms_template_scaffolds DEPENDS ${cp_algorithms_scaffold_targets})
  set_target_properties(cp_algorithms_template_scaffolds PROPERTIES FOLDER "probes")
endif()

# ------------------------ Umbrella All-Probes Target ------------------------ #
# cp_algorithms_all_probes depends on whichever family aggregates were actually
# registered, so disabling a family via its option keeps this target valid.
set(_cp_all_probe_deps "")
if(cp_algorithms_verify_targets)
  list(APPEND _cp_all_probe_deps cp_algorithms_verify_probes)
endif()
if(cp_algorithms_header_targets)
  list(APPEND _cp_all_probe_deps cp_algorithms_header_probes)
endif()
if(cp_algorithms_scaffold_targets)
  list(APPEND _cp_all_probe_deps cp_algorithms_template_scaffolds)
endif()
if(_cp_all_probe_deps)
  add_custom_target(cp_algorithms_all_probes DEPENDS ${_cp_all_probe_deps})
  set_target_properties(cp_algorithms_all_probes PROPERTIES FOLDER "probes")
endif()

# ---------------------------- Configure Summary ----------------------------- #
list(LENGTH cp_algorithms_verify_targets  _cp_n_verify)
list(LENGTH cp_algorithms_header_targets  _cp_n_headers)
list(LENGTH cp_algorithms_scaffold_targets _cp_n_scaffolds)

if(CP_ALGORITHMS_ENABLE_PROBE_SANITIZERS)
  set(_cp_alg_san_status "enabled")
else()
  set(_cp_alg_san_status "disabled")
endif()
if(CP_ALGORITHMS_ENABLE_TIMING)
  set(_cp_alg_timing_status "enabled")
else()
  set(_cp_alg_timing_status "disabled")
endif()
if(CP_ALGORITHMS_WERROR)
  set(_cp_alg_werror_status "enabled")
else()
  set(_cp_alg_werror_status "disabled")
endif()
if(CP_ALGORITHMS_ENABLE_CTEST)
  set(_cp_alg_ctest_status "enabled (use 'ctest' after building runtime probes)")
else()
  set(_cp_alg_ctest_status "disabled")
endif()
if(CP_ALGORITHMS_ENABLE_UNITY_BUILD)
  set(_cp_alg_unity_status "enabled (header probes bundled, batch=16)")
else()
  set(_cp_alg_unity_status "disabled")
endif()
if(CP_ALGORITHMS_FAST_LINKER_FLAGS)
  set(_cp_alg_linker_status "lld")
else()
  set(_cp_alg_linker_status "system default")
endif()

message(STATUS "CP Algorithms Workspace configured:")
message(STATUS "  Verify probes   : ${_cp_n_verify}")
message(STATUS "  Header probes   : ${_cp_n_headers}")
message(STATUS "  Scaffold probes : ${_cp_n_scaffolds}")
message(STATUS "  Probe PCH       : ${CP_ALGORITHMS_PROBE_PCH_STATUS}")
message(STATUS "  Unity build     : ${_cp_alg_unity_status}")
message(STATUS "  Runtime linker  : ${_cp_alg_linker_status}")
message(STATUS "  Sanitizers      : ${_cp_alg_san_status}")
message(STATUS "  Timing reports  : ${_cp_alg_timing_status}")
message(STATUS "  Warnings as err : ${_cp_alg_werror_status}")
message(STATUS "  CTest           : ${_cp_alg_ctest_status}")

# ============================================================================ #
