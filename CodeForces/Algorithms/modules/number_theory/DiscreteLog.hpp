#ifndef CP_MODULES_NUMBER_THEORY_DISCRETE_LOG_HPP
#define CP_MODULES_NUMBER_THEORY_DISCRETE_LOG_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/**
 * @brief Baby-step Giant-step discrete logarithm.
 * @details Returns minimum x such that a^x ≡ b (mod m), or -1 if no solution.
 * Works also when gcd(a, m) > 1 (cp-algorithms extended variant).
*/
inline I64 discrete_log(I64 a, I64 b, I64 m) {
  if (m <= 0) return -1;
  a %= m;
  b %= m;
  if (a < 0) a += m;
  if (b < 0) b += m;
  if (m == 1) return 0;

  I64 k = 1;
  I64 add = 0;
  while (true) {
    I64 g = std::gcd(a, m);
    if (g == 1) break;
    if (b == k) return add;
    if (b % g != 0) return -1;
    b /= g;
    m /= g;
    ++add;
    k = as<I64>((as<I128>(k) * (a / g)) % m);
  }

  I64 n = as<I64>(std::sqrt(as<F80>(m)) + 1);
  I64 an = 1;
  FOR(i, n) an = as<I64>((as<I128>(an) * a) % m);

  UnorderedMap<I64, I64> values;
  values.reserve((n + 1));
  I64 cur = b;
  FOR(q, n + 1) {
    values[cur] = q;
    cur = as<I64>((as<I128>(cur) * a) % m);
  }

  cur = k;
  FOR(p, 1, n + 1) {
    cur = as<I64>((as<I128>(cur) * an) % m);
    auto it = values.find(cur);
    if (it != values.end()) {
      return p * n - it->second + add;
    }
  }

  return -1;  // No solution.
}

#endif
