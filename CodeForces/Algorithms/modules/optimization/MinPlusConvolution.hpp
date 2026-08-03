#ifndef CP_MODULES_OPTIMIZATION_MIN_PLUS_CONVOLUTION_HPP
#define CP_MODULES_OPTIMIZATION_MIN_PLUS_CONVOLUTION_HPP

#include "MonotoneMinima.hpp"

/**
 * @file MinPlusConvolution.hpp
 * @brief (min,+) and (max,+) convolution under convexity assumptions.
 *
 * @details The product is @c c[k]=min(a[i]+b[j]) over @c i+j==k, which is
 * quadratic in general. Convexity is what buys the speedup, and how much
 * depends on how many operands have it:
 * - both convex: O(n+m), by merging the two slope sequences;
 * - one convex: O((n+m) log n), because the resulting matrix is Monge and
 *   @ref monotone_minima applies.
 *
 * A sequence is convex when its consecutive differences are non-decreasing.
 * Both ends may be padded with @c infinity<T> to mark an unusable prefix or
 * suffix, which is the usual way to express a knapsack item that cannot be
 * taken fewer than some number of times; interior entries must stay finite.
 * The @c infinity<T> of this library is a quarter of the type maximum, so a
 * padded entry can still be added to a finite one without overflowing.
 *
 * The (max,+) mirrors expect concave operands padded with @c neg_infinity<T>.
 */

/// @brief (min,+) convolution of two convex sequences, in O(n+m).
template <typename T>
[[nodiscard]] auto minplus_convolution_convex_convex(const Vec<T>& a, const Vec<T>& b) -> Vec<T> {
  if (a.empty() || b.empty()) return {};
  I32 n = isz(a);
  I32 m = isz(b);
  Vec<T> c(n + m - 1, infinity<T>);
  while (n > 0 && a[n - 1] >= infinity<T>) --n;
  while (m > 0 && b[m - 1] >= infinity<T>) --m;
  if (n == 0 || m == 0) return c;

  I32 i = 0;
  I32 j = 0;
  while (i < n && a[i] >= infinity<T>) ++i;
  while (j < m && b[j] >= infinity<T>) ++j;
  // Walking the merged slope sequence: the cheapest way to raise the output
  // index by one is always to follow whichever operand rises less steeply.
  c[i + j] = a[i] + b[j];
  FOR(k, i + j + 1, n + m - 1) {
    if (j == m - 1 || (i != n - 1 && a[i + 1] + b[j] < a[i] + b[j + 1])) {
      c[k] = a[++i] + b[j];
    } else {
      c[k] = a[i] + b[++j];
    }
  }
  return c;
}

/// @brief (min,+) convolution of an arbitrary @p a with a convex @p b, in O((n+m) log n).
template <typename T>
[[nodiscard]] auto minplus_convolution_arbitrary_convex(const Vec<T>& a, const Vec<T>& b) -> Vec<T> {
  if (a.empty() || b.empty()) return {};
  const I32 n = isz(a);
  I32 m = isz(b);
  Vec<T> c(n + m - 1, infinity<T>);
  while (m > 0 && b[m - 1] >= infinity<T>) --m;
  if (m == 0) return c;
  I32 lo = 0;
  while (lo < m && b[lo] >= infinity<T>) ++lo;

  // Row k selects the split of output index lo+k, column i the share taken
  // from a. Convexity of b makes that matrix Monge, hence totally monotone.
  const I32 rows = n + m - lo - 1;
  auto select = [&](I32 k, I32 i1, I32 i2) -> bool {
    if (k < i2) return false;
    if (k - i1 >= m - lo) return true;
    return a[i1] + b[lo + k - i1] >= a[i2] + b[lo + k - i2];
  };
  const VecI32 argmin = monotone_minima(rows, n, select);
  FOR(k, rows) {
    const T x = a[argmin[k]];
    const T y = b[lo + k - argmin[k]];
    if (x < infinity<T> && y < infinity<T>) c[lo + k] = x + y;
  }
  return c;
}

/// @brief (max,+) convolution of two concave sequences, in O(n+m).
template <typename T>
[[nodiscard]] auto maxplus_convolution_concave_concave(const Vec<T>& a, const Vec<T>& b) -> Vec<T> {
  if (a.empty() || b.empty()) return {};
  I32 n = isz(a);
  I32 m = isz(b);
  Vec<T> c(n + m - 1, neg_infinity<T>);
  while (n > 0 && a[n - 1] <= neg_infinity<T>) --n;
  while (m > 0 && b[m - 1] <= neg_infinity<T>) --m;
  if (n == 0 || m == 0) return c;

  I32 i = 0;
  I32 j = 0;
  while (i < n && a[i] <= neg_infinity<T>) ++i;
  while (j < m && b[j] <= neg_infinity<T>) ++j;
  c[i + j] = a[i] + b[j];
  FOR(k, i + j + 1, n + m - 1) {
    if (j == m - 1 || (i != n - 1 && a[i + 1] + b[j] > a[i] + b[j + 1])) {
      c[k] = a[++i] + b[j];
    } else {
      c[k] = a[i] + b[++j];
    }
  }
  return c;
}

/// @brief (max,+) convolution of an arbitrary @p a with a concave @p b, in O((n+m) log n).
template <typename T>
[[nodiscard]] auto maxplus_convolution_arbitrary_concave(const Vec<T>& a, const Vec<T>& b) -> Vec<T> {
  if (a.empty() || b.empty()) return {};
  const I32 n = isz(a);
  I32 m = isz(b);
  Vec<T> c(n + m - 1, neg_infinity<T>);
  while (m > 0 && b[m - 1] <= neg_infinity<T>) --m;
  if (m == 0) return c;
  I32 lo = 0;
  while (lo < m && b[lo] <= neg_infinity<T>) ++lo;

  const I32 rows = n + m - lo - 1;
  auto select = [&](I32 k, I32 i1, I32 i2) -> bool {
    if (k < i2) return false;
    if (k - i1 >= m - lo) return true;
    return a[i1] + b[lo + k - i1] <= a[i2] + b[lo + k - i2];
  };
  const VecI32 argmax = monotone_minima(rows, n, select);
  FOR(k, rows) {
    const T x = a[argmax[k]];
    const T y = b[lo + k - argmax[k]];
    if (x > neg_infinity<T> && y > neg_infinity<T>) c[lo + k] = x + y;
  }
  return c;
}

#endif
