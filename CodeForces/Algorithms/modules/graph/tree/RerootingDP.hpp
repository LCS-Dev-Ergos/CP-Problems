#ifndef CP_MODULES_GRAPH_TREE_REROOTING_DP_HPP
#define CP_MODULES_GRAPH_TREE_REROOTING_DP_HPP

#include "TreeCore.hpp"

/**
 * @brief All-roots tree DP via the prefix/suffix rerooting trick.
 *
 * @details The DP is described by three callbacks plus a unit element:
 * - @c merge(x, y) combines two sibling contributions; associative with
 *   @c unit as identity. Siblings are combined in adjacency order and the
 *   parent contribution is always leftmost, so commutativity is not required.
 * - @c add_edge(x, e) lifts the value sitting at @c e.to across the edge into
 *   @c e.from. Every edge is handed over oriented that way, so @c e.from is
 *   the vertex being merged into and @c e.to the endpoint being lifted.
 * - @c add_vertex(x, v) folds vertex @p v into its merged children.
 *
 * Forests are supported; each component is rerooted independently. Non-tree
 * edges left in the graph are ignored, only spanning-forest edges are walked.
 *
 * Complexity: O(n) calls to each callback.
 */
template <typename Data, typename Weight = I64>
struct RerootingDP {
  const TreeCore<Weight>* tree = nullptr;
  Vec<Data> subtree; ///< Value of v's subtree, as seen from v's parent.
  Vec<Data> outside; ///< Value of everything outside v's subtree, as seen from v.
  Vec<Data> full;    ///< Value of the whole component when rooted at v.

  RerootingDP() = default;

  template <typename FMerge, typename FEdge, typename FVertex>
  RerootingDP(const TreeCore<Weight>& t, FMerge merge, FEdge add_edge, FVertex add_vertex, const Data& unit) {
    build(t, merge, add_edge, add_vertex, unit);
  }

  /// @brief Value of the whole component when rooted at @p v.
  auto operator[](I32 v) const -> const Data& { return full[v]; }

  /// @brief Value of v's subtree when the component is rooted at @p root.
  [[nodiscard]] auto get(I32 v, I32 root) const -> const Data& {
    if (v == root) return full[v];
    if (!tree->in_subtree(root, v)) return subtree[v];
    return outside[tree->jump(v, root, 1)];
  }

  template <typename FMerge, typename FEdge, typename FVertex>
  void build(const TreeCore<Weight>& t, FMerge merge, FEdge add_edge, FVertex add_vertex, const Data& unit) {
    tree = &t;
    const I32 n = t.n;
    const auto& g = *t.g;
    subtree.assign(n, unit);
    outside.assign(n, unit);
    full.assign(n, unit);

    // Bottom-up: order is a DFS order, so children precede parents in reverse.
    FOR_R(i, n) {
      const I32 v = t.order[i];
      Data acc = unit;
      for (const auto& e : g.adj[v]) {
        if (t.edge_to_parent[e.to] != e.id) continue;
        acc = merge(acc, add_edge(subtree[e.to], e));
      }
      subtree[v] = add_vertex(acc, v);
    }

    // Top-down: outside[p] is already final when p is reached.
    VecI32 kids;
    Vec<Data> val;
    Vec<Data> prefix;
    FOR(i, n) {
      const I32 p = t.order[i];
      kids.clear();
      val.clear();
      Data up = unit;
      for (const auto& e : g.adj[p]) {
        if (t.parent[p] != -1 && e.id == t.edge_to_parent[p]) {
          up = add_edge(outside[p], e);
        } else if (t.edge_to_parent[e.to] == e.id) {
          kids.push_back(e.to);
          val.push_back(add_edge(subtree[e.to], e));
        }
      }

      const I32 k = isz(kids);
      if (k == 0) {
        full[p] = add_vertex(up, p);
        continue;
      }
      prefix.assign(k, up);
      FOR(j, k - 1) prefix[j + 1] = merge(prefix[j], val[j]);
      Data suffix = unit;
      FOR_R(j, k) {
        outside[kids[j]] = add_vertex(merge(prefix[j], suffix), p);
        suffix = merge(val[j], suffix);
      }
      full[p] = add_vertex(merge(up, suffix), p);
    }
  }
};

#endif
