#ifndef CP_MODULES_NUMBER_THEORY_BINOMIAL_HPP
#define CP_MODULES_NUMBER_THEORY_BINOMIAL_HPP

#include "_Common.hpp"

/**
 * @brief Factorial tables over a modular ring, with the usual counting queries.
 *
 * @details The modulus must be a prime greater than every argument passed in,
 * so that all factorials involved stay invertible. Unlike @c Lucas, which is
 * bounded by O(p) memory and only fits small primes, this table is bounded by
 * the largest n actually queried and is the right choice for the standard
 * 998244353 / 1e9+7 moduli. The table grows geometrically on demand, so a
 * default-constructed instance is fine when the bound is not known up front.
 */
template <typename Mint>
struct Binomial {
  Vec<Mint> fact;
  Vec<Mint> inv_fact;

  explicit Binomial(I32 n = 1) { extend(n); }

  /// @brief Ensures factorials are tabulated up to @p n inclusive.
  void extend(I32 n) {
    const I32 old = isz(fact);
    if (n < old) return;
    const I32 size = std::max({old * 2, n + 1, 2});
    fact.resize(size);
    inv_fact.resize(size);
    if (old == 0) fact[0] = Mint(1);
    FOR(i, std::max(old, 1), size) fact[i] = fact[i - 1] * Mint(i);
    inv_fact[size - 1] = Mint(1) / fact[size - 1];
    FOR_R(i, size - 1) inv_fact[i] = inv_fact[i + 1] * Mint(i + 1);
  }

  /// @brief Binomial coefficient n choose r; zero outside 0 <= r <= n.
  auto C(I64 n, I64 r) -> Mint {
    if (n < 0 || r < 0 || r > n) return Mint(0);
    my_assert(n <= Limits<I32>::max()); // n beyond the table: use C_huge instead.
    extend(I32(n));
    return fact[n] * inv_fact[r] * inv_fact[n - r];
  }

  /// @brief Falling factorial n * (n-1) * ... * (n-r+1).
  auto P(I64 n, I64 r) -> Mint {
    if (n < 0 || r < 0 || r > n) return Mint(0);
    my_assert(n <= Limits<I32>::max());
    extend(I32(n));
    return fact[n] * inv_fact[n - r];
  }

  /// @brief Multiset coefficient: ways to pick r items from n types with repeats.
  auto H(I64 n, I64 r) -> Mint {
    if (n < 0 || r < 0) return Mint(0);
    if (r == 0) return Mint(1);
    return C(n + r - 1, r);
  }

  /**
   * @brief Binomial coefficient for huge n and small r, without tabulating n.
   *
   * @details Costs O(r) multiplications, so it is the variant to reach for when
   * n is up to 1e18 but r stays small.
   */
  auto C_huge(I64 n, I32 r) -> Mint {
    if (r < 0) return Mint(0);
    extend(r);
    Mint num(1);
    FOR(i, r) num *= Mint(n - i);
    return num * inv_fact[r];
  }

  /// @brief n-th Catalan number.
  auto catalan(I32 n) -> Mint {
    if (n < 0) return Mint(0);
    return C(2 * I64(n), n) - C(2 * I64(n), n + 1);
  }

  /// @brief Modular inverse of n, reusing the tables.
  auto inv(I32 n) -> Mint {
    my_assert(n > 0);
    extend(n);
    return inv_fact[n] * fact[n - 1];
  }
};

#endif
