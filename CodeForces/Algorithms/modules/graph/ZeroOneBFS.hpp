#ifndef CP_MODULES_GRAPH_ZERO_ONE_BFS_HPP
#define CP_MODULES_GRAPH_ZERO_ONE_BFS_HPP

#include "Graph.hpp"
#include "_Common.hpp"

#include <cassert>

/**
 * @brief 0-1 BFS shortest paths for graphs with edge weights in {0, 1}.
 *
 * @details Returns distance vector from source. Unreachable vertices get INF.
 * Complexity: O(V + E).
 */
template <typename Weight = I64>
Vec<Weight> zero_one_bfs(const Graph<Weight>& g, I32 source) {
  Vec<Weight> dist(g.n, infinity<Weight>);
  if (source < 0 || source >= g.n)
    return dist;

  Deque<I32> dq;
  dist[source] = 0;
  dq.push_front(source);

  while (!dq.empty()) {
    I32 v = dq.front();
    dq.pop_front();

    for (const auto& e : g.adj[v]) {
      if (e.weight != 0 && e.weight != 1) {
#ifndef NDEBUG
        assert(false && "zero_one_bfs expects edge weights in {0,1}");
#endif
        continue;
      }

      Weight nd = dist[v] + e.weight;
      if (nd < dist[e.to]) {
        dist[e.to] = nd;
        if (e.weight == 0) {
          dq.push_front(e.to);
        } else {
          dq.push_back(e.to);
        }
      }
    }
  }

  return dist;
}

#endif
