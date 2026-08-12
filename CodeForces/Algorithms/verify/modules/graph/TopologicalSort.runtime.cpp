#include "modules/graph/TopologicalSort.hpp"

#include <cassert>

int main() {
  Graph<> dag(5, true);
  dag.add_edge(0, 2);
  dag.add_edge(1, 2);
  dag.add_edge(2, 3);
  dag.add_edge(2, 4);
  bool has_cycle = true;
  const VecI32 order = topological_sort(dag, &has_cycle);
  assert(!has_cycle);
  VecI32 pos(5);
  FOR(i, 5) pos[order[i]] = i;
  for (const auto& e : dag.edges) assert(pos[e.from] < pos[e.to]);

  Graph<> cycle(3, true);
  cycle.add_edge(0, 1);
  cycle.add_edge(1, 2);
  cycle.add_edge(2, 0);
  topological_sort(cycle, &has_cycle);
  assert(has_cycle);
  return 0;
}
