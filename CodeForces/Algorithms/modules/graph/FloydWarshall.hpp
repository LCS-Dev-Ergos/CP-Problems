#ifndef CP_MODULES_GRAPH_FLOYD_WARSHALL_HPP
#define CP_MODULES_GRAPH_FLOYD_WARSHALL_HPP

#include "_Common.hpp"
#include "templates/core/Constants.hpp"
#include "templates/core/MinMax.hpp"

/**
 * @brief Floyd-Warshall all-pairs shortest paths.
 *
 * @details Handles negative edge weights but no negative cycles. Returns distance matrix.
 * @Complexity: O(V^3).
 */
template <typename Weight = I64>
Vec2D<Weight> floyd_warshall(I32 n, const Vec<std::tuple<I32, I32, Weight>>& edges) {
  Vec2D<Weight> dist(n, Vec<Weight>(n, infinity<Weight>));

  FOR(i, n) dist[i][i] = 0;

  for (const auto& [u, v, w] : edges) {
    chmin(dist[u][v], w);
  }

  FOR(k, n) {
    FOR(i, n) {
      FOR(j, n) {
        if (dist[i][k] != infinity<Weight> && dist[k][j] != infinity<Weight>) {
          chmin(dist[i][j], dist[i][k] + dist[k][j]);
        }
      }
    }
  }

  return dist;
}

#endif
