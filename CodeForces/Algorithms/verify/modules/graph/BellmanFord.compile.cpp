#include "modules/graph/BellmanFord.hpp"

int main() {
  Graph<> g(2, true);
  g.add_edge(0, 1, -1);
  const auto [has_cycle, dist] = bellman_ford(g, 0);
  return !has_cycle && dist[1] == -1 ? 0 : 1;
}
