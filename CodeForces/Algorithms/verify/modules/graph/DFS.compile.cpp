#include "modules/graph/DFS.hpp"

int main() {
  Graph<> g(2);
  g.add_edge(0, 1);
  return dfs(g, 0).size() == 2 ? 0 : 1;
}
