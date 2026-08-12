#ifndef CP_MODULES_GRAPH_HLD_HPP
#define CP_MODULES_GRAPH_HLD_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"
#include "Graph.hpp"

/**
 * @brief Heavy-Light Decomposition for tree path/subtree queries.
 * @tparam Weight Graph edge weight type.
 *
 * Maps tree vertices to a base array so any path can be decomposed into
 * O(log V) contiguous segments.
 */
template <typename Weight = I64>
struct HLD {
  I32 n;
  const Graph<Weight>& g;
  VecI32 parent, depth, heavy, head, pos, subtree_size, order, component;
  I32 cur_pos;

  /// @brief Builds decomposition for every connected component.
  HLD(const Graph<Weight>& g)
    : n(g.n), g(g), parent(n, -1), depth(n, 0), heavy(n, -1),
      head(n, 0), pos(n, -1), subtree_size(n, 0), order(n),
      component(n, -1), cur_pos(0) {
    VecBool visited(n, false);

    auto dfs = [&](auto&& self, I32 v, I32 p, I32 comp_id) -> I32 {
      visited[v] = true;
      parent[v] = p;
      component[v] = comp_id;
      subtree_size[v] = 1;
      I32 max_subtree = 0;
      for (const auto& e : g.adj[v]) {
        if (e.to == p || visited[e.to]) continue;
        depth[e.to] = depth[v] + 1;
        I32 child_size = self(self, e.to, v, comp_id);
        subtree_size[v] += child_size;
        if (child_size > max_subtree) {
          max_subtree = child_size;
          heavy[v] = e.to;
        }
      }
      return subtree_size[v];
    };

    auto decompose = [&](auto&& self, I32 v, I32 h, I32 p) -> void {
      head[v] = h;
      order[cur_pos] = v;
      pos[v] = cur_pos++;
      if (heavy[v] != -1) {
        self(self, heavy[v], h, v);
      }
      for (const auto& e : g.adj[v]) {
        if (e.to == p || e.to == heavy[v]) continue;
        self(self, e.to, e.to, v);
      }
    };

    FOR(i, n) {
      if (parent[i] != -1) continue;
      depth[i] = 0;
      dfs(dfs, i, -1, i);
      decompose(decompose, i, i, -1);
    }
  }

  /// @brief LCA query using chain heads. Returns -1 for different components.
  I32 lca(I32 u, I32 v) const {
    if (component[u] != component[v]) return -1;
    while (head[u] != head[v]) {
      if (depth[head[u]] > depth[head[v]]) {
        u = parent[head[u]];
      } else {
        v = parent[head[v]];
      }
    }
    return depth[u] < depth[v] ? u : v;
  }

  /// @brief Iterates path(u, v) as O(log V) base-array segments.
  template <class F>
  void process_path(I32 u, I32 v, F&& op, bool edge_mode = false) const {
    if (component[u] != component[v]) return;
    while (head[u] != head[v]) {
      if (depth[head[u]] < depth[head[v]]) std::swap(u, v);
      op(pos[head[u]], pos[u]);
      u = parent[head[u]];
    }
    if (depth[u] > depth[v]) std::swap(u, v);
    I32 l = pos[u] + as<I32>(edge_mode);
    I32 r = pos[v];
    if (l <= r) op(l, r);
  }

  /// @brief Returns subtree interval in base-array coordinates.
  Pair<I32, I32> subtree_range(I32 v) const {
    return {pos[v], pos[v] + subtree_size[v] - 1};
  }

  /// @brief Returns the k-th ancestor of vertex v (0-th ancestor = v itself).
  I32 kth_ancestor(I32 v, I32 k) const {
    if (k > depth[v]) return -1;
    I32 target = depth[v] - k;
    while (depth[head[v]] > target) {
      v = parent[head[v]];
    }
    return order[pos[v] - (depth[v] - target)];
  }
};

#endif
