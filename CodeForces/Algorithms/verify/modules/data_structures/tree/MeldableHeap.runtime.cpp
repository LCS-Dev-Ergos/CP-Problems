#include "modules/data_structures/tree/MeldableHeap.hpp"

#include <cassert>
#include <random>

int main() {
  MeldableHeap<I32> heap;
  auto a = MeldableHeap<I32>::null;
  auto b = MeldableHeap<I32>::null;
  PriorityQueue<I32> expected;
  std::mt19937 rng(20260812);

  FOR(i, 5000) {
    const I32 x = I32(rng() % 100000) - 50000;
    if (i & 1) a = heap.push(a, x);
    else b = heap.push(b, x);
    expected.push(x);
  }
  auto root = heap.meld(a, b);
  assert(heap.size(root) == isz(expected));
  while (!expected.empty()) {
    assert(heap.top(root) == expected.top());
    root = heap.pop(root);
    expected.pop();
  }
  assert(heap.empty(root));

  MeldableHeap<I32, std::greater<I32>> min_heap;
  auto mn = MeldableHeap<I32, std::greater<I32>>::null;
  for (const I32 x : {8, 3, 5, 1, 7}) mn = min_heap.push(mn, x);
  for (const I32 x : {1, 3, 5, 7, 8}) {
    assert(min_heap.top(mn) == x);
    mn = min_heap.pop(mn);
  }
  return 0;
}
