#include "modules/optimization/SlopeTrick.hpp"

int main() {
  SlopeTrick f;
  f.add_abs(3);
  f.add_abs(7);
  f.add_x_minus_a(1);
  // min |x-3| + |x-7| + (x-1)+ is attained at x = 3.
  return (f.min() == 6 && f.eval(3) == 6 && f.eval(7) == 10) ? 0 : 1;
}
