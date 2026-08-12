#ifndef CP_MODULES_GRAPH_MCMF_HPP
#define CP_MODULES_GRAPH_MCMF_HPP

#include "_Common.hpp"
#include "templates/core/Constants.hpp"

/**
 * @brief Min-Cost Max-Flow via Successive Shortest Paths with SPFA.
 *
 * @details Handles negative edge costs correctly via Bellman-Ford (SPFA).
 */
template <typename Cap = I64, typename Cost = I64>
struct MCMF {
  struct Edge {
    I32 to, rev;
    Cap cap;
    Cost cost;
  };

  I32 n;
  Vec<Vec<Edge>> g;
  Vec<Cost> dist;
  VecI32 prev_v, prev_e;
  VecBool in_queue;

  MCMF(I32 n) : n(n), g(n), dist(n), prev_v(n), prev_e(n), in_queue(n) {}

  /// @brief Adds a directed edge with capacity and cost (and reverse with 0 cap, -cost).
  void add_edge(I32 from, I32 to, Cap cap, Cost cost) {
    I32 from_index = isz(g[from]);
    I32 to_index = isz(g[to]);
    if (from == to) ++to_index;
    g[from].push_back({to, to_index, cap, cost});
    g[to].push_back({from, from_index, 0, -cost});
  }

  /// @brief SPFA shortest path from s to t in the residual graph.
  bool spfa(I32 s, I32 t) {
    std::fill(all(dist), infinity<Cost>);
    std::fill(all(in_queue), false);
    dist[s] = 0;
    Queue<I32> que;
    que.push(s);
    in_queue[s] = true;

    while (!que.empty()) {
      I32 v = que.front(); que.pop();
      in_queue[v] = false;
      FOR(i, sz(g[v])) {
        auto& e = g[v][i];
        if (e.cap > 0 && dist[v] + e.cost < dist[e.to]) {
          dist[e.to] = dist[v] + e.cost;
          prev_v[e.to] = v;
          prev_e[e.to] = i;
          if (!in_queue[e.to]) {
            que.push(e.to);
            in_queue[e.to] = true;
          }
        }
      }
    }
    return dist[t] < infinity<Cost>;
  }

  /// @brief Computes min-cost flow up to max_flow units from s to t.
  Pair<Cap, Cost> min_cost_flow(I32 s, I32 t, Cap max_flow) {
    Cap flow = 0;
    Cost cost = 0;
    while (flow < max_flow && spfa(s, t)) {
      // Find bottleneck along shortest path.
      Cap d = max_flow - flow;
      for (I32 v = t; v != s; v = prev_v[v]) {
        d = std::min(d, g[prev_v[v]][prev_e[v]].cap);
      }
      // Augment along path.
      for (I32 v = t; v != s; v = prev_v[v]) {
        auto& e = g[prev_v[v]][prev_e[v]];
        e.cap -= d;
        g[v][e.rev].cap += d;
      }
      flow += d;
      cost += d * dist[t];
    }
    return {flow, cost};
  }
};

#endif
