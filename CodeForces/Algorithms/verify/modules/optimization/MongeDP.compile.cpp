#include "modules/optimization/MongeDP.hpp"

int main() {
  // f(l, r) = (r - l)^2 is Monge, so every step of the optimal path is unit.
  auto f = [](I32 l, I32 r) -> I64 { return I64(r - l) * I64(r - l); };
  const Vec<I64> dp = monge_shortest_path<I64>(4, f);
  const bool path_ok = (dp[4] == 4);
  const bool d_edge_ok = (monge_shortest_path_d_edge<I64>(4, 2, 16, f) == 8);
  return (path_ok && d_edge_ok && is_monge<I64>(4, f)) ? 0 : 1;
}
