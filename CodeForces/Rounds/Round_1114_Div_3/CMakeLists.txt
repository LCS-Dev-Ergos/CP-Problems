# Thin round-level CMakeLists: delegates all build logic to centralized modules.
cmake_minimum_required(VERSION 3.20)
project(competitive_programming LANGUAGES CXX)

set(CP_ROUND_BOOTSTRAP "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/cmake/CPRoundBootstrap.cmake")
if(NOT EXISTS "${CP_ROUND_BOOTSTRAP}")
  message(FATAL_ERROR
    "Central build bootstrap not found: ${CP_ROUND_BOOTSTRAP}\n"
    "Run cppinit (or recreate algorithms/cmake symlink) to restore shared build modules.")
endif()

include("${CP_ROUND_BOOTSTRAP}")
