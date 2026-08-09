#include "modules/data_structures/offline/PointAddRectangleSum.hpp"

int main() {
  PointAddRectangleSum<I64> solver;
  solver.add_query(3, 7, 5);
  solver.sum_query(0, 4, 0, 8);
  solver.sum_query(4, 9, 0, 8);

  // A wider coordinate type has to instantiate just as well.
  PointAddRectangleSum<I64, I64> wide;
  wide.add_query(1'000'000'000'000LL, -1'000'000'000'000LL, 1);
  wide.sum_query(0, 2'000'000'000'000LL, -2'000'000'000'000LL, 0);

  const VecI64 answer = solver.calc();
  return (answer == VecI64{5, 0} && wide.calc() == VecI64{1}) ? 0 : 1;
}
