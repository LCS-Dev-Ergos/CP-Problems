#include "modules/optimization/MinPlusConvolution.hpp"

int main() {
  const VecI64 a = {0, 1, 3, 6};
  const VecI64 b = {0, 2, 5};
  const VecI64 want = {0, 1, 3, 6, 8, 11};
  const bool convex_ok = (minplus_convolution_convex_convex(a, b) == want);
  const bool mixed_ok = (minplus_convolution_arbitrary_convex(a, b) == want);
  return (convex_ok && mixed_ok) ? 0 : 1;
}
