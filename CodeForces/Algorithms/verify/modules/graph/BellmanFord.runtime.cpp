#include "modules/graph/BellmanFord.hpp"

#include <cassert>

int main() {
  Graph<> g(5, true);
  g.add_edge(0, 1, 4);
  g.add_edge(0, 2, 5);
  g.add_edge(1, 2, -2);
  g.add_edge(2, 3, 3);
  const auto [has_cycle, dist] = bellman_ford(g, 0);
  assert(!has_cycle);
  assert((dist == VecI64{0, 4, 2, 5, infinity<I64>}));

  g.add_edge(3, 1, -6);
  assert(bellman_ford(g, 0).first);

  Graph<> overflow(3, true);
  overflow.add_edge(0, 1, Limits<I64>::max() - 1);
  overflow.add_edge(1, 2, 10);
  assert(bellman_ford(overflow, 0).second[2] == infinity<I64>);
  return 0;
}
