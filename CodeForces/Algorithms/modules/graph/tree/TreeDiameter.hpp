#ifndef CP_MODULES_GRAPH_TREE_TREE_DIAMETER_HPP
#define CP_MODULES_GRAPH_TREE_TREE_DIAMETER_HPP

#include "../Graph.hpp"
#include "../_Common.hpp"

/// @brief Diameter endpoints, distance, and vertex path in one tree component.
template <typename Weight = I64>
struct TreeDiameterResult {
  I32 from = -1;
  I32 to   = -1;
  Weight dist = Weight{};
  VecI32 path;
};

template <typename Weight = I64>
struct TreeDiameter {
  using Result = TreeDiameterResult<Weight>;

  /// @brief Computes the diameter of the connected component containing root.
  [[nodiscard]] static auto solve(const Graph<Weight>& g, I32 root = 0) -> Result {
    my_assert(0 <= root && root < g.n);
    const I32 a = farthest(g, root).first;
    auto [b, dist, parent] = farthest_with_parent(g, a);

    VecI32 path;
    for (I32 v = b; v != -1; v = parent[v]) path.push_back(v);
    std::ranges::reverse(path);
    return {.from = a, .to = b, .dist = dist[b], .path = path};
  }

private:
  [[nodiscard]] static auto farthest(const Graph<Weight>& g, I32 root) -> Pair<I32, Weight> {
    auto [v, dist, parent] = farthest_with_parent(g, root);
    return {v, dist[v]};
  }

  [[nodiscard]] static auto farthest_with_parent(const Graph<Weight>& g, I32 root)
      -> Tuple<I32, Vec<Weight>, VecI32> {
    Vec<Weight> dist(g.n, Weight{});
    VecI32 parent(g.n, -1);
    Vec<char> seen(g.n, false);
    VecI32 st{root};
    seen[root] = true;

    while (!st.empty()) {
      const I32 v = st.back();
      st.pop_back();
      for (const auto& e : g.adj[v]) {
        if (seen[e.to]) continue;
        seen[e.to] = true;
        parent[e.to] = v;
        dist[e.to] = dist[v] + e.weight;
        st.push_back(e.to);
      }
    }

    I32 best = root;
    FOR(v, g.n) {
      if (seen[v] && dist[best] < dist[v]) best = v;
    }
    return {best, dist, parent};
  }
};

template <typename Weight = I64>
[[nodiscard]] auto tree_diameter(const Graph<Weight>& g, I32 root = 0) -> TreeDiameterResult<Weight> {
  return TreeDiameter<Weight>::solve(g, root);
}

#endif
