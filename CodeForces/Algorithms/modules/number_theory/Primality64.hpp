#ifndef CP_MODULES_NUMBER_THEORY_PRIMALITY64_HPP
#define CP_MODULES_NUMBER_THEORY_PRIMALITY64_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/// @brief Overflow-safe modular multiplication on 64-bit inputs.
inline I64 mod_mul64(I64 a, I64 b, I64 mod) {
  return as<I64>((as<I128>(a) * b) % mod);
}

/// @brief Fast modular exponentiation on 64-bit inputs.
inline I64 mod_pow64(I64 base, I64 exp, I64 mod) {
  I64 result = 1 % mod;
  base %= mod;
  while (exp > 0) {
    if (exp & 1) result = mod_mul64(result, base, mod);
    base = mod_mul64(base, base, mod);
    exp >>= 1;
  }
  return result;
}

/// @brief Absolute difference helper avoiding signed overflow pitfalls.
inline I64 abs_diff64(I64 a, I64 b) {
  return a > b ? a - b : b - a;
}

/// @brief Deterministic Miller-Rabin primality test for 64-bit integers.
inline bool miller_rabin(I64 n) {
  if (n < 2) return false;
  if (n == 2) return true;
  if (n % 2 == 0) return false;

  // Witnesses for deterministic test up to 2^64.
  const Vec<I64> witnesses = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

  I64 d = n - 1;
  I32 r = 0;
  while (d % 2 == 0) {
    d /= 2;
    r++;
  }

  auto check_witness = [&](I64 a) {
    I64 x = mod_pow64(a, d, n);
    if (x == 1 || x == n - 1) return false;

    FOR(_, r - 1) {
      x = mod_mul64(x, x, n);
      if (x == n - 1) return false;
    }
    return true;
  };

  for (I64 a : witnesses) {
    if (a >= n) break;
    if (check_witness(a)) return false;
  }

  return true;
}

#endif
