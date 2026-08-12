//===----------------------------------------------------------------------===//
/* PCH.h — Portable Precompiled Header for C++23
 * Competitive programming PCH for GCC, Clang, and MSVC.
 * Delegates compiler pragmas, standard includes, types, constants, and
 * utilities to templates/core. Direct inline definitions removed.
 */
//===----------------------------------------------------------------------===//
// clang-format off

#ifndef MODERN_PCH_H
#define MODERN_PCH_H

// PCH.h uses portable standard includes (no bits/stdc++.h).
#ifndef CP_USE_BITS_HEADER
  #define CP_USE_BITS_HEADER 0
#endif
// Preserve previous aggressive-optimization behavior for GCC.
#ifndef CP_ENABLE_AGGRESSIVE_OPTIMIZATIONS
  #define CP_ENABLE_AGGRESSIVE_OPTIMIZATIONS 1
#endif
#ifndef CP_ENABLE_ARCH_TARGET_PRAGMAS
  #define CP_ENABLE_ARCH_TARGET_PRAGMAS 0
#endif
// Signal that this PCH manages the preamble (Compiler/StdHeaders/Debug)
// directly, so Types.hpp must not re-enter Preamble.hpp.
#define CP_TYPES_NO_PREAMBLE 1

#include "templates/core/Compiler.hpp"       // Explicit GCC optimizer/target opt-ins
#include "templates/core/StdHeaders.hpp"     // PortableStdHeaders.hpp (CP_USE_BITS_HEADER=0)
#include "templates/core/FeatureDetect.hpp"
#include "templates/core/Debug.hpp"

// Scalar types and container aliases — Preamble.hpp already handled above.
#include "templates/core/Types.hpp"

#include "templates/core/Constants.hpp"      // PI, E, INF32/64, MOD, POW10[], ...
#include "templates/core/MinMax.hpp"         // chmin/chmax with optional Compare
#include "templates/core/Macros.hpp"         // FOR, sz, ALL, fix, as<>, ...
#include "templates/modules/Random.hpp"      // rng (mt19937_64), rnd(), reseed()

//===----------------------------------------------------------------------===//
// Legacy type aliases for older round sources that include PCH directly.

#ifndef CP_PCH_ENABLE_SHORT_ALIASES
  #define CP_PCH_ENABLE_SHORT_ALIASES 1
#endif
#if CP_PCH_ENABLE_SHORT_ALIASES
  using ll  = I64;
  using ull = U64;
  using ld  = F80;
  using LL  = I64;
  using ULL = U64;
  using LD  = F80;
#endif

#ifndef CP_PCH_ENABLE_CONTAINER_ALIASES
  #define CP_PCH_ENABLE_CONTAINER_ALIASES 1
#endif
#if CP_PCH_ENABLE_CONTAINER_ALIASES
  template <class T> using VC   = Vec<T>;
  template <class T> using VVC  = Vec2D<T>;
  template <class T> using VVVC = Vec3D<T>;
  template <class T, class U> using P  = Pair<T, U>;
  template <class T, class U> using VP = VecPair<T, U>;
  template <class T, class U> using TP = Pair<T, U>;
#endif

//===----------------------------------------------------------------------===//
// I/O setup macros.

// FAST_IO_SETUP() configures fast I/O only. Use SET_PRECISION() separately.
#ifndef FAST_IO_SETUP
  #define FAST_IO_SETUP() \
    std::ios_base::sync_with_stdio(false); \
    std::cin.tie(nullptr)
#endif

#ifndef SET_PRECISION
  #define SET_PRECISION(n) (std::cout << std::fixed << std::setprecision(n))
#endif

// Sanitizer detection.
#if defined(__has_feature)
  #if __has_feature(address_sanitizer) && !defined(HAS_ASAN)
    #define HAS_ASAN 1
  #endif
  #if __has_feature(undefined_behavior_sanitizer) && !defined(HAS_UBSAN)
    #define HAS_UBSAN 1
  #endif
