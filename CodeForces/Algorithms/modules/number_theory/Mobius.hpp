#ifndef CP_MODULES_NUMBER_THEORY_MOBIUS_HPP
#define CP_MODULES_NUMBER_THEORY_MOBIUS_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/// @brief Mobius function precomputation and inversion helper.
struct Mobius {
  VecI32 mu;
  VecI32 primes;
  I32 n;

  /// @brief Computes mu[1..size] in O(size).
  explicit Mobius(const I32 size) : mu(size + 1, 0), n(size) {
    my_assert(size >= 1);
    mu[1] = 1;
    VecI32 min_prime(n + 1, 0);

    FOR(i, 2, n + 1) {
      if (min_prime[i] == 0) {
        min_prime[i] = i;
        primes.push_back(i);
        mu[i] = -1;
      }

      for (const I32 p : primes) {
        const I64 nxt = as<I64>(i) * p;
        if (nxt > n) break;
        min_prime[nxt] = p;
        if (i % p == 0) {
          mu[nxt] = 0;
          break;
        }
        mu[nxt] = -mu[i];
      }
    }
  }

  /// @brief Mobius inversion: f(n) = sum_{d|n} mu(d) * g(n/d), given g(n) = sum_{d|n} f(d).
  [[nodiscard]] auto invert(const VecI64& g) const -> VecI64 {
    VecI64 f(sz(g));
    FOR(i, 1, sz(g)) {
      for (I32 j = i; j < sz(g); j += i) {
        f[j] += mu[i] * g[j / i];
      }
    }
    return f;
  }

  /// @brief Returns all positive divisors of @p value not exceeding @p limit.
  [[nodiscard]] auto divisors_up_to_limit(const I64 value, const I32 limit) const -> VecI32 {
    VecI32 divisors;
    if (value <= 0 || limit <= 0) return divisors;

    const VecPair<I64, I32> factors = factorize(value);
    divisors.reserve(2048);

    const auto dfs = [&](const auto& self, const I32 idx, const I64 cur) -> void {
      if (idx == sz(factors)) {
        if (cur <= limit) divisors.push_back(as<I32>(cur));
        return;
      }

      const auto [prime, exponent] = factors[idx];
      I64 val = cur;
      for (I32 e = 0; e <= exponent; ++e) {
        if (val > limit) break;
        self(self, idx + 1, val);
        if (e == exponent) break;
        val *= prime;
      }
    };

    dfs(dfs, 0, 1);
    std::sort(divisors.begin(), divisors.end());
    return divisors;
  }

  /// @brief Computes weights w[T] = sum_{d|gcd(T, value)} mu(T/d) for T in [1..limit].
  [[nodiscard]] auto build_divisibility_weights(const I64 value, const I32 limit) const -> VecI32 {
    my_assert(limit <= n);
    VecI32 weights((limit + 1), 0);
    if (value <= 0 || limit <= 0) return weights;

    const VecI32 divisors = divisors_up_to_limit(value, limit);
    for (const I32 g : divisors) {
      for (I32 t = 1; t * g <= limit; ++t) {
        weights[t * g] += mu[t];
      }
    }
    return weights;
  }

private:
  [[nodiscard]] auto factorize(const I64 value) const -> VecPair<I64, I32> {
    VecPair<I64, I32> factors;
    I64 x = value;

    for (const I32 p32 : primes) {
      const I64 p = as<I64>(p32);
      if (p * p > x) break;
      if (x % p != 0) continue;

      I32 cnt = 0;
      while (x % p == 0) {
        x /= p;
        ++cnt;
      }
      factors.push_back({p, cnt});
    }

    if (x > 1) factors.push_back({x, 1});
    return factors;
  }
};

#endif
