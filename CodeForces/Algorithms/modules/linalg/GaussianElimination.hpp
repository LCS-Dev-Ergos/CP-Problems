#ifndef CP_MODULES_LINALG_GAUSSIAN_ELIMINATION_HPP
#define CP_MODULES_LINALG_GAUSSIAN_ELIMINATION_HPP

#include "Matrix.hpp"

/**
 * @file GaussianElimination.hpp
 * @brief Rank, determinant, inverse and linear solving over a field.
 *
 * @details All routines assume T is a field: pivots are chosen as the first
 * non-zero entry and divided out. That is exact for @c ModInt but numerically
 * naive for floating point, which needs partial pivoting by magnitude.
 */

namespace cp::linalg_detail {

/// @brief Reduces @p a to RREF using only the first @p width columns as pivots.
/// @return Pivot column indices, in increasing order.
template <typename T>
auto rref(Matrix<T>& a, I32 width) -> VecI32 {
  VecI32 pivots;
  I32 rank = 0;
  FOR(j, width) {
    if (rank == a.n) break;
    I32 sel = -1;
    FOR(i, rank, a.n) {
      if (a[i][j] != T(0)) {
        sel = i;
        break;
      }
    }
    if (sel == -1) continue;
    a.swap_rows(rank, sel);
    const T inv = T(1) / a[rank][j];
    FOR(k, j, a.m) a[rank][k] *= inv;
    FOR(i, a.n) {
      if (i == rank) continue;
      const T c = a[i][j];
      if (c == T(0)) continue;
      FOR(k, j, a.m) a[i][k] -= a[rank][k] * c;
    }
    pivots.push_back(j);
    ++rank;
  }
  return pivots;
}

} // namespace cp::linalg_detail

/// @brief Rank of a matrix over a field.
template <typename T>
auto matrix_rank(Matrix<T> a) -> I32 {
  return isz(cp::linalg_detail::rref(a, a.m));
}

/// @brief Determinant of a square matrix over a field.
template <typename T>
auto matrix_det(Matrix<T> a) -> T {
  my_assert(a.n == a.m);
  T res(1);
  FOR(j, a.n) {
    I32 sel = -1;
    FOR(i, j, a.n) {
      if (a[i][j] != T(0)) {
        sel = i;
        break;
      }
    }
    if (sel == -1) return T(0);
    if (sel != j) {
      a.swap_rows(j, sel);
      res = -res;
    }
    res *= a[j][j];
    const T inv = T(1) / a[j][j];
    FOR(i, j + 1, a.n) {
      const T c = a[i][j] * inv;
      if (c == T(0)) continue;
      FOR(k, j, a.n) a[i][k] -= a[j][k] * c;
    }
  }
  return res;
}

/// @brief Inverse of a square matrix, or an empty optional when singular.
template <typename T>
auto matrix_inverse(const Matrix<T>& a) -> Optional<Matrix<T>> {
  my_assert(a.n == a.m);
  const I32 n = a.n;
  Matrix<T> aug(n, 2 * n);
  FOR(i, n) {
    FOR(j, n) aug[i][j] = a[i][j];
    aug[i][n + i] = T(1);
  }
  if (isz(cp::linalg_detail::rref(aug, n)) != n) return std::nullopt;

  Matrix<T> out(n, n);
  FOR(i, n) FOR(j, n) out[i][j] = aug[i][n + j];
  return out;
}

/**
 * @brief Solves a * x = b over a field.
 *
 * @return Empty when the system is inconsistent. Otherwise row 0 holds one
 *         particular solution and rows 1.. are a basis of the kernel, so the
 *         full solution set is row 0 plus any span of the remaining rows.
 */
template <typename T>
auto solve_linear(const Matrix<T>& a, const Vec<T>& b) -> Vec2D<T> {
  my_assert(a.n == isz(b));
  const I32 n = a.n;
  const I32 m = a.m;

  Matrix<T> aug(n, m + 1);
  FOR(i, n) {
    FOR(j, m) aug[i][j] = a[i][j];
    aug[i][m] = b[i];
  }
  const VecI32 pivots = cp::linalg_detail::rref(aug, m);
  const I32 rank = isz(pivots);
  FOR(i, rank, n) {
    if (aug[i][m] != T(0)) return {};
  }

  Vec2D<T> res(1, Vec<T>(m, T{}));
  VecI32 pivot_row(m, -1);
  FOR(i, rank) {
    res[0][pivots[i]] = aug[i][m];
    pivot_row[pivots[i]] = i;
  }
  FOR(j, m) {
    if (pivot_row[j] != -1) continue;
    Vec<T> x(m, T{});
    x[j] = T(-1);
    FOR(k, j) {
      if (pivot_row[k] != -1) x[k] = aug[pivot_row[k]][j];
    }
    res.push_back(std::move(x));
  }
  return res;
}

#endif
