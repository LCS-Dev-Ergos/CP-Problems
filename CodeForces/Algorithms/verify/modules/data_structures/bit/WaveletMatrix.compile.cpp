#include "modules/data_structures/bit/WaveletMatrix.hpp"

int main() {
  WaveletMatrix<I64> wm(Vec<I64>{10, 20, 10, 30});
  return wm.count_value(0, 4, 10) == 2 && wm.kth(0, 4, 3) == 30 ? 0 : 1;
}
