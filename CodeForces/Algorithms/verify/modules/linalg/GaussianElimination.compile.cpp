#include "modules/linalg/GaussianElimination.hpp"
#include "templates/modules/Mod_Int.hpp"

using Mint = ModInt<998244353>;

int main() {
  const Matrix<Mint> a = Matrix<Mint>::identity(2);
  return matrix_rank(a) == 2 && matrix_det(a) == Mint(1) ? 0 : 1;
}
