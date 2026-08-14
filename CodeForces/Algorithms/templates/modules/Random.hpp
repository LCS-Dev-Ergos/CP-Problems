#pragma once
#include "templates/core/Contracts.hpp"
#include "templates/core/CoreConcepts.hpp"

//=====----- [ RNG ] ----------------------------------------------------=====//

inline U64 default_rng_seed() noexcept {
#ifdef CP_SEED
  return U64(CP_SEED);
#else
  const U64 ticks = as<U64>(std::chrono::steady_clock::now().time_since_epoch().count());
  U64 entropy = 0;
  try {
    entropy = as<U64>(std::random_device{}());
  } catch (...) {
    entropy = U64(reinterpret_cast<std::uintptr_t>(&default_rng_seed));
  }
  return ticks ^ (entropy * 0x9e37'79b9'7f4a'7c15ULL);
#endif
}

inline std::mt19937_64 rng(default_rng_seed());

inline void reseed(U64 seed) { rng.seed(seed); }

template <cp::Int T>
inline T rnd(T a, T b) {
  static_assert(sizeof(T) <= sizeof(U64), "rnd(): extended 128-bit integers are unsupported.");
  CP_EXPECT(a <= b, "rnd(): expected a <= b.");
  return std::uniform_int_distribution<T>(a, b)(rng);
}

template <cp::Float T>
inline T rnd(T a, T b) {
  CP_EXPECT(std::isfinite(a) && std::isfinite(b) && a <= b,
            "rnd(): expected a finite ordered interval.");
  return std::uniform_real_distribution<T>(a, b)(rng);
}
