//===----------------------------------------------------------------------===//
/* PCH_Wrapper.h — GCC-Specific Precompiled Header for Competitive Programming
 * Uses bits/stdc++.h and GCC Policy-Based Data Structures.
 * Delegates compiler pragmas, types, constants, and utilities to templates/core.
 * For Clang or MSVC use PCH.h instead.
 */
//===----------------------------------------------------------------------===//
// clang-format off

#ifndef PCH_WRAPPER_HPP
#define PCH_WRAPPER_HPP

#if !defined(__GNUC__) || defined(__clang__)
  #error "PCH_Wrapper.h is designed for GCC. Use PCH.h for Clang/MSVC."
#endif

#if __cplusplus < 202002L
  #warning "C++20 or later recommended for full feature support."
#endif

// Architecture target pragmas are enabled for GCC contest use.
#ifndef CP_ENABLE_ARCH_TARGET_PRAGMAS
  #define CP_ENABLE_ARCH_TARGET_PRAGMAS 0
#endif
// Signal that this PCH manages the preamble (Compiler/StdHeaders/Debug)
// directly, so Types.hpp must not re-enter Preamble.hpp.
#define CP_TYPES_NO_PREAMBLE 1

#include "templates/core/Compiler.hpp"       // Explicit GCC optimizer/target opt-ins
#include "templates/core/StdHeaders.hpp"     // bits/stdc++.h (CP_USE_BITS_HEADER=1 default)
#include "templates/core/FeatureDetect.hpp"  // HAS_INT128, HAS_FLOAT128, PBDS_AVAILABLE
#include "templates/core/Debug.hpp"

// Scalar types and container aliases — Preamble.hpp already handled above.
#include "templates/core/Types.hpp"

#include "templates/core/Constants.hpp"      // PI, E, INF32/64, MOD, POW10[], ...
#include "templates/core/MinMax.hpp"         // chmin/chmax with optional Compare
#include "templates/core/Macros.hpp"         // FOR, sz, ALL, fix, as<>, ...
#include "templates/modules/Random.hpp"      // rng (mt19937_64), rnd(), reseed()

//===----------------------------------------------------------------------===//
// Namespace imports.

using namespace std;
// FeatureDetect.hpp now only probes availability; pull the pb_ds headers in
// directly here since this PCH imports the __gnu_pbds namespace.
#if defined(PBDS_AVAILABLE) && PBDS_AVAILABLE
  #include <ext/pb_ds/assoc_container.hpp>
  #include <ext/pb_ds/tree_policy.hpp>
  using namespace __gnu_pbds;
#endif

//===----------------------------------------------------------------------===//
// Legacy type aliases for older round sources that include this PCH directly.

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
    ios_base::sync_with_stdio(false); \
    cin.tie(nullptr)
#endif

#ifndef SET_PRECISION
  #define SET_PRECISION(n) (cout << fixed << setprecision(n))
#endif

#endif // PCH_WRAPPER_HPP
//===----------------------------------------------------------------------===//
