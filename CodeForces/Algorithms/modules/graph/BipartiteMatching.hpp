#ifndef CP_MODULES_GRAPH_BIPARTITE_MATCHING_HPP
#define CP_MODULES_GRAPH_BIPARTITE_MATCHING_HPP

#include "_Common.hpp"
#include "templates/core/Constants.hpp"

/**
 * @brief Hopcroft-Karp maximum bipartite matching.
 *
 * O(E * sqrt(V)) complexity.
 * Left vertices: [0, left_size), Right vertices: [0, right_size).
 */
struct BipartiteMatching {
  I32 left_sz, right_sz;
  Vec2D<I32> adj;
  VecI32 match_left, match_right, dist_left;

  BipartiteMatching(I32 left_size, I32 right_size)
      : left_sz(left_size), right_sz(right_size), adj(left_size),
        match_left(left_size, -1), match_right(right_size, -1), dist_left(left_size) {}

  /// @brief Adds an edge from left vertex u to right vertex v.
  void add_edge(I32 u, I32 v) { adj[u].push_back(v); }

  /// @brief BFS phase: builds layered graph of free left vertices.
  bool bfs() {
    Queue<I32> que;
    FOR(u, left_sz) {
      if (match_left[u] == -1) {
        dist_left[u] = 0;
        que.push(u);
      } else {
        dist_left[u] = infinity<I32>;
      }
    }
    bool found = false;
    while (!que.empty()) {
      I32 u = que.front(); que.pop();
      for (I32 v : adj[u]) {
        I32 w = match_right[v];
        if (w == -1) {
          found = true;
        } else if (dist_left[w] == infinity<I32>) {
          dist_left[w] = dist_left[u] + 1;
          que.push(w);
        }
      }
    }
    return found;
  }

  /// @brief DFS phase: finds augmenting path from left vertex u.
  bool dfs(I32 u) {
    for (I32 v : adj[u]) {
      I32 w = match_right[v];
      if (w == -1 || (dist_left[w] == dist_left[u] + 1 && dfs(w))) {
        match_left[u] = v;
        match_right[v] = u;
        return true;
      }
    }
    dist_left[u] = infinity<I32>;
    return false;
  }

  /// @brief Computes maximum matching size.
  I32 max_matching() {
    I32 result = 0;
    while (bfs()) {
      FOR(u, left_sz) {
        if (match_left[u] == -1 && dfs(u)) ++result;
      }
    }
    return result;
  }
};

#endif
