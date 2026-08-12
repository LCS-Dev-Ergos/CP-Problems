#include "modules/graph/TopologicalSort.hpp"

int main() {
  Graph<> g(2, true);
  g.add_edge(0, 1);
  return topological_sort(g).size() == 2 ? 0 : 1;
}
