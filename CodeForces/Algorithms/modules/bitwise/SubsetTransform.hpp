#ifndef CP_MODULES_BITWISE_SUBSET_TRANSFORM_HPP
#define CP_MODULES_BITWISE_SUBSET_TRANSFORM_HPP

#include "_Common.hpp"

/**
 * @file SubsetTransform.hpp
 * @brief Zeta/Moebius transforms over the subset lattice and their convolutions.
 *
 * @details Every routine takes a vector indexed by bitmask whose length is a
 * power of two. XOR convolution lives in @c Number_Theory.hpp instead, under
 * @c FWHT, since it needs a different butterfly.
 */

/// @brief In-place subset sum: f[S] becomes the sum of f[T] over T subset of S.
template <typename T>
void subset_zeta(Vec<T>& f) {
  const I32 n = isz(f);
  my_assert(n > 0 && (n & (n - 1)) == 0);
  for (I32 bit = 1; bit < n; bit <<= 1) {
    FOR(mask, n) {
      if (mask & bit) f[mask] += f[mask ^ bit];
    }
  }
}

/// @brief Inverse of @c subset_zeta.
template <typename T>
void subset_mobius(Vec<T>& f) {
  const I32 n = isz(f);
  my_assert(n > 0 && (n & (n - 1)) == 0);
  for (I32 bit = 1; bit < n; bit <<= 1) {
    FOR(mask, n) {
      if (mask & bit) f[mask] -= f[mask ^ bit];
    }
  }
}

/// @brief In-place superset sum: f[S] becomes the sum of f[T] over T superset of S.
template <typename T>
void superset_zeta(Vec<T>& f) {
  const I32 n = isz(f);
  my_assert(n > 0 && (n & (n - 1)) == 0);
  for (I32 bit = 1; bit < n; bit <<= 1) {
    FOR(mask, n) {
      if (!(mask & bit)) f[mask] += f[mask | bit];
    }
  }
}

/// @brief Inverse of @c superset_zeta.
template <typename T>
void superset_mobius(Vec<T>& f) {
  const I32 n = isz(f);
  my_assert(n > 0 && (n & (n - 1)) == 0);
  for (I32 bit = 1; bit < n; bit <<= 1) {
    FOR(mask, n) {
      if (!(mask & bit)) f[mask] -= f[mask | bit];
    }
  }
}

/// @brief OR convolution: c[S] = sum of a[X] * b[Y] over X | Y == S.
template <typename T>
auto or_convolution(Vec<T> a, Vec<T> b) -> Vec<T> {
  my_assert(isz(a) == isz(b));
  subset_zeta(a);
  subset_zeta(b);
  FOR(i, isz(a)) a[i] *= b[i];
  subset_mobius(a);
  return a;
}

/// @brief AND convolution: c[S] = sum of a[X] * b[Y] over X & Y == S.
template <typename T>
auto and_convolution(Vec<T> a, Vec<T> b) -> Vec<T> {
  my_assert(isz(a) == isz(b));
  superset_zeta(a);
  superset_zeta(b);
  FOR(i, isz(a)) a[i] *= b[i];
  superset_mobius(a);
  return a;
}

/**
 * @brief Subset convolution: c[S] = sum of a[X] * b[Y] over disjoint X | Y == S.
 *
 * @details Ranks each mask by popcount so that the disjointness condition
 * becomes an ordinary degree constraint, then runs one zeta per rank.
 * Complexity O(n^2 * 2^n) for masks of n bits.
 */
template <typename T>
auto subset_convolution(const Vec<T>& a, const Vec<T>& b) -> Vec<T> {
  const I32 n = isz(a);
  my_assert(n > 0 && (n & (n - 1)) == 0 && isz(b) == n);
  const I32 bits = bit_width(n) - 1;

  Vec2D<T> fa(bits + 1, Vec<T>(n, T{}));
  Vec2D<T> fb(bits + 1, Vec<T>(n, T{}));
  FOR(mask, n) {
    const I32 r = popcount(mask);
    fa[r][mask] = a[mask];
    fb[r][mask] = b[mask];
  }
  FOR(r, bits + 1) {
    subset_zeta(fa[r]);
    subset_zeta(fb[r]);
  }

  Vec2D<T> fc(bits + 1, Vec<T>(n, T{}));
  FOR(i, bits + 1) FOR(j, bits + 1 - i) {
    FOR(mask, n) fc[i + j][mask] += fa[i][mask] * fb[j][mask];
  }
  FOR(r, bits + 1) subset_mobius(fc[r]);

  Vec<T> res(n, T{});
  FOR(mask, n) res[mask] = fc[popcount(mask)][mask];
  return res;
}

#endif
