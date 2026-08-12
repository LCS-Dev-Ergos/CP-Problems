#include "modules/graph/RangeToRangeGraph.hpp"
#include "modules/graph/DijkstraSparse.hpp"
#include "templates/core/MinMax.hpp"

#include <cassert>
#include <random>

namespace {

/// Dense O(n^2) Dijkstra used as the reference shortest path.
auto dense_dijkstra(const Vec2D<I64>& dense, I32 src) -> VecI64 {
  const I32 n = isz(dense);
  VecI64 dist(n, infinity<I64>);
  Vec<bool> done(n, false);
  dist[src] = 0;
  FOR(round, n) {
    I32 u = -1;
    FOR(v, n) {
      if (!done[v] && (u == -1 || dist[v] < dist[u])) u = v;
    }
    if (u == -1 || dist[u] >= infinity<I64>) break;
    done[u] = true;
    FOR(v, n) {
      if (dense[u][v] < infinity<I64>) chmin(dist[v], dist[u] + dense[u][v]);
    }
  }
  return dist;
}

}  // namespace

int main() {
  std::mt19937 rng(20260809);

  // Every edge kind, cross-checked against the fully expanded dense graph.
  FOR(iter, 300) {
    const I32 n = 1 + I32(rng() % 12);
    RangeToRangeGraph<I64> builder(n);
    Vec2D<I64> dense(n, VecI64(n, infinity<I64>));
    assert(builder.size() == 3 * n);
    I32 expected_nodes = 3 * n;

    const I32 ops = 1 + I32(rng() % 12);
    FOR(op, ops) {
      const I32 a0 = I32(rng() % U32(n + 1));
      const I32 a1 = I32(rng() % U32(n + 1));
      const I32 fl = std::min(a0, a1);
      const I32 fr = std::max(a0, a1);
      const I32 b0 = I32(rng() % U32(n + 1));
      const I32 b1 = I32(rng() % U32(n + 1));
      const I32 tl = std::min(b0, b1);
      const I32 tr = std::max(b0, b1);
      const I32 u = I32(rng() % U32(n));
      const I32 v = I32(rng() % U32(n));
      const I64 w = I64(rng() % 20);

      switch (rng() % 4) {
        case 0:
          builder.add(u, v, w);
          chmin(dense[u][v], w);
          break;
        case 1:
          builder.add_to_range(u, tl, tr, w);
          FOR(j, tl, tr) chmin(dense[u][j], w);
          break;
        case 2:
          builder.add_from_range(fl, fr, v, w);
          FOR(i, fl, fr) chmin(dense[i][v], w);
          break;
        default:
          builder.add_range_to_range(fl, fr, tl, tr, w);
          FOR(i, fl, fr) FOR(j, tl, tr) chmin(dense[i][j], w);
          // A hub is only spent when neither side collapsed to a point.
          if (fr - fl > 1 && tr - tl > 1) ++expected_nodes;
          break;
      }
    }

    assert(builder.size() == expected_nodes);
    const Graph<I64> g = builder.build();
    assert(g.n == expected_nodes);

    const I32 src = I32(rng() % U32(n));
    const VecI64 got = dijkstra_sparse(g, src).dist;
    const VecI64 want = dense_dijkstra(dense, src);
    FOR(v, n) assert(got[v] == want[v]);
  }

  // Auxiliary vertices must not create paths of their own: with no edges added
  // at all, nothing outside the source is reachable.
  {
    const I32 n = 16;
    RangeToRangeGraph<I64> builder(n);
    const Graph<I64> g = builder.build();
    const VecI64 dist = dijkstra_sparse(g, 0).dist;
    assert(dist[0] == 0);
    FOR(v, 1, n) assert(dist[v] >= infinity<I64>);
  }

  // Degenerate sizes.
  {
    RangeToRangeGraph<I64> one(1);
    one.add_to_range(0, 0, 1, 5);
    const Graph<I64> g = one.build();
    const VecI64 dist = dijkstra_sparse(g, 0).dist;
    assert(dist[0] == 0);

    RangeToRangeGraph<I64> empty(0);
    assert(empty.size() == 0);
    assert(empty.build().n == 0);
  }

  // Scale: from every i one step reaches [i+1, i+K), so the distance from 0 is
  // the number of blocks of K-1 steps needed to cover v.
  {
    const I32 n = 100000;
    const I32 k = 7;
    RangeToRangeGraph<I64> builder(n);
    FOR(i, n) {
      const I32 r = std::min(n, i + k);
      if (i + 1 < r) builder.add_to_range(i, i + 1, r, 1);
    }
    const Graph<I64> g = builder.build();
    const VecI64 dist = dijkstra_sparse(g, 0).dist;
    FOR(v, n) assert(dist[v] == I64((v + k - 2) / (k - 1)));
  }

  // Scale, range to range: layer i covers [i*B, (i+1)*B) and reaches the whole
  // next layer at cost 1, so the distance is the layer index. Nothing ever
  // returns to layer 0, which leaves its other members unreachable.
  {
    const I32 block = 100;
    const I32 layers = 500;
    const I32 n = block * layers;
    RangeToRangeGraph<I64> builder(n);
    FOR(i, layers - 1) {
      builder.add_range_to_range(i * block, (i + 1) * block, (i + 1) * block, (i + 2) * block, 1);
    }
    assert(builder.size() == 3 * n + (layers - 1));
    const Graph<I64> g = builder.build();
    const VecI64 dist = dijkstra_sparse(g, 0).dist;
    assert(dist[0] == 0);
    FOR(v, 1, block) assert(dist[v] >= infinity<I64>);
    FOR(v, block, n) assert(dist[v] == I64(v / block));
  }

  return 0;
}
