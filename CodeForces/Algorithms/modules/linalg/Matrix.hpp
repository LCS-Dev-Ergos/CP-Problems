#ifndef CP_MODULES_LINALG_MATRIX_HPP
#define CP_MODULES_LINALG_MATRIX_HPP

#include "_Common.hpp"

/**
 * @brief Dense row-major matrix over a ring, with binary exponentiation.
 *
 * @details Storage is a single flat buffer; @c a[i] yields a raw row pointer so
 * @c a[i][j] keeps the usual syntax without a vector-of-vectors layout. The
 * element type only needs @c +=, @c -=, @c * and construction from an integer,
 * so it works with @c ModInt, @c I64 and floating point alike.
 */
template <typename T>
struct Matrix {
  I32 n = 0;
  I32 m = 0;
  Vec<T> dat;

  Matrix() = default;
  Matrix(I32 rows, I32 cols) : n(rows), m(cols), dat(Size(rows) * Size(cols), T{}) {}

  /// @brief Builds from a row-major nested vector.
  explicit Matrix(const Vec2D<T>& rows) {
    n = isz(rows);
    m = n == 0 ? 0 : isz(rows[0]);
    dat.assign(Size(n) * Size(m), T{});
    FOR(i, n) FOR(j, m) (*this)[i][j] = rows[i][j];
  }

  /// @brief Identity matrix of order @p order.
  static auto identity(I32 order) -> Matrix {
    Matrix a(order, order);
    FOR(i, order) a[i][i] = T(1);
    return a;
  }

  auto operator[](I32 i) -> T* { return dat.data() + Size(i) * Size(m); }
  auto operator[](I32 i) const -> const T* { return dat.data() + Size(i) * Size(m); }

  [[nodiscard]] auto rows() const -> I32 { return n; }
  [[nodiscard]] auto cols() const -> I32 { return m; }

  /// @brief Swaps two rows in place.
  void swap_rows(I32 i, I32 j) {
    if (i == j) return;
    FOR(k, m) std::swap((*this)[i][k], (*this)[j][k]);
  }

  auto operator+=(const Matrix& rhs) -> Matrix& {
    my_assert(n == rhs.n && m == rhs.m);
    FOR(i, isz(dat)) dat[i] += rhs.dat[i];
    return *this;
  }

  auto operator-=(const Matrix& rhs) -> Matrix& {
    my_assert(n == rhs.n && m == rhs.m);
    FOR(i, isz(dat)) dat[i] -= rhs.dat[i];
    return *this;
  }

  friend auto operator+(Matrix lhs, const Matrix& rhs) -> Matrix { return lhs += rhs; }
  friend auto operator-(Matrix lhs, const Matrix& rhs) -> Matrix { return lhs -= rhs; }

  friend auto operator*(const Matrix& lhs, const Matrix& rhs) -> Matrix {
    my_assert(lhs.m == rhs.n);
    Matrix out(lhs.n, rhs.m);
    // Transposing the right operand keeps both inner loops sequential.
    Vec<T> t(Size(rhs.m) * Size(rhs.n), T{});
    FOR(i, rhs.n) FOR(j, rhs.m) t[Size(j) * Size(rhs.n) + Size(i)] = rhs[i][j];
    FOR(i, lhs.n) {
      const T* a = lhs[i];
      T* c = out[i];
      FOR(j, rhs.m) {
        const T* b = t.data() + Size(j) * Size(rhs.n);
        T s{};
        FOR(k, lhs.m) s += a[k] * b[k];
        c[j] = s;
      }
    }
    return out;
  }

  /// @brief Matrix-vector product.
  friend auto operator*(const Matrix& lhs, const Vec<T>& v) -> Vec<T> {
    my_assert(lhs.m == isz(v));
    Vec<T> out(lhs.n, T{});
    FOR(i, lhs.n) {
      const T* a = lhs[i];
      T s{};
      FOR(k, lhs.m) s += a[k] * v[k];
      out[i] = s;
    }
    return out;
  }

  /// @brief @p k -th power of a square matrix.
  [[nodiscard]] auto pow(I64 k) const -> Matrix {
    my_assert(n == m && k >= 0);
    Matrix res = identity(n);
    Matrix base = *this;
    while (k > 0) {
      if (k & 1) res = res * base;
      k >>= 1;
      if (k) base = base * base;
    }
    return res;
  }

  [[nodiscard]] auto transpose() const -> Matrix {
    Matrix out(m, n);
    FOR(i, n) FOR(j, m) out[j][i] = (*this)[i][j];
    return out;
  }

  friend auto operator==(const Matrix& lhs, const Matrix& rhs) -> bool {
    return lhs.n == rhs.n && lhs.m == rhs.m && lhs.dat == rhs.dat;
  }
};

#endif
