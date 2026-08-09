#ifndef CP_MODULES_NUMBER_THEORY_NTT_HPP
#define CP_MODULES_NUMBER_THEORY_NTT_HPP

#include "_Common.hpp"

/// @brief Largest k such that 2^k divides @p value; zero is reported as 0.
[[nodiscard]] constexpr auto two_adic_valuation(I64 value) -> I32 {
  I32 exponent = 0;
  while (value != 0 && value % 2 == 0) {
    value /= 2;
    ++exponent;
  }
  return exponent;
}

/**
 * @brief Number Theoretic Transform over a prime modulus of the form c*2^k+1.
 *
 * @details The transform length is capped by the 2-adic valuation of Mod-1,
 * which is where the needed roots of unity live; MAX_LOG_N reports it.
 *
 * @tparam Mod An NTT-friendly prime.
 * @tparam PrimitiveRoot A primitive root modulo @p Mod.
 */
template <I64 Mod, I64 PrimitiveRoot>
struct NTTOver {
  static constexpr I64 MOD = Mod;
  static constexpr I64 ROOT = PrimitiveRoot;
  static constexpr I32 MAX_LOG_N = two_adic_valuation(Mod - 1);
  static constexpr I32 MAX_SIZE = 1 << MAX_LOG_N;

  /// @brief In-place NTT on vector a. If inverse=true, computes inverse transform.
  ///
  /// Every entry has to already be reduced into [0, MOD).
  static void ntt(VecI64& a, bool inverse) {
    const I32 n = isz(a);
    if (n == 0) return;
    my_assert((n & (n - 1)) == 0);
    my_assert(n <= MAX_SIZE);
    if (n == 1) return;

    // Bit reversal (iterative Gray-code style).
    for (I32 i = 1, j = 0; i < n; ++i) {
      I32 bit = n >> 1;
      while (j & bit) {
        j ^= bit;
        bit >>= 1;
      }
      j ^= bit;
      if (i < j) std::swap(a[i], a[j]);
    }

    // NTT computation.
    for (I32 len = 2; len <= n; len <<= 1) {
      const I64 w = inverse ? mod_pow(ROOT, MOD - 1 - (MOD - 1) / len, MOD)
                            : mod_pow(ROOT, (MOD - 1) / len, MOD);

      for (I32 i = 0; i < n; i += len) {
        I64 wn = 1;
        FOR(j, len / 2) {
          const I64 u = a[i + j];
          const I64 v = a[i + j + len / 2] * wn % MOD;
          a[i + j] = (u + v) % MOD;
          a[i + j + len / 2] = (u - v + MOD) % MOD;
          wn = wn * w % MOD;
        }
      }
    }

    if (inverse) {
      const I64 n_inv = mod_pow<I64>(n, MOD - 2, MOD);
      FOR(i, n) a[i] = a[i] * n_inv % MOD;
    }
  }

  /// @brief Polynomial multiplication via NTT; inputs need not be reduced.
  [[nodiscard]] static auto multiply(VecI64 a, VecI64 b) -> VecI64 {
    if (a.empty() || b.empty()) return {};
    const I32 result_size = isz(a) + isz(b) - 1;
    I32 n = 1;
    while (n < result_size) n <<= 1;
    my_assert(n <= MAX_SIZE);

    a.resize(n);
    b.resize(n);
    FOR(i, n) {
      a[i] = mod_floor(a[i], MOD);
      b[i] = mod_floor(b[i], MOD);
    }

    ntt(a, false);
    ntt(b, false);

    FOR(i, n) a[i] = a[i] * b[i] % MOD;

    ntt(a, true);
    a.resize(result_size);

    return a;
  }
};

/// @brief Number Theoretic Transform over MOD2=998244353.
using NTT = NTTOver<MOD2, 3>;

#endif
