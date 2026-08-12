#include "modules/graph/BFS.hpp"

int main() {
  Graph<> g(2);
  g.add_edge(0, 1);
  return bfs(g, 0)[1] == 1 ? 0 : 1;
}
