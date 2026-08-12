#ifndef CP_MODULES_GRAPH_BELLMAN_FORD_HPP
#define CP_MODULES_GRAPH_BELLMAN_FORD_HPP

#include "Graph.hpp"
#include "_Common.hpp"
#include "detail/Weight.hpp"
#include "templates/core/Constants.hpp"

/// @brief Shortest paths and reachable negative-cycle detection.
template <typename Weight = I64>
Pair<bool, Vec<Weight>> bellman_ford(const Graph<Weight>& g, I32 source) {
  Vec<Weight> dist(g.n, infinity<Weight>);
  if (source < 0 || source >= g.n)
    return {false, dist};
  dist[source] = 0;

  FOR(g.n - 1) {
    bool updated = false;
    for (const auto& e : g.edges) {
      if (dist[e.from] == infinity<Weight>)
        continue;
      Weight next{};
      if (cp::graph_detail::checked_add(dist[e.from], e.weight, next) && next < dist[e.to]) {
        dist[e.to] = next;
        updated = true;
      }
    }
    if (!updated)
      break;
  }

  for (const auto& e : g.edges) {
    if (dist[e.from] == infinity<Weight>)
      continue;
    Weight next{};
    if (cp::graph_detail::checked_add(dist[e.from], e.weight, next) && next < dist[e.to])
      return {true, dist};
  }
  return {false, dist};
}

#endif
