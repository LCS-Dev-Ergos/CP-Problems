# ============================================================================ #
# ------------ CP ROUND CONFIGURATION SUMMARY (Phase 3: Summary) ------------- #
# ============================================================================ #
#
# Description:
#   Displays build environment summary with compiler settings, detected problems,
#   active features (PCH, sanitizers, ccache), and context-sensitive tips.
#
# Usage:
#   Automatically included via CPRoundBootstrap.cmake
#
# ============================================================================ #

include_guard(GLOBAL)

# -------------------------- Configuration Summary --------------------------- #

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Sanitize" CACHE STRING "Supported build types" FORCE)

message(STATUS "")
message(STATUS "${ANSI_COLOR_BLUE}╔═══────── COMPETITIVE PROGRAMMING SETUP SUMMARY ────────────═══╗${ANSI_COLOR_RESET}")
message(STATUS "│")
message(STATUS "│ ${ANSI_COLOR_CYAN}Compiler${ANSI_COLOR_RESET}            : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "│ ${ANSI_COLOR_CYAN}Compiler Path${ANSI_COLOR_RESET}       : ${CMAKE_CXX_COMPILER}")
message(STATUS "│ ${ANSI_COLOR_CYAN}Build Type${ANSI_COLOR_RESET}          : ${ANSI_COLOR_YELLOW}${CMAKE_BUILD_TYPE}${ANSI_COLOR_RESET}")
message(STATUS "│ ${ANSI_COLOR_CYAN}C++ Standard${ANSI_COLOR_RESET}        : C++${CMAKE_CXX_STANDARD}")

# Special note for Sanitize builds
if(CMAKE_BUILD_TYPE STREQUAL "Sanitize")
    if(USING_CLANG_FOR_SANITIZERS)
        if(PCH_SUPPORTED)
            message(STATUS "│ ${ANSI_COLOR_CYAN}Sanitizer Mode${ANSI_COLOR_RESET}      : ${ANSI_COLOR_GREEN}Using Clang with PCH.h${ANSI_COLOR_RESET}")
        else()
            message(STATUS "│ ${ANSI_COLOR_CYAN}Sanitizer Mode${ANSI_COLOR_RESET}      : ${ANSI_COLOR_YELLOW}Using Clang (PCH disabled)${ANSI_COLOR_RESET}")
        endif()
    else()
        message(STATUS "│ ${ANSI_COLOR_CYAN}Sanitizer Mode${ANSI_COLOR_RESET}      : ${ANSI_COLOR_YELLOW}Using GCC (check sanitizer availability)${ANSI_COLOR_RESET}")
    endif()
endif()

if(CP_CCACHE_ENABLED)
    message(STATUS "│ ${ANSI_COLOR_CYAN}Build Cache${ANSI_COLOR_RESET}         : ${ANSI_COLOR_GREEN}ccache enabled${ANSI_COLOR_RESET}")
elseif(CCACHE_PROGRAM)
    message(STATUS "│ ${ANSI_COLOR_CYAN}Build Cache${ANSI_COLOR_RESET}         : ${ANSI_COLOR_YELLOW}ccache detected but disabled${ANSI_COLOR_RESET}")
else()
    message(STATUS "│ ${ANSI_COLOR_CYAN}Build Cache${ANSI_COLOR_RESET}         : ${ANSI_COLOR_YELLOW}ccache not found${ANSI_COLOR_RESET}")
endif()

message(STATUS "│")

if(COMPILER_SYSTEM_INCLUDE_PATHS)
    list(LENGTH COMPILER_SYSTEM_INCLUDE_PATHS INCLUDE_COUNT)
    message(STATUS "│ ${ANSI_COLOR_CYAN}Clangd Assist${ANSI_COLOR_RESET}       : ${ANSI_COLOR_GREEN}OK (${INCLUDE_COUNT} include paths detected)${ANSI_COLOR_RESET}")
else()
    message(STATUS "│ ${ANSI_COLOR_CYAN}Clangd Assist${ANSI_COLOR_RESET}       : ${ANSI_COLOR_YELLOW}WARNING - No include paths found${ANSI_COLOR_RESET}")
endif()

if(PROBLEM_SOURCES)
    list(LENGTH PROBLEM_SOURCES PROBLEM_COUNT)
    message(STATUS "│ ${ANSI_COLOR_CYAN}Problems Found${ANSI_COLOR_RESET}      : ${PROBLEM_COUNT} C++ source files")

    if(PROBLEM_COUNT GREATER 10)
        list(SUBLIST PROBLEM_SOURCES 0 10 SHOWN_SOURCES)
        foreach(source ${SHOWN_SOURCES})
            get_filename_component(name ${source} NAME)
            message(STATUS "│   - ${name}")
        endforeach()
        math(EXPR REMAINING "${PROBLEM_COUNT} - 10")
        message(STATUS "│   ... and ${REMAINING} more.")
    else()
        foreach(source ${PROBLEM_SOURCES})
            get_filename_component(name ${source} NAME)
            message(STATUS "│   - ${name}")
        endforeach()
    endif()
else()
    message(STATUS "│ ${ANSI_COLOR_CYAN}Problems Found${ANSI_COLOR_RESET}      : No C++ source files detected.")
endif()

message(STATUS "│")

# Build type specific tips.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "│ ${ANSI_COLOR_YELLOW}Tip: Use 'cppconf Release' for maximum performance.${ANSI_COLOR_RESET}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "│ ${ANSI_COLOR_GREEN}Performance mode active - optimizations enabled.${ANSI_COLOR_RESET}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Sanitize")
    if(USING_CLANG_FOR_SANITIZERS AND PCH_SUPPORTED)
        message(STATUS "│ ${ANSI_COLOR_CYAN}Using PCH.h as replacement for bits/stdc++.h${ANSI_COLOR_RESET}")
    endif()
    message(STATUS "│ ${ANSI_COLOR_CYAN}Sanitizers active - great for finding bugs!${ANSI_COLOR_RESET}")
endif()

message(STATUS "${ANSI_COLOR_BLUE}╚═══─────────────────────────────────────────────────────────═══╝${ANSI_COLOR_RESET}")

# ============================================================================ #
# End of CP Round Summary Module.
