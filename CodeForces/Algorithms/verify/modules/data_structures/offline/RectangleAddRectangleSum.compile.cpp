#include "modules/data_structures/offline/RectangleAddRectangleSum.hpp"

int main() {
  RectangleAddRectangleSum<I64> solver;
  solver.add_query(0, 4, 0, 8, 5);
  solver.sum_query(2, 6, 3, 9);
  solver.sum_query(0, 4, 0, 8);

  // A narrower coordinate type has to instantiate just as well.
  RectangleAddRectangleSum<I64, I32> narrow;
  narrow.add_query(0, 2, 0, 2, 1);
  narrow.sum_query(0, 2, 0, 2);

  const VecI64 answer = solver.calc();
  return (answer == VecI64{50, 160} && narrow.calc() == VecI64{4}) ? 0 : 1;
}
