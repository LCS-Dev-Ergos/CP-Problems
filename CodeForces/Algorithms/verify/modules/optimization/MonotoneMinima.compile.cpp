#include "modules/optimization/MonotoneMinima.hpp"

int main() {
  const Vec2D<I64> a = {{5, 3, 4}, {6, 2, 1}};
  auto select = [&](I32 i, I32 j, I32 k) -> bool { return a[i][k] < a[i][j]; };
  const VecI32 want = {1, 2};
  return (monotone_minima(2, 3, select) == want && smawk(2, 3, select) == want) ? 0 : 1;
}
