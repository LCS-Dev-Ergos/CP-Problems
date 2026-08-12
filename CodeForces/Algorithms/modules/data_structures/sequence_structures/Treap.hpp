#ifndef CP_MODULES_DATA_STRUCTURES_TREAP_HPP
#define CP_MODULES_DATA_STRUCTURES_TREAP_HPP

#include "../_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/// @brief Randomized treap supporting duplicates and order statistics.
struct Treap {
  /// @brief Treap node storing key, multiplicity and subtree aggregates.
  struct Node {
    I64 key, priority;
    I32 left, right, size, cnt;
    I64 sum;

    Node(I64 k, I64 p) : key(k), priority(p), left(-1), right(-1), size(1), cnt(1), sum(k) {}
  };

  Vec<Node> nodes;
  I32 root;

  /// @brief Constructs empty treap.
  Treap() : root(-1) { nodes.reserve(200000); }

  static auto rng() -> std::mt19937_64& {
    static thread_local std::mt19937_64 gen(as<U64>(std::chrono::steady_clock::now().time_since_epoch().count()));
    return gen;
  }

  static auto next_priority() -> I64 {
    static thread_local std::uniform_int_distribution<I64> dist(Limits<I64>::lowest(), Limits<I64>::max());
    return dist(rng());
  }

  I32 get_size(I32 t) { return t == -1 ? 0 : nodes[t].size; }
  I64 get_sum(I32 t) { return t == -1 ? 0 : nodes[t].sum; }

  /// @brief Recomputes subtree aggregates for node @p t.
  void update(I32 t) {
    if (t == -1) return;
    nodes[t].size = nodes[t].cnt + get_size(nodes[t].left) + get_size(nodes[t].right);
    nodes[t].sum = nodes[t].key * nodes[t].cnt + get_sum(nodes[t].left) + get_sum(nodes[t].right);
  }

  /// @brief Splits tree into keys < key and keys >= key.
  void split_lt(I32 t, I64 key, I32& l, I32& r) {
    if (t == -1) {
      l = r = -1;
      return;
    }
    if (nodes[t].key < key) {
      split_lt(nodes[t].right, key, nodes[t].right, r);
      l = t;
    } else {
      split_lt(nodes[t].left, key, l, nodes[t].left);
      r = t;
    }
    update(t);
  }

  /// @brief Splits tree into keys <= key and keys > key.
  void split_le(I32 t, I64 key, I32& l, I32& r) {
    if (t == -1) {
      l = r = -1;
      return;
    }
    if (nodes[t].key <= key) {
      split_le(nodes[t].right, key, nodes[t].right, r);
      l = t;
    } else {
      split_le(nodes[t].left, key, l, nodes[t].left);
      r = t;
    }
    update(t);
  }

  /// @brief Merges two treaps preserving BST and heap properties.
  void merge(I32& t, I32 l, I32 r) {
    if (l == -1 || r == -1) {
      t = l == -1 ? r : l;
      return;
    }
    if (nodes[l].priority > nodes[r].priority) {
      merge(nodes[l].right, nodes[l].right, r);
      t = l;
    } else {
      merge(nodes[r].left, l, nodes[r].left);
      t = r;
    }
    update(t);
  }

  /// @brief Inserts one occurrence of @p key.
  void insert(I64 key) {
    I32 l, m, r;
    split_lt(root, key, l, m);
    split_le(m, key, m, r);
    if (m == -1) {
      m = sz(nodes);
      nodes.eb(key, next_priority());
    } else {
      nodes[m].cnt++;
      update(m);
    }
    merge(root, l, m);
    merge(root, root, r);
  }

  /// @brief Removes one occurrence of @p key if present.
  void erase(I64 key) {
    I32 l, m, r;
    split_lt(root, key, l, m);
    split_le(m, key, m, r);

    if (m != -1) {
      if (nodes[m].cnt > 1) {
        nodes[m].cnt--;
        update(m);
      } else {
        merge(m, nodes[m].left, nodes[m].right);
      }
    }

    merge(root, l, m);
    merge(root, root, r);
  }

  /// @brief Returns k-th smallest key (0-indexed).
  I64 kth_element(I32 k) {
    I32 t = root;
    while (t != -1) {
      I32 left_size = get_size(nodes[t].left);
      if (k == left_size) return nodes[t].key;
      if (k < left_size) {
        t = nodes[t].left;
      } else {
        if (k < left_size + nodes[t].cnt) return nodes[t].key;
        k -= left_size + nodes[t].cnt;
        t = nodes[t].right;
      }
    }
    return -1;
  }
};

#endif
