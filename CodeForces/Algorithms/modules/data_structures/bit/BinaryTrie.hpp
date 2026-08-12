#ifndef CP_MODULES_DATA_STRUCTURES_BIT_BINARY_TRIE_HPP
#define CP_MODULES_DATA_STRUCTURES_BIT_BINARY_TRIE_HPP

#include "../_Common.hpp"

/// @brief Binary trie multiset with xor-aware order statistics.
template <typename UInt = U32, I32 BIT_WIDTH = Limits<UInt>::digits>
struct BinaryTrie {
  static_assert(std::is_unsigned_v<UInt>, "BinaryTrie requires an unsigned integer type.");
  static_assert(0 < BIT_WIDTH && BIT_WIDTH <= Limits<UInt>::digits,
                "BinaryTrie BIT_WIDTH must fit inside the unsigned type.");

  struct Node {
    I32 child[2] = {-1, -1};
    I32 count = 0;
  };

  Vec<Node> nodes;

  BinaryTrie() { clear(); }

  /// @brief Clears the trie back to one empty root.
  void clear() {
    nodes.clear();
    nodes.eb();
  }

  /// @brief Returns the number of stored values, counting multiplicity.
  [[nodiscard]] auto size() const -> I32 { return nodes[0].count; }

  /// @brief Returns whether the trie is empty.
  [[nodiscard]] auto empty() const -> bool { return size() == 0; }

  /// @brief Inserts @p count occurrences of @p value.
  void insert(UInt value, I32 count = 1) {
    my_assert(count >= 0);
    if (count == 0) return;
    I32 cur = 0;
    nodes[cur].count += count;
    ROF(bit, BIT_WIDTH) {
      const I32 dir = (value >> bit) & UInt(1);
      if (nodes[cur].child[dir] == -1) {
        nodes[cur].child[dir] = isz(nodes);
        nodes.eb();
      }
      cur = nodes[cur].child[dir];
      nodes[cur].count += count;
    }
  }

  /// @brief Erases up to @p count occurrences of @p value and returns how many
  /// were removed.
  auto erase(UInt value, I32 count = 1) -> I32 {
    my_assert(count >= 0);
    if (count == 0) return 0;
    const I32 have = count_value(value);
    const I32 take = std::min(have, count);
    if (take == 0) return 0;
    I32 cur = 0;
    nodes[cur].count -= take;
    ROF(bit, BIT_WIDTH) {
      const I32 dir = (value >> bit) & UInt(1);
      cur = nodes[cur].child[dir];
      nodes[cur].count -= take;
    }
    return take;
  }

  /// @brief Returns how many times @p value is present.
  [[nodiscard]] auto count_value(UInt value) const -> I32 {
    I32 cur = 0;
    ROF(bit, BIT_WIDTH) {
      const I32 dir = (value >> bit) & UInt(1);
      cur = nodes[cur].child[dir];
      if (cur == -1) return 0;
    }
    return nodes[cur].count;
  }

  /// @brief Returns the k-th smallest value after xor with @p xor_mask.
  [[nodiscard]] auto kth(I32 k, UInt xor_mask = 0) const -> UInt {
    my_assert(0 <= k && k < size());
    I32 cur = 0;
    UInt value = 0;
    ROF(bit, BIT_WIDTH) {
      const I32 preferred = (xor_mask >> bit) & UInt(1);
      const I32 left = nodes[cur].child[preferred];
      const I32 left_count = left == -1 ? 0 : nodes[left].count;
      if (k < left_count) {
        cur = left;
      } else {
        k -= left_count;
        cur = nodes[cur].child[preferred ^ 1];
        value |= UInt(1) << bit;
      }
    }
    return value;
  }

  /// @brief Returns the minimum value after xor with @p xor_mask.
  [[nodiscard]] auto min_element(UInt xor_mask = 0) const -> UInt {
    my_assert(!empty());
    return kth(0, xor_mask);
  }

  /// @brief Returns the maximum value after xor with @p xor_mask.
  [[nodiscard]] auto max_element(UInt xor_mask = 0) const -> UInt {
    my_assert(!empty());
    return kth(size() - 1, xor_mask);
  }

  /// @brief Returns how many xor-shifted values lie in [0, upper).
  [[nodiscard]] auto prefix_count(UInt upper, UInt xor_mask = 0) const -> I32 {
    if (upper == 0 || empty()) return 0;
    I32 cur = 0;
    I32 res = 0;
    ROF(bit, BIT_WIDTH) {
      if (cur == -1) break;
      const I32 upper_bit = (upper >> bit) & UInt(1);
      const I32 xor_bit = (xor_mask >> bit) & UInt(1);
      if (upper_bit) {
        const I32 same = nodes[cur].child[xor_bit];
        if (same != -1) res += nodes[same].count;
        cur = nodes[cur].child[xor_bit ^ 1];
      } else {
        cur = nodes[cur].child[xor_bit];
      }
    }
    return res;
  }

  /// @brief Returns how many xor-shifted values lie in [lo, hi).
  [[nodiscard]] auto count_range(UInt lo, UInt hi, UInt xor_mask = 0) const -> I32 {
    if (hi <= lo) return 0;
    return prefix_count(hi, xor_mask) - prefix_count(lo, xor_mask);
  }

  /// @brief Enumerates values in sorted order, coalesced by multiplicity.
  template <typename F>
  void enumerate(F&& fn) const {
    enumerate_dfs(0, BIT_WIDTH - 1, 0, std::forward<F>(fn));
  }

 private:
  template <typename F>
  void enumerate_dfs(I32 node, I32 bit, UInt value, F&& fn) const {
    if (node == -1 || nodes[node].count == 0) return;
    if (bit < 0) {
      fn(value, nodes[node].count);
      return;
    }
    enumerate_dfs(nodes[node].child[0], bit - 1, value, std::forward<F>(fn));
    enumerate_dfs(nodes[node].child[1], bit - 1, value | (UInt(1) << bit), std::forward<F>(fn));
  }
};

#endif
