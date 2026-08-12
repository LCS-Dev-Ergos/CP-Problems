#ifndef CP_MODULES_STRINGS_ROLLING_HASH_HPP
#define CP_MODULES_STRINGS_ROLLING_HASH_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/// @brief Double rolling hash utility for substring hashing/LCP.
struct RollingHash {
  static constexpr I64 MOD1 = 1e9 + 7;
  static constexpr I64 MOD2 = 1e9 + 9;
  static constexpr I64 BASE1 = 911382323;
  static constexpr I64 BASE2 = 972663749;

  static_assert(BASE1 > 1 && BASE1 < MOD1, "BASE1 must be in (1, MOD1)");
  static_assert(BASE2 > 1 && BASE2 < MOD2, "BASE2 must be in (1, MOD2)");

  String s;
  VecI64 hash1, hash2, pow1, pow2;

  /// @brief Precomputes prefix hashes and powers.
  RollingHash(const String& str) : s(str) {
    I32 n = sz(s);
    hash1.resize(n + 1);
    hash2.resize(n + 1);
    pow1.resize(n + 1);
    pow2.resize(n + 1);

    pow1[0] = pow2[0] = 1;
    FOR(i, n) {
      I64 ch = as<U8>(s[i]);
      hash1[i + 1] = (hash1[i] * BASE1 + ch) % MOD1;
      hash2[i + 1] = (hash2[i] * BASE2 + ch) % MOD2;
      pow1[i + 1]  = (pow1[i] * BASE1) % MOD1;
      pow2[i + 1]  = (pow2[i] * BASE2) % MOD2;
    }
  }

  /// @brief Returns pair hash of substring [l, r).
  PairI64 get_hash(I32 l, I32 r) const {
    I64 h1 = (hash1[r] - hash1[l] * pow1[r - l] % MOD1 + MOD1) % MOD1;
    I64 h2 = (hash2[r] - hash2[l] * pow2[r - l] % MOD2 + MOD2) % MOD2;
    return {h1, h2};
  }

  /// @brief Checks equality of two substrings in O(1).
  bool equal(I32 l1, I32 r1, I32 l2, I32 r2) const {
    return get_hash(l1, r1) == get_hash(l2, r2);
  }

  /// @brief Longest common prefix of suffixes starting at i and j.
  I32 lcp(I32 i, I32 j) const {
    I32 left = 0, right = std::min(sz(s) - i, sz(s) - j) + 1;
    while (right - left > 1) {
      I32 mid = (left + right) / 2;
      if (equal(i, i + mid, j, j + mid)) {
        left = mid;
      } else {
        right = mid;
      }
    }
    return left;
  }
};

#endif
