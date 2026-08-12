#pragma once
#include "templates/core/CoreConcepts.hpp"

//===----------------------------------------------------------------------===//
/* Randomized Hash Utilities (anti-collision for unordered containers) */

namespace cp::hashing {

[[gnu::always_inline]] inline U64 splitmix64(U64 x) noexcept {
  x += 0x9e37'79b9'7f4a'7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58'476d'1ce4'e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d0'49bb'1331'11ebULL;
  return x ^ (x >> 31);
}

[[gnu::always_inline]] inline U64& fixed_random_seed_storage() noexcept {
#ifdef CP_SEED
  static U64 seed = U64(CP_SEED);
#else
  // Clock alone is guessable by an anti-hash test.
  static U64 seed = []() noexcept {
    const U64 ticks = U64(std::chrono::steady_clock::now().time_since_epoch().count());
    try {
      return ticks ^ (U64(std::random_device{}()) * 0x9e37'79b9'7f4a'7c15ULL);
    } catch (...) {
      return ticks ^ U64(reinterpret_cast<std::uintptr_t>(&fixed_random_seed_storage));
    }
  }();
#endif
  return seed;
}

[[gnu::always_inline]] inline U64 fixed_random_seed() noexcept { return fixed_random_seed_storage(); }

inline void set_seed(U64 seed) noexcept { fixed_random_seed_storage() = seed; }

[[nodiscard]] constexpr inline U64 hash_combine(U64 lhs, U64 rhs) noexcept {
  return lhs ^ (rhs + 0x9e37'79b9'7f4a'7c15ULL + (lhs << 6) + (lhs >> 2));
}

template <class T>
[[gnu::always_inline]] inline U64 raw_hash(const T& value) {
  using U = cp::RemoveCvrefT<T>;
  if constexpr (Int<U>)
    return U64(value);
  else if constexpr (Enum<U>)
    return U64(std::underlying_type_t<U>(value));
  else {
    static_assert(Hashable<U>, "raw_hash(): type is not hashable; provide a std::hash specialization.");
    return U64(std::hash<U>{}(value));
  }
}

template <class T>
struct SplitMixHash {
  U64 seed = fixed_random_seed();

  size_t operator()(const T& value) const noexcept(noexcept(raw_hash(value))) {
    return size_t(splitmix64(raw_hash(value) + seed));
  }
};

template <class T, class U>
struct PairHash {
  U64 seed = fixed_random_seed();

  size_t operator()(const std::pair<T, U>& value) const
      noexcept(noexcept(raw_hash(value.first)) && noexcept(raw_hash(value.second))) {
    const U64 lhs = splitmix64(raw_hash(value.first) + seed);
    const U64 rhs = splitmix64(raw_hash(value.second) + seed);
    return size_t(splitmix64(hash_combine(lhs, rhs) + seed));
  }
};

template <class K, class V, class Hash = SplitMixHash<K>>
using FastHashMap = std::unordered_map<K, V, Hash>;

template <class K, class Hash = SplitMixHash<K>>
using FastHashSet = std::unordered_set<K, Hash>;

template <class K1, class K2, class V, class Hash = PairHash<K1, K2>>
using FastHashMap2 = std::unordered_map<std::pair<K1, K2>, V, Hash>;

} // namespace cp::hashing
