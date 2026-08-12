#include "modules/graph/DFS.hpp"

#include <cassert>

int main() {
  Graph<> g(7, true);
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  g.add_edge(1, 3);
  g.add_edge(1, 4);
  g.add_edge(2, 5);
  g.add_edge(4, 0);
  assert((dfs(g, 0) == VecI32{0, 1, 3, 4, 2, 5}));
  assert((dfs(g, 6) == VecI32{6}));
  assert(dfs(g, 7).empty());
  return 0;
}