#endif
#if defined(__SANITIZE_ADDRESS__) && !defined(HAS_ASAN)
  #define HAS_ASAN 1
#endif
// Ensure HAS_ASAN/HAS_UBSAN are always 0 or 1 so they can be used in ternaries.
#ifndef HAS_ASAN
  #define HAS_ASAN 0
#endif
#ifndef HAS_UBSAN
  #define HAS_UBSAN 0
#endif

// RAII I/O optimizer — applies FAST_IO_SETUP() at program start.
struct IOOptimizer {
    IOOptimizer() {
        FAST_IO_SETUP();
#if HAS_ASAN
        // Re-enable sync under ASan for accurate I/O ordering in debug builds.
        std::ios_base::sync_with_stdio(true);
#endif
    }
    ~IOOptimizer() {
        std::cout.flush();
        std::cerr.flush();
    }
};

#ifndef DISABLE_AUTO_IO_OPTIMIZATION
  [[maybe_unused]] static IOOptimizer io_optimizer_instance;
#endif

//===----------------------------------------------------------------------===//
// Namespace aliases.

#if CP_USE_GLOBAL_STD_NAMESPACE
  using namespace std;
#endif

#if __cplusplus >= 202002L
  namespace ranges = std::ranges;
  namespace views  = std::ranges::views;
#endif

//===----------------------------------------------------------------------===//
// Opt-in compile-time diagnostics.
// Define SHOW_COMPILATION_INFO in LOCAL builds to print a feature summary.

#if defined(LOCAL) && defined(SHOW_COMPILATION_INFO)
  // 0/1 wrappers for feature-test macros that may be absent.
  #ifdef __cpp_concepts
    #define CP_HAS_CONCEPTS 1
  #else
    #define CP_HAS_CONCEPTS 0
  #endif
  #ifdef __cpp_lib_ranges
    #define CP_HAS_RANGES 1
  #else
    #define CP_HAS_RANGES 0
  #endif
  #ifdef __cpp_lib_format
    #define CP_HAS_FORMAT 1
  #else
    #define CP_HAS_FORMAT 0
  #endif

  namespace {
    struct CompilationDiagnostics {
      CompilationDiagnostics() {
        std::cerr << "╭─── PCH Compilation Info ─────────────╮\n";
        std::cerr << "│ Compiler : "
#if defined(__clang__) && defined(__apple_build_version__)
                  << "Apple Clang " << __clang_major__ << "." << __clang_minor__
#elif defined(__clang__)
                  << "Clang " << __clang_major__ << "." << __clang_minor__
#elif defined(__GNUC__)
                  << "GCC " << __GNUC__ << "." << __GNUC_MINOR__
#elif defined(_MSC_VER)
                  << "MSVC " << _MSC_VER
#else
                  << "Unknown"
#endif
                  << "\n";
        std::cerr << "│ C++ Std  : " << __cplusplus                          << "\n";
        std::cerr << "│ Features :\n";
        std::cerr << "│   • int128   : " << (HAS_INT128      ? "✓" : "✗")    << "\n";
        std::cerr << "│   • float128 : " << (HAS_FLOAT128    ? "✓" : "✗")    << "\n";
        std::cerr << "│   • PBDS     : " << (PBDS_AVAILABLE  ? "✓" : "✗")    << "\n";
        std::cerr << "│   • concepts : " << (CP_HAS_CONCEPTS ? "✓" : "✗")    << "\n";
        std::cerr << "│   • ranges   : " << (CP_HAS_RANGES   ? "✓" : "✗")    << "\n";
        std::cerr << "│   • format   : " << (CP_HAS_FORMAT   ? "✓" : "✗")    << "\n";
        std::cerr << (HAS_ASAN  ? "│ ⚠  ASan  : ENABLED\n" : "");
        std::cerr << (HAS_UBSAN ? "│ ⚠  UBSan : ENABLED\n" : "");
        std::cerr << "╰──────────────────────────────────────╯\n";
      }
    } _cp_diagnostics_instance;
  }
#endif

#endif // MODERN_PCH_H
//===----------------------------------------------------------------------===//
