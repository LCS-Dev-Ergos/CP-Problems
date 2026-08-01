#ifndef CP_MODULES_NUMBER_THEORY_FLOOR_SUM_HPP
#define CP_MODULES_NUMBER_THEORY_FLOOR_SUM_HPP

#include "_Common.hpp"

namespace cp::floor_sum_detail {

/// @brief Euclidean-like descent for non-negative arguments.
constexpr auto floor_sum_unsigned(U64 n, U64 m, U64 a, U64 b) -> U64 {
  U64 ans = 0;
  while (true) {
    if (a >= m) {
      ans += n * (n - 1) / 2 * (a / m);
      a %= m;
    }
    if (b >= m) {
      ans += n * (b / m);
      b %= m;
    }
    const U64 y_max = a * n + b;
    if (y_max < m) break;
    // Swap the roles of the two axes and keep descending, as in Euclid.
    n = y_max / m;
    b = y_max % m;
    std::swap(m, a);
  }
  return ans;
}

} // namespace cp::floor_sum_detail

/**
 * @brief Sum of floor((a*i + b) / m) for i in [0, n).
 *
 * @details Runs in O(log(max(a, m))) by mirroring the Euclidean algorithm on
 * the lattice under the line. Negative @p a and @p b are shifted into range
 * first. Intermediate products are taken modulo 2^64, which is exact for the
 * documented bounds: @p n and @p m below 2^32.
 */
constexpr auto floor_sum(I64 n, I64 m, I64 a, I64 b) -> I64 {
  my_assert(0 <= n && 0 < m);
  U64 ans = 0;
  if (a < 0) {
    const U64 shifted = U64(a % m + m);
    ans -= U64(n) * U64(n - 1) / 2 * ((shifted - U64(a)) / U64(m));
    a = I64(shifted);
  }
  if (b < 0) {
    const U64 shifted = U64(b % m + m);
    ans -= U64(n) * ((shifted - U64(b)) / U64(m));
    b = I64(shifted);
  }
  return I64(ans + cp::floor_sum_detail::floor_sum_unsigned(U64(n), U64(m), U64(a), U64(b)));
}

/**
 * @brief Sum of (a*i + b) mod m for i in [0, n).
 *
 * @details Derived from @c floor_sum via (x mod m) = x - m * floor(x / m).
 * The caller is responsible for keeping a * n * n within I64 range.
 */
constexpr auto mod_sum(I64 n, I64 m, I64 a, I64 b) -> I64 {
  my_assert(0 <= n && 0 < m);
  a = ((a % m) + m) % m;
  b = ((b % m) + m) % m;
  const I64 total = a * (n - 1) * n / 2 + b * n;
  return total - m * floor_sum(n, m, a, b);
}

#endif
