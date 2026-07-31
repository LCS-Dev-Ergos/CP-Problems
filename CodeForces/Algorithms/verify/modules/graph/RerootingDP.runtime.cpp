#include "modules/graph/tree/RerootingDP.hpp"

#include <cassert>
#include <random>

using Edge = Graph<I64>::Edge;

/// @brief Subtree vertex count plus total distance, the canonical rerooting DP.
struct Node {
  I64 cnt;
  I64 sum;
};

namespace {

auto merge(Node a, Node b) -> Node { return {a.cnt + b.cnt, a.sum + b.sum}; }
auto lift(Node x, const Edge& e) -> Node { return {x.cnt, x.sum + x.cnt * e.weight}; }
auto fold(Node x, I32) -> Node { return {x.cnt + 1, x.sum}; }

} // namespace

int main() {
  std::mt19937 rng(20260804);
  FOR(iter, 120) {
    const I32 n = 1 + I32(rng() % 10);
    Graph<I64> g(n);
    Vec2D<I64> w(n, Vec<I64>(n, 0));
    FOR(v, 1, n) {
      const I32 p = I32(rng() % v);
      const I64 weight = I64(1 + rng() % 5);
      g.add_edge(p, v, weight);
      w[p][v] = w[v][p] = weight;
    }

    TreeCore<I64> tree(g);
    RerootingDP<Node> dp(tree, merge, lift, fold, Node{0, 0});

    // Brute force all-pairs distances over the tree.
    Vec2D<I64> d(n, Vec<I64>(n, infinity<I64>));
    FOR(i, n) d[i][i] = 0;
    FOR(i, n) FOR(j, n) {
      if (w[i][j] != 0) d[i][j] = w[i][j];
    }
    FOR(k, n) FOR(i, n) FOR(j, n) {
      if (d[i][k] + d[k][j] < d[i][j]) d[i][j] = d[i][k] + d[k][j];
    }

    FOR(v, n) {
      I64 want = 0;
      FOR(u, n) want += d[v][u];
      assert(dp[v].cnt == n);
      assert(dp[v].sum == want);
    }

    // get(v, root) is the subtree of v once the tree hangs from root.
    FOR(root, n) FOR(v, n) {
      I64 cnt = 0;
      FOR(u, n) cnt += (d[root][u] == d[root][v] + d[v][u]);
      assert(dp.get(v, root).cnt == cnt);
    }
  }

  // Forests: components must stay independent.
  Graph<I64> forest(5);
  forest.add_edge(0, 1, 2);
  forest.add_edge(2, 3, 3);
  forest.add_edge(3, 4, 4);
  TreeCore<I64> ft(forest);
  RerootingDP<Node> fdp(ft, merge, lift, fold, Node{0, 0});
  assert(fdp[0].cnt == 2 && fdp[0].sum == 2);
  assert(fdp[2].cnt == 3 && fdp[2].sum == 10);
  assert(fdp[3].cnt == 3 && fdp[3].sum == 7);

  // Single isolated vertex.
  Graph<I64> point(1);
  TreeCore<I64> pt(point);
  RerootingDP<Node> pdp(pt, merge, lift, fold, Node{0, 0});
  assert(pdp[0].cnt == 1 && pdp[0].sum == 0);
  return 0;
}
