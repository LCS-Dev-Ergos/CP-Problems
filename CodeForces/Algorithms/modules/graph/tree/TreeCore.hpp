#ifndef CP_MODULES_GRAPH_TREE_TREE_CORE_HPP
#define CP_MODULES_GRAPH_TREE_TREE_CORE_HPP

#include "../Graph.hpp"
#include "../_Common.hpp"

/**
 * @brief Iterative tree/forest view with Euler order and HLD path segments.
 *
 * @details Builds parent, depth, subtree ranges, heavy paths, LCA, jumps, and
 * edge-to-child mapping. Path segments are closed intervals over @c order; a
 * segment with l > r is traversed upward and must be queried with reversed
 * order for non-commutative monoids.
 */
template <typename Weight = I64>
struct TreeCore {
  I32 n = 0;
  const Graph<Weight>* g = nullptr;
  VecI32 parent;
  VecI32 depth;
  Vec<Weight> depth_weight;
  VecI32 heavy;
  VecI32 head;
  VecI32 pos;
  VecI32 tout;
  VecI32 subtree_size;
  VecI32 order;
  VecI32 component;
  VecI32 root;
  VecI32 edge_to_parent;
  VecI32 edge_to_vertex;

  TreeCore() = default;
  explicit TreeCore(const Graph<Weight>& graph, I32 first_root = 0) { build(graph, first_root); }

  /// @brief Builds a forest view, visiting @p first_root component first when valid.
  void build(const Graph<Weight>& graph, I32 first_root = 0) {
    g = &graph;
    n = graph.n;
    parent.assign(n, -2);
    depth.assign(n, 0);
    depth_weight.assign(n, Weight{});
    heavy.assign(n, -1);
    head.assign(n, -1);
    pos.assign(n, -1);
    tout.assign(n, -1);
    subtree_size.assign(n, 0);
    order.assign(n, -1);
    component.assign(n, -1);
    root.assign(n, -1);
    edge_to_parent.assign(n, -1);
    edge_to_vertex.assign(std::max<I32>(graph.m, 0), -1);

    VecI32 roots;
    if (0 <= first_root && first_root < n) roots.push_back(first_root);
    FOR(v, n) {
      if (v != first_root) roots.push_back(v);
    }

    VecI32 visit_order;
    visit_order.reserve(n);
    for (I32 start : roots) {
      if (parent[start] != -2) continue;
      parent[start] = -1;
      component[start] = start;
      root[start] = start;

      VecI32 st{start};
      while (!st.empty()) {
        const I32 v = st.back();
        st.pop_back();
        visit_order.push_back(v);
        for (const auto& e : graph.adj[v]) {
          if (e.to == parent[v] || parent[e.to] != -2) continue;
          parent[e.to] = v;
          depth[e.to] = depth[v] + 1;
          depth_weight[e.to] = depth_weight[v] + e.weight;
          component[e.to] = start;
          root[e.to] = start;
          edge_to_parent[e.to] = e.id;
          if (0 <= e.id && e.id < isz(edge_to_vertex)) edge_to_vertex[e.id] = e.to;
          st.push_back(e.to);
        }
      }
    }

    for (I32 i = isz(visit_order) - 1; i >= 0; --i) {
      const I32 v = visit_order[i];
      subtree_size[v] = 1;
      I32 best_size = 0;
      for (const auto& e : graph.adj[v]) {
        if (parent[e.to] != v) continue;
        subtree_size[v] += subtree_size[e.to];
        if (subtree_size[e.to] > best_size) {
          best_size = subtree_size[e.to];
          heavy[v] = e.to;
        }
      }
    }

    I32 timer = 0;
    for (I32 start : roots) {
      if (start < 0 || start >= n || head[start] != -1) continue;
      Vec<PairI32> chains{{start, start}};
      while (!chains.empty()) {
        auto [v, h] = chains.back();
        chains.pop_back();
        for (I32 cur = v; cur != -1; cur = heavy[cur]) {
          head[cur] = h;
          pos[cur] = timer;
          order[timer++] = cur;
          for (const auto& e : graph.adj[cur]) {
            if (parent[e.to] == cur && e.to != heavy[cur]) {
              chains.push_back({e.to, e.to});
            }
          }
        }
      }
    }
    FOR(v, n) tout[v] = pos[v] + subtree_size[v];
  }

