#pragma once
#include "templates/core/Config.hpp"

#if CP_ENABLE_GCC_OPTIMIZE_PRAGMAS || CP_ENABLE_ARCH_TARGET_PRAGMAS
  //===----------------------------------------------------------------------===//
  /* Compiler Pragmas */

  #if defined(__GNUC__) && !defined(__clang__)
    #if CP_ENABLE_GCC_OPTIMIZE_PRAGMAS
      #if CP_ENABLE_AGGRESSIVE_OPTIMIZATIONS
        #pragma GCC optimize("O3,unroll-loops,inline-functions,omit-frame-pointer")
      #else
        #pragma GCC optimize("O3,unroll-loops,inline-functions")
      #endif
      // Separate opt-in: fast-math drops NaN/Inf and reassociates FP.
      #if CP_ENABLE_FAST_MATH
        #pragma GCC optimize("Ofast,fast-math")
      #endif
    #endif
    #if CP_ENABLE_ARCH_TARGET_PRAGMAS
      #if defined(__x86_64__) && !defined(__MINGW32__) && !defined(__MINGW64__)
        #pragma GCC target("avx2,bmi,bmi2,popcnt,lzcnt,sse4.2,fma")
      #endif
      #if defined(__aarch64__) && !defined(__MINGW32__) && !defined(__MINGW64__)
        #pragma GCC target("+simd,+crypto,+fp16")
      #endif
    #endif
  #endif
#endif
