#ifndef CP_MODULES_SEQUENCES_CARTESIAN_TREE_HPP
#define CP_MODULES_SEQUENCES_CARTESIAN_TREE_HPP

#include "_Common.hpp"

/**
 * @file CartesianTree.hpp
 * @brief Stable Cartesian tree in O(N), with subtree intervals.
 *
 * @details The in-order traversal is the input order. With the default
 * comparator the tree is a min-heap; equal values keep the leftmost index
 * above the later one.
 */
template <typename T, typename Compare = std::less<T>>
struct CartesianTree {
  I32 root = -1;
  VecI32 parent;
  VecI32 left;
  VecI32 right;
  VecI32 range_l;
  VecI32 range_r;

  CartesianTree() = default;
  explicit CartesianTree(const Vec<T>& a, Compare comp = {}) { build(a, comp); }

  void build(const Vec<T>& a, Compare comp = {}) {
    const I32 n = isz(a);
    parent.assign(n, -1);
    left.assign(n, -1);
    right.assign(n, -1);
    range_l.resize(n);
    range_r.resize(n);
    root = -1;
    if (n == 0) return;

    VecI32 st;
    st.reserve(n);
    FOR(i, n) {
      I32 last = -1;
      while (!st.empty() && comp(a[i], a[st.back()])) {
        last = st.back();
        st.pop_back();
      }
      if (!st.empty()) {
        parent[i] = st.back();
        right[st.back()] = i;
      }
      if (last != -1) {
        parent[last] = i;
        left[i] = last;
      }
      st.push_back(i);
    }
    root = st.front();

    VecI32 order;
    order.reserve(n);
    st.assign(1, root);
    while (!st.empty()) {
      const I32 v = st.back();
      st.pop_back();
      order.push_back(v);
      if (left[v] != -1) st.push_back(left[v]);
      if (right[v] != -1) st.push_back(right[v]);
    }
    FOR_R(i, n) {
      const I32 v = order[i];
      range_l[v] = left[v] == -1 ? v : range_l[left[v]];
      range_r[v] = right[v] == -1 ? v + 1 : range_r[right[v]];
    }
  }

  [[nodiscard]] auto size() const -> I32 { return isz(parent); }
  [[nodiscard]] auto empty() const -> bool { return parent.empty(); }
  [[nodiscard]] auto range(I32 v) const -> Pair<I32, I32> {
    my_assert(0 <= v && v < size());
    return {range_l[v], range_r[v]};
  }
};

#endif
