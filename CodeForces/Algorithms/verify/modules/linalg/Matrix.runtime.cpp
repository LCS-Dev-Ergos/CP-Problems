#include "modules/linalg/Matrix.hpp"
#include "templates/modules/Mod_Int.hpp"

#include <cassert>

using Mint = ModInt<998244353>;

int main() {
  // Fibonacci transfer matrix: the classic reason to have matrix power at all.
  Matrix<Mint> fib(Vec2D<Mint>{{1, 1}, {1, 0}});
  assert(fib.pow(10)[0][1].val() == 55);
  assert(fib.pow(1) == fib);
  assert(fib.pow(0) == Matrix<Mint>::identity(2));

  Matrix<I64> a(Vec2D<I64>{{1, 2, 3}, {4, 5, 6}});
  Matrix<I64> b(Vec2D<I64>{{7, 8}, {9, 10}, {11, 12}});
  const Matrix<I64> c = a * b;
  assert(c.rows() == 2 && c.cols() == 2);
  assert(c[0][0] == 58 && c[0][1] == 64);
  assert(c[1][0] == 139 && c[1][1] == 154);

  const Matrix<I64> t = a.transpose();
  assert(t.rows() == 3 && t.cols() == 2);
  FOR(i, 2) FOR(j, 3) assert(t[j][i] == a[i][j]);

  const Vec<I64> v = a * Vec<I64>{1, 1, 1};
  assert(isz(v) == 2 && v[0] == 6 && v[1] == 15);

  Matrix<I64> sum = a + a;
  FOR(i, 2) FOR(j, 3) assert(sum[i][j] == 2 * a[i][j]);
  sum -= a;
  assert(sum == a);

  Matrix<I64> sw(Vec2D<I64>{{1, 2}, {3, 4}});
  sw.swap_rows(0, 1);
  assert(sw[0][0] == 3 && sw[1][0] == 1);
  return 0;
}
