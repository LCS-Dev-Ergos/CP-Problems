#ifndef CP_MODULES_DATA_STRUCTURES_TREE_MELDABLE_HEAP_HPP
#define CP_MODULES_DATA_STRUCTURES_TREE_MELDABLE_HEAP_HPP

#include "../_Common.hpp"

/**
 * @file MeldableHeap.hpp
 * @brief Mutable leftist heap forest with O(log N) meld.
 *
 * @details Roots are integer handles into a vector pool. meld() and pop()
 * consume their input roots: old roots must not be used afterwards.
 */
template <typename T, typename Compare = std::less<T>>
struct MeldableHeap {
  using Root = I32;
  static constexpr Root null = -1;

private:
  struct Node {
    T value;
    Root left = null;
    Root right = null;
    I32 rank = 1;
    I32 size = 1;
  };

  Vec<Node> nodes;
  Compare comp;

  [[nodiscard]] auto rank(Root root) const -> I32 {
    return root == null ? 0 : nodes[root].rank;
  }

public:
  explicit MeldableHeap(Compare comp_ = {}) : comp(comp_) {}

  [[nodiscard]] auto singleton(const T& x) -> Root {
    nodes.push_back(Node{x});
    return isz(nodes) - 1;
  }

  [[nodiscard]] auto meld(Root a, Root b) -> Root {
    if (a == null) return b;
    if (b == null) return a;
    if (comp(nodes[a].value, nodes[b].value)) std::swap(a, b);
    nodes[a].right = meld(nodes[a].right, b);
    if (rank(nodes[a].left) < rank(nodes[a].right)) {
      std::swap(nodes[a].left, nodes[a].right);
    }
    nodes[a].rank = rank(nodes[a].right) + 1;
    nodes[a].size = 1 + size(nodes[a].left) + size(nodes[a].right);
    return a;
  }

  [[nodiscard]] auto push(Root root, const T& x) -> Root {
    return meld(root, singleton(x));
  }

  [[nodiscard]] auto pop(Root root) -> Root {
    my_assert(root != null);
    return meld(nodes[root].left, nodes[root].right);
  }

  [[nodiscard]] auto top(Root root) const -> const T& {
    my_assert(root != null);
    return nodes[root].value;
  }

  [[nodiscard]] auto size(Root root) const -> I32 {
    return root == null ? 0 : nodes[root].size;
  }

  [[nodiscard]] auto empty(Root root) const -> bool { return root == null; }
  [[nodiscard]] auto allocated() const -> I32 { return isz(nodes); }

  void clear() { nodes.clear(); }
};

#endif
