#ifndef CP_MODULES_GRAPH_BFS_HPP
#define CP_MODULES_GRAPH_BFS_HPP

#include "Graph.hpp"

/// @brief Unweighted shortest-path distances from @p source.
template <typename Weight = I64>
VecI32 bfs(const Graph<Weight>& g, I32 source) {
  VecI32 dist(g.n, -1);
  if (source < 0 || source >= g.n)
    return dist;

  Queue<I32> q;
  dist[source] = 0;
  q.push(source);
  while (!q.empty()) {
    const I32 u = q.front();
    q.pop();
    for (const auto& e : g.adj[u]) {
      if (dist[e.to] != -1)
        continue;
      dist[e.to] = dist[u] + 1;
      q.push(e.to);
    }
  }
  return dist;
}

#endif
