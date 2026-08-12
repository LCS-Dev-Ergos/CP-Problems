#ifndef CP_MODULES_GRAPH_DFS_HPP
#define CP_MODULES_GRAPH_DFS_HPP

#include "Graph.hpp"

/// @brief Iterative depth-first preorder from @p source.
template <typename Weight = I64>
VecI32 dfs(const Graph<Weight>& g, I32 source) {
  VecI32 order;
  if (source < 0 || source >= g.n)
    return order;

  VecBool seen(g.n);
  Stack<I32> st;
  st.push(source);
  while (!st.empty()) {
    const I32 u = st.top();
    st.pop();
    if (seen[u])
      continue;
    seen[u] = true;
    order.push_back(u);
    for (auto it = g.adj[u].rbegin(); it != g.adj[u].rend(); ++it) {
      if (!seen[it->to])
        st.push(it->to);
    }
  }
  return order;
}

#endif
