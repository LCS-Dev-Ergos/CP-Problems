#include "modules/sequences/CartesianTree.hpp"

#include <cassert>
#include <random>

namespace {

auto inspect(const CartesianTree<I32>& tree, I32 v, const VecI32& a, VecI32& order)
    -> Pair<I32, I32> {
  I32 lo = v;
  I32 hi = v + 1;
  if (tree.left[v] != -1) {
    assert(tree.parent[tree.left[v]] == v);
    assert(a[v] <= a[tree.left[v]]);
    const auto [l, r] = inspect(tree, tree.left[v], a, order);
    lo = l;
    hi = std::max(hi, r);
  }
  order.push_back(v);
  if (tree.right[v] != -1) {
    assert(tree.parent[tree.right[v]] == v);
    assert(a[v] <= a[tree.right[v]]);
    const auto [l, r] = inspect(tree, tree.right[v], a, order);
    lo = std::min(lo, l);
    hi = r;
  }
  assert((tree.range(v) == Pair<I32, I32>{lo, hi}));
  return {lo, hi};
}

void check(const VecI32& a) {
  const CartesianTree<I32> tree(a);
  assert(tree.size() == isz(a));
  if (a.empty()) {
    assert(tree.empty() && tree.root == -1);
    return;
  }
  assert(tree.parent[tree.root] == -1);
  VecI32 order;
  inspect(tree, tree.root, a, order);
  VecI32 expected(isz(a));
  std::iota(all(expected), 0);
  assert(order == expected);
  const I32 mn = *std::min_element(all(a));
  assert(a[tree.root] == mn);
  assert(tree.root == I32(std::find(all(a), mn) - a.begin()));
}

}  // namespace

int main() {
  check({});
  check({7});
  check({3, 1, 4, 1, 5, 9, 2, 6});
  check({2, 2, 2, 2});

  std::mt19937 rng(20260812);
  FOR(n, 1, 61) {
    FOR(iter, 50) {
      VecI32 a(n);
      for (I32& x : a) x = I32(rng() % 17) - 8;
      check(a);
    }
  }
  return 0;
}