  /// @brief Returns true when both vertices belong to the same tree.
  [[nodiscard]] bool same_tree(I32 u, I32 v) const { return component[u] == component[v]; }

  /// @brief Half-open subtree interval [l, r) in Euler/HLD order.
  [[nodiscard]] PairI32 subtree_range(I32 v) const { return {pos[v], tout[v]}; }

  /// @brief Whether u is inside v's rooted subtree.
  [[nodiscard]] bool in_subtree(I32 u, I32 v) const {
    return same_tree(u, v) && pos[v] <= pos[u] && pos[u] < tout[v];
  }

  /// @brief Lowest common ancestor, or -1 for different trees.
  [[nodiscard]] I32 lca(I32 u, I32 v) const {
    if (!same_tree(u, v)) return -1;
    while (head[u] != head[v]) {
      if (depth[head[u]] > depth[head[v]]) u = parent[head[u]];
      else v = parent[head[v]];
    }
    return depth[u] < depth[v] ? u : v;
  }

  /// @brief Distance in edges, or -1 for different trees.
  [[nodiscard]] I32 dist(I32 u, I32 v) const {
    const I32 w = lca(u, v);
    if (w == -1) return -1;
    return depth[u] + depth[v] - 2 * depth[w];
  }

  /// @brief Weighted distance. Returns zero for vertices in different trees.
  [[nodiscard]] Weight dist_weighted(I32 u, I32 v) const {
    const I32 w = lca(u, v);
    if (w == -1) return Weight{};
    return depth_weight[u] + depth_weight[v] - Weight(2) * depth_weight[w];
  }

  /// @brief k-th ancestor of v; k=0 returns v, invalid requests return -1.
  [[nodiscard]] I32 kth_ancestor(I32 v, I32 k) const {
    if (k < 0 || k > depth[v]) return -1;
    I32 target = depth[v] - k;
    while (depth[head[v]] > target) v = parent[head[v]];
    return order[pos[v] - (depth[v] - target)];
  }

  /// @brief Vertex reached after k edges on path u -> v, or -1 if absent.
  [[nodiscard]] I32 jump(I32 u, I32 v, I32 k) const {
    const I32 w = lca(u, v);
    if (w == -1) return -1;
    const I32 up = depth[u] - depth[w];
    const I32 down = depth[v] - depth[w];
    if (k < 0 || k > up + down) return -1;
    if (k <= up) return kth_ancestor(u, k);
    return kth_ancestor(v, up + down - k);
  }

  /// @brief Converts an undirected edge id to its child endpoint in this forest.
  [[nodiscard]] I32 edge_child(I32 edge_id) const {
    if (edge_id < 0 || edge_id >= isz(edge_to_vertex)) return -1;
    return edge_to_vertex[edge_id];
  }

  /// @brief Closed HLD intervals covering path u -> v; empty for different trees.
  [[nodiscard]] Vec<PairI32> path_segments(I32 u, I32 v, bool edge_mode = false) const {
    Vec<PairI32> up;
    Vec<PairI32> down;
    if (!same_tree(u, v)) return up;

    auto add_up = [&](I32 l, I32 r) {
      if (l >= r) up.push_back({l, r});
    };
    auto add_down = [&](I32 l, I32 r) {
      if (l <= r) down.push_back({l, r});
    };

    while (head[u] != head[v]) {
      if (depth[head[u]] > depth[head[v]]) {
        add_up(pos[u], pos[head[u]]);
        u = parent[head[u]];
      } else {
        add_down(pos[head[v]], pos[v]);
        v = parent[head[v]];
      }
    }

    if (depth[u] > depth[v]) {
      add_up(pos[u], pos[v] + (edge_mode ? 1 : 0));
    } else {
      add_down(pos[u] + (edge_mode ? 1 : 0), pos[v]);
    }

    std::ranges::reverse(down);
    up.insert(up.end(), all(down));
    return up;
  }

  /// @brief Restores all vertices on path u -> v in order.
  [[nodiscard]] VecI32 restore_path(I32 u, I32 v) const {
    VecI32 path;
    for (auto [l, r] : path_segments(u, v, false)) {
      if (l <= r) {
        FOR(i, l, r + 1) path.push_back(order[i]);
      } else {
        for (I32 i = l; i >= r; --i) path.push_back(order[i]);
      }
    }
    return path;
  }
};

#endif
