#include "modules/data_structures/bit/BinaryTrie.hpp"

int main() {
  BinaryTrie<U64, 20> trie;
  trie.insert(7);
  trie.insert(12);
  return trie.max_element(3) >= trie.min_element(3) ? 0 : 1;
}
