#ifndef CP_MODULES_GRAPH_CORE_HPP
#define CP_MODULES_GRAPH_CORE_HPP

#include "templates/core/Types.hpp"

/// @brief Directed or undirected adjacency-list graph.
template <typename Weight = I64>
struct Graph {
  struct Edge {
    I32 from;
    I32 to;
    Weight weight;
    I32 id;

    Edge(I32 f, I32 t, Weight w = 1, I32 i = -1) : from(f), to(t), weight(w), id(i) {}

    bool operator<(const Edge& other) const { return weight < other.weight; }
  };

  I32 n;
  I32 m = 0;
  Vec2D<Edge> adj;
  Vec<Edge> edges;
  bool directed;

  Graph(I32 n_, bool directed_ = false) : n(n_), adj(n_), directed(directed_) {}

  void add_edge(I32 from, I32 to, Weight weight = 1) {
    adj[from].emplace_back(from, to, weight, m);
    edges.emplace_back(from, to, weight, m);
    if (!directed) {
      adj[to].emplace_back(to, from, weight, m);
      edges.emplace_back(to, from, weight, m);
    }
    ++m;
  }
};

#endif
