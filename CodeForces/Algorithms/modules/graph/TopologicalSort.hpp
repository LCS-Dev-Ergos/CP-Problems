#ifndef CP_MODULES_GRAPH_TOPOLOGICAL_SORT_HPP
#define CP_MODULES_GRAPH_TOPOLOGICAL_SORT_HPP

#include "Graph.hpp"
#include "_Common.hpp"

/// @brief Kahn topological order; cyclic graphs return their maximal acyclic prefix.
template <typename Weight = I64>
VecI32 topological_sort(const Graph<Weight>& g, bool* has_cycle = nullptr) {
  VecI32 indegree(g.n);
  FOR(u, g.n) {
    for (const auto& e : g.adj[u])
      ++indegree[e.to];
  }

  Queue<I32> q;
  FOR(v, g.n) {
    if (indegree[v] == 0)
      q.push(v);
  }

  VecI32 order;
  order.reserve(g.n);
  while (!q.empty()) {
    const I32 u = q.front();
    q.pop();
    order.push_back(u);
    for (const auto& e : g.adj[u]) {
      if (--indegree[e.to] == 0)
        q.push(e.to);
    }
  }

  if (has_cycle != nullptr)
    *has_cycle = isz(order) != g.n;
  return order;
}

#endif
