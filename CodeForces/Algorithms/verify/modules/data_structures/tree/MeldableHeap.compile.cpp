#include "modules/data_structures/tree/MeldableHeap.hpp"

int main() {
  MeldableHeap<I32> heap;
  auto a = heap.push(MeldableHeap<I32>::null, 4);
  a = heap.push(a, 9);
  auto b = heap.singleton(7);
  const auto root = heap.meld(a, b);
  return heap.top(root) == 9 && heap.size(root) == 3 ? 0 : 1;
}
