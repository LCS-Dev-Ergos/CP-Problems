#ifndef CP_MODULES_GRAPH_KRUSKAL_HPP
#define CP_MODULES_GRAPH_KRUSKAL_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"
#include "DSU.hpp"

/**
 * @brief Kruskal minimum spanning tree.
 *
 * @details Returns total weight and list of original edge indices in the MST.
 * Edges should be provided in (u, v, w) format.
 * @Complexity: O(E log E).
 */
template <typename Weight = I64>
Pair<Weight, VecI32> kruskal_mst(I32 n, const Vec<Tuple<I32, I32, Weight>>& edges) {
  struct EdgeWithId {
    I32 u;
    I32 v;
    Weight w;
    I32 id;
  };

  Vec<EdgeWithId> sorted_edges;
  sorted_edges.reserve(edges.size());
  FOR(i, sz(edges)) {
    auto [u, v, w] = edges[i];
    sorted_edges.push_back({u, v, w, as<I32>(i)});
  }

  std::sort(all(sorted_edges), [](const EdgeWithId& a, const EdgeWithId& b) {
    return a.w < b.w;
  });

  DSU dsu(n);
  Weight total_weight = 0;
  VecI32 mst_edges;
  if (n > 0) mst_edges.reserve(n - 1);

  for (const auto& e : sorted_edges) {
    auto [u, v, w, id] = e;
    if (dsu.unite(u, v)) {
      total_weight += w;
      mst_edges.push_back(id);
      if (sz(mst_edges) == n - 1) break;
    }
  }

  return {total_weight, mst_edges};
}

#endif
