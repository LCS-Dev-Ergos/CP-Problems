#include "modules/data_structures/offline/RectangleAddPointSum.hpp"

int main() {
  RectangleAddPointSum<I64> solver;
  solver.add_query(0, 4, 0, 8, 5);
  solver.sum_query(3, 7);
  solver.sum_query(4, 7);

  // A wider coordinate type has to instantiate just as well.
  RectangleAddPointSum<I64, I64> wide;
  wide.add_query(-1'000'000'000'000LL, 1'000'000'000'000LL, 0, 1'000'000'000'000LL, 1);
  wide.sum_query(0, 5);

  const VecI64 answer = solver.calc();
  return (answer == VecI64{5, 0} && wide.calc() == VecI64{1}) ? 0 : 1;
}
