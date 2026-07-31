#include "modules/graph/tree/RerootingDP.hpp"

int main() {
  Graph<> g(2);
  g.add_edge(0, 1);
  TreeCore<> tree(g);
  RerootingDP<I64> dp(
    tree,
    [](I64 a, I64 b) { return a + b; },
    [](I64 x, const Graph<>::Edge&) { return x; },
    [](I64 x, I32) { return x + 1; },
    I64(0));
  return dp[0] == 2 ? 0 : 1;
}
