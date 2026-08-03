#include "modules/optimization/KnuthOptimization.hpp"

int main() {
  // Merging stones of sizes 1, 2, 3: best is (1+2) then +3, for 3 + 6 = 9.
  const VecI64 a = {1, 2, 3};
  auto w = [&](I32 i, I32 j) -> I64 {
    I64 s = 0;
    FOR(k, i, j + 1) s += a[k];
    return s;
  };
  const Vec2D<I64> dp = knuth_interval_dp<I64>(3, w);
  return dp[0][2] == 9 ? 0 : 1;
}
