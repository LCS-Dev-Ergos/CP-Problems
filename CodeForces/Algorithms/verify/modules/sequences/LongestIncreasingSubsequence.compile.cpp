#include "modules/sequences/LongestIncreasingSubsequence.hpp"

int main() {
  const VecI32 a = {3, 1, 2, 2, 4};
  const VecI32 idx = longest_increasing_subsequence_indices(a);
  return idx == VecI32({1, 3, 4}) &&
                 longest_increasing_subsequence_length<false>(a) == 4
             ? 0
             : 1;
}
