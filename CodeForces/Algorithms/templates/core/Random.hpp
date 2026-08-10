#pragma once
#include "templates/core/IdiomAliases.hpp"
#include "templates/core/Macros.hpp"

//===----------------------------------------------------------------------===//
/* Random Utilities */

inline U64 default_rng_seed() {
#ifdef CP_SEED
  return U64(CP_SEED);
#else
  const U64 ticks = as<U64>(std::chrono::steady_clock::now().time_since_epoch().count());
  const U64 entropy = as<U64>(std::random_device{}());
  return ticks ^ (entropy * 0x9e37'79b9'7f4a'7c15ULL);
#endif
}

inline std::mt19937_64 rng(default_rng_seed());

inline void reseed(U64 seed) { rng.seed(seed); }

template <cp::Int T>
inline T rnd(T a, T b) { return std::uniform_int_distribution<T>(a, b)(rng); }

template <cp::Float T>
inline T rnd(T a, T b) { return std::uniform_real_distribution<T>(a, b)(rng); }
