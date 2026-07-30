#include "modules/linalg/Matrix.hpp"

int main() {
  Matrix<I64> a(2, 2);
  a[0][0] = 1;
  a[1][1] = 1;
  return a.pow(3) == Matrix<I64>::identity(2) ? 0 : 1;
}
