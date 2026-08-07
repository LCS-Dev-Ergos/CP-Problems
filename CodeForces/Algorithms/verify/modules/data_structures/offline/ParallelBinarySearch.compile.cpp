#include "modules/data_structures/offline/ParallelBinarySearch.hpp"

int main() {
  // Prefix sums of {1, 1, 1, 1}: the first prefix reaching 3 has length 3.
  I64 total = 0;
  const VecI32 answer = parallel_binary_search(
      2,
      4,
      0,
      [&] { total = 0; },
      [&](I32) { total += 1; },
      [&](I32 q) { return total >= I64(q + 2); });
  return (answer[0] == 2 && answer[1] == 3) ? 0 : 1;
}
