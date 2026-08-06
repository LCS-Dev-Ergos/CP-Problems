#ifndef CP_MODULES_GRAPH_RANGE_TO_RANGE_GRAPH_HPP
#define CP_MODULES_GRAPH_RANGE_TO_RANGE_GRAPH_HPP

#include "Graph.hpp"

/**
 * @brief Graph on [0, n) with edges that touch whole index ranges at once.
 *
 * @details Naively connecting a vertex to a range costs one edge per element.
 * Two segment trees over the same index space bring that down to O(log n)
 * edges: a downward tree whose internal nodes reach every leaf below them, and
 * an upward tree whose leaves reach every ancestor. A point-to-range edge lands
 * on the O(log n) canonical nodes of the downward tree, a range-to-point edge
 * starts from the canonical nodes of the upward tree, and a range-to-range edge
 * routes both through one fresh hub vertex.
 *
 * The auxiliary nodes carry weight zero, so shortest paths, 0-1 BFS and
 * reachability on the built graph answer the original question directly. Only
 * the first @c n vertices are the caller's; the rest are internal.
 *
 * Sizes: @c 3n vertices before any range-to-range edge, plus one more per such
 * edge, and O(log n) edges per range operation.
 *
 * @code
 * RangeToRangeGraph<I64> builder(n);
 * builder.add_to_range(u, l, r, w);      // u -> every v in [l, r)
 * builder.add_from_range(l, r, v, w);    // every u in [l, r) -> v
 * Graph<I64> g = builder.build();
 * const Vec<I64> dist = g.dijkstra(source);
 * @endcode
 */
template <typename Weight = I64>
struct RangeToRangeGraph {
  I32 n = 0;
  I32 node_count = 0;
  Vec<Tuple<I32, I32, Weight>> pending;

  explicit RangeToRangeGraph(I32 n_) : n(std::max<I32>(n_, 0)) {
    node_count = 3 * n;
    // Downward tree: a parent reaches both children. Upward tree: a child
    // reaches its parent. Both share the caller's vertices as their leaves.
    FOR(i, 2, n + n) {
      pending.eb(down_node(i >> 1), down_node(i), Weight(0));
      pending.eb(up_node(i), up_node(i >> 1), Weight(0));
    }
  }

  /// @brief Total number of vertices the built graph will have.
  [[nodiscard]] auto size() const -> I32 { return node_count; }

  /// @brief Number of edges queued so far.
  [[nodiscard]] auto edge_count() const -> I32 { return isz(pending); }

  /// @brief Plain edge between two caller vertices.
  void add(I32 from, I32 to, Weight weight) {
    my_assert(0 <= from && from < n);
    my_assert(0 <= to && to < n);
    pending.eb(from, to, weight);
  }

  /// @brief Edge from every vertex of [from_l, from_r) to @p to.
  void add_from_range(I32 from_l, I32 from_r, I32 to, Weight weight) {
    my_assert(0 <= from_l && from_l <= from_r && from_r <= n);
    my_assert(0 <= to && to < node_count);
    I32 l = from_l + n;
    I32 r = from_r + n;
    while (l < r) {
      if (l & 1) pending.eb(up_node(l++), to, weight);
      if (r & 1) pending.eb(up_node(--r), to, weight);
      l >>= 1;
      r >>= 1;
    }
  }

  /// @brief Edge from @p from to every vertex of [to_l, to_r).
  void add_to_range(I32 from, I32 to_l, I32 to_r, Weight weight) {
    my_assert(0 <= from && from < node_count);
    my_assert(0 <= to_l && to_l <= to_r && to_r <= n);
    I32 l = to_l + n;
    I32 r = to_r + n;
    while (l < r) {
      if (l & 1) pending.eb(from, down_node(l++), weight);
      if (r & 1) pending.eb(from, down_node(--r), weight);
      l >>= 1;
      r >>= 1;
    }
  }

  /**
   * @brief Edge from every vertex of [from_l, from_r) to every one of [to_l, to_r).
   *
   * @details The whole bundle costs @p weight exactly once, not once per hop.
   * A hub vertex is only spent when both sides really are ranges.
   */
  void add_range_to_range(I32 from_l, I32 from_r, I32 to_l, I32 to_r, Weight weight) {
    my_assert(0 <= from_l && from_l <= from_r && from_r <= n);
    my_assert(0 <= to_l && to_l <= to_r && to_r <= n);
    if (from_l == from_r || to_l == to_r) return;
    if (from_r - from_l == 1) {
      add_to_range(from_l, to_l, to_r, weight);
      return;
    }
    if (to_r - to_l == 1) {
      add_from_range(from_l, from_r, to_l, weight);
      return;
    }
    const I32 hub = node_count++;
    add_from_range(from_l, from_r, hub, weight);
    add_to_range(hub, to_l, to_r, Weight(0));
  }

  /// @brief Materializes the directed graph; vertices [0, n) are the caller's.
  [[nodiscard]] auto build() const -> Graph<Weight> {
    Graph<Weight> g(node_count, true);
    for (const auto& [from, to, weight] : pending) g.add_edge(from, to, weight);
    return g;
  }

  /**
   * @brief Downward-tree vertex for segment-tree index @p i in [1, 2n).
   *
   * @details Leaves collapse onto the caller's own vertices, so no zero-weight
   * hop is wasted at the bottom of either tree.
   */
  [[nodiscard]] auto down_node(I32 i) const -> I32 { return i >= n ? i - n : n + i; }

  /// @brief Upward-tree vertex for segment-tree index @p i in [1, 2n).
  [[nodiscard]] auto up_node(I32 i) const -> I32 { return i >= n ? i - n : n + n + i; }
};

#endif
