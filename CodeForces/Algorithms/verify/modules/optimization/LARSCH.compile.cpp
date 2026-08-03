#include "modules/optimization/LARSCH.hpp"

int main() {
  const Vec2D<I64> a = {{4, 9, 9}, {5, 1, 9}, {7, 3, 2}};
  auto entry = [&](I32 i, I32 j) -> I64 { return a[i][j]; };
  LARSCH<I64, decltype(entry)> larsch(3, entry);
  return (larsch.get_argmin() == 0 && larsch.get_argmin() == 1 && larsch.get_argmin() == 2) ? 0 : 1;
}
