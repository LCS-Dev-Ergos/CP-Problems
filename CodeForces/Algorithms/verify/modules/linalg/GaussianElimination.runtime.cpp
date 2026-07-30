#include "modules/linalg/GaussianElimination.hpp"
#include "templates/modules/Mod_Int.hpp"

#include <cassert>

using Mint = ModInt<998244353>;

int main() {
  Matrix<Mint> a(Vec2D<Mint>{{1, 2}, {3, 4}});
  assert(matrix_det(a) == Mint(-2));
  assert(matrix_rank(a) == 2);

  const auto inv = matrix_inverse(a);
  assert(inv.has_value());
  assert(a * *inv == Matrix<Mint>::identity(2));
  assert(*inv * a == Matrix<Mint>::identity(2));

  // Unique solution.
  const auto sol = solve_linear(a, Vec<Mint>{5, 11});
  assert(isz(sol) == 1);
  assert(sol[0][0] == Mint(1) && sol[0][1] == Mint(2));

  // Singular system: rank drops, no inverse, inconsistent right-hand side.
  Matrix<Mint> sing(Vec2D<Mint>{{1, 2}, {2, 4}});
  assert(matrix_det(sing) == Mint(0));
  assert(matrix_rank(sing) == 1);
  assert(!matrix_inverse(sing).has_value());
  assert(solve_linear(sing, Vec<Mint>{1, 3}).empty());

  // Consistent but underdetermined: kernel vectors must annihilate the matrix.
  Matrix<Mint> wide(Vec2D<Mint>{{1, 1, 1}});
  const auto many = solve_linear(wide, Vec<Mint>{6});
  assert(isz(many) == 3);
  assert((wide * many[0])[0] == Mint(6));
  FOR(t, 1, isz(many)) assert((wide * many[t])[0] == Mint(0));

  // Determinant is multiplicative and flips sign on a row swap.
  Matrix<Mint> b(Vec2D<Mint>{{2, 0}, {1, 3}});
  assert(matrix_det(a * b) == matrix_det(a) * matrix_det(b));
  Matrix<Mint> swapped = a;
  swapped.swap_rows(0, 1);
  assert(matrix_det(swapped) == -matrix_det(a));

  // Rank of a zero matrix and of a rectangular matrix.
  assert(matrix_rank(Matrix<Mint>(3, 3)) == 0);
  assert(matrix_rank(Matrix<Mint>(Vec2D<Mint>{{1, 2, 3}, {2, 4, 6}})) == 1);
  return 0;
}
