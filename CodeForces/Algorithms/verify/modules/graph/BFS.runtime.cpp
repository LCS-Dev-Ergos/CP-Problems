#include "modules/graph/BFS.hpp"

#include <cassert>

int main() {
  Graph<> g(6);
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(0, 3);
  g.add_edge(3, 4);
  assert((bfs(g, 0) == VecI32{0, 1, 2, 1, 2, -1}));
  assert((bfs(g, -1) == VecI32(6, -1)));
  return 0;
}
