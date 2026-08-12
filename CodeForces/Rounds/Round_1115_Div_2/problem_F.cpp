#ifndef CP_TEMPLATE_PROFILE_STRICT
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#define NEED_MACROS
#define NEED_TIMER
#define NEED_TYPES
#define CP_IO_PROFILE_SIMPLE

#include "templates/Base.hpp"
#include "modules/graph/tree/TreeCore.hpp"

//===----------------------------------------------------------------------===//
/* Main Solver Function */

void solve() {
  INT(n);
  VecI32 c(n), k(n);
  IN(c, k);

  Vec2D<I32> at(n);
  FOR(v, n) {
    --c[v];
    at[c[v]].push_back(v);
  }

  Graph<> g(n);
  FOR(n - 1) {
    INT(u, v);
    g.add_edge(--u, --v);
  }

  TreeCore<> tr(g, 0);
  VecI64 ans(n, -1);

  FOR(col, n) {
    if (at[col].empty()) continue;

    auto nodes  = at[col];
    auto by_pos = [&](I32 u, I32 v) { return tr.pos[u] < tr.pos[v]; };
    sort(all(nodes), by_pos);

    I32 m = isz(nodes);
    FOR(i, 1, m) nodes.push_back(tr.lca(nodes[i - 1], nodes[i]));
    sort(all(nodes), by_pos);
    nodes.erase(unique(all(nodes)), nodes.end());

    I32 q = isz(nodes);
    VecI32 par(q, -1), dist(q), st{0};
    auto anc = [&](I32 u, I32 v) {
      return tr.pos[u] <= tr.pos[v] && tr.pos[v] < tr.pos[u] + tr.subtree_size[u];
    };

    FOR(i, 1, q) {
      while (!anc(nodes[st.back()], nodes[i])) st.pop_back();
      par[i]  = st.back();
      dist[i] = tr.depth[nodes[i]] - tr.depth[nodes[par[i]]];
      st.push_back(i);
    }

    VecI32 sub(q);
    FOR(i, q) sub[i] = c[nodes[i]] == col;

    Vec<PairI32> gain;
    gain.reserve(q - 1);
    I64 cur = 0;
    for (I32 i = q - 1; i > 0; --i) {
      I32 b = min(sub[i], m - sub[i]);
      cur += I64(b) * dist[i];
      gain.push_back({b, dist[i]});
      sub[par[i]] += sub[i];
    }

    sort(gain.rbegin(), gain.rend());
    I32 need = k[col] - 1;
    for (auto [b, cnt] : gain) {
      I32 take = min(need, cnt);
      cur  -= I64(b) * take;
      need -= take;
      if (!need) break;
    }
    ans[col] = cur;
  }

  OUT(ans);
}

//===----------------------------------------------------------------------===//
/* Main Function */

auto main() -> int {
#ifdef LOCAL
  Stopwatch timer;
#endif

  INT(T);
  FOR(T) solve();

  return 0;
}

//===----------------------------------------------------------------------===//
