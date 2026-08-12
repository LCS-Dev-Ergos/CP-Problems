#include "modules/data_structures/bit/BinaryTrie.hpp"

#include <algorithm>
#include <cassert>

int main() {
  BinaryTrie<U32, 6> trie;
  trie.insert(5);
  trie.insert(9, 2);
  trie.insert(17);
  trie.insert(3);

  assert(trie.size() == 5);
  assert(trie.count_value(9) == 2);
  assert(trie.min_element() == 3);
  assert(trie.max_element() == 17);
  assert(trie.kth(0) == 3);
  assert(trie.kth(2) == 9);

  const U32 xor_mask = 10;
  Vec<U32> shifted = {5 ^ xor_mask, 9 ^ xor_mask, 9 ^ xor_mask, 17 ^ xor_mask, 3 ^ xor_mask};
  std::ranges::sort(shifted);
  FOR(i, isz(shifted)) assert(trie.kth(i, xor_mask) == shifted[i]);
  assert(trie.prefix_count(10, xor_mask) == 3);
  assert(trie.count_range(8, 16, xor_mask) == 2);

  Vec<Pair<U32, I32>> enumerated;
  trie.enumerate([&](U32 value, I32 count) { enumerated.eb(value, count); });
  const Vec<Pair<U32, I32>> expected = {{3, 1}, {5, 1}, {9, 2}, {17, 1}};
  assert(enumerated == expected);

  assert(trie.erase(9) == 1);
  assert(trie.count_value(9) == 1);
  assert(trie.erase(9, 5) == 1);
  assert(trie.count_value(9) == 0);
  assert(trie.size() == 3);
  return 0;
}
