#ifndef CP_MODULES_GRAPH_DIJKSTRA_SPARSE_HPP
#define CP_MODULES_GRAPH_DIJKSTRA_SPARSE_HPP

#include "Graph.hpp"
#include "_Common.hpp"
#include "detail/Weight.hpp"
#include "templates/core/Constants.hpp"

#include <cassert>

/**
 * @brief Dijkstra on sparse graphs with path restoration.
 *
 * @details Works for non-negative edge weights.
 * @Complexity: O((V + E) log V).
 */
template <typename Weight = I64>
struct DijkstraResult {
  Vec<Weight> dist;
  VecI32      parent;
};

template <typename Weight = I64>
DijkstraResult<Weight> dijkstra_sparse(const Graph<Weight>& g, I32 source) {
  DijkstraResult<Weight> res{Vec<Weight>(g.n, infinity<Weight>), VecI32(g.n, -1)};
  if (source < 0 || source >= g.n)
    return res;

  using State = Pair<Weight, I32>;
  MinPriorityQueue<State> pq;
  res.dist[source] = 0;
  pq.push({0, source});

  while (!pq.empty()) {
    auto [d, v] = pq.top();
    pq.pop();
    if (d != res.dist[v])
      continue;

    for (const auto& e : g.adj[v]) {
      if (e.weight < 0) {
#ifndef NDEBUG
        assert(false && "dijkstra_sparse requires non-negative edge weights");
#endif
        continue;
      }

      Weight nd{};
      if (cp::graph_detail::checked_add(d, e.weight, nd) && nd < res.dist[e.to]) {
        res.dist[e.to]   = nd;
        res.parent[e.to] = v;
        pq.push({nd, e.to});
      }
    }
  }

  return res;
}

/// @brief Restores source->target path from Dijkstra parent array.
inline VecI32 restore_path(I32 target, const VecI32& parent) {
  VecI32 path;
  if (target < 0 || target >= isz(parent))
    return path;
  for (I32 v = target; v != -1; v = parent[v]) {
    path.push_back(v);
  }
  std::reverse(all(path));
  return path;
}

#endif
