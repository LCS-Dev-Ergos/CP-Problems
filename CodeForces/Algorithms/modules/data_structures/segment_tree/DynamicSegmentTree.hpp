#ifndef CP_MODULES_DATA_STRUCTURES_DYNAMIC_SEGMENT_TREE_HPP
#define CP_MODULES_DATA_STRUCTURES_DYNAMIC_SEGMENT_TREE_HPP

#include "../_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/// @brief Dynamic segment tree on an arbitrary integer interval.
template <typename Monoid, bool Persistent = false>
struct DynamicSegmentTree {
  using Value = typename Monoid::value_type;
  using Root = I32;

  struct Node {
    Root left = -1;
    Root right = -1;
    Value value = Monoid::identity();
  };

  I64 left_bound = 0;
  I64 right_bound = 0;
  Function<Value(I64, I64)> default_value;
  Vec<Node> nodes;

  explicit DynamicSegmentTree(
      I64 left_bound_,
      I64 right_bound_,
      Function<Value(I64, I64)> default_value_ =
          [](I64, I64) -> Value { return Monoid::identity(); })
      : left_bound(left_bound_),
        right_bound(right_bound_),
        default_value(std::move(default_value_)) {}

  /// @brief Returns an empty root representing the default segment.
  [[nodiscard]] static constexpr auto new_root() -> Root { return -1; }

  /// @brief Builds a full tree from a dense vector. Requires [0, n) as interval.
  [[nodiscard]] auto build(const Vec<Value>& values) -> Root {
    my_assert(left_bound == 0 && right_bound == isz(values));
    return build_rec(values, left_bound, right_bound);
  }

  /// @brief Returns the value at one point.
  [[nodiscard]] auto get(Root root, I64 idx) const -> Value {
    return query(root, idx, idx + 1);
  }

  /// @brief Half-open range query on [l, r).
  [[nodiscard]] auto query(Root root, I64 l, I64 r) const -> Value {
    my_assert(left_bound <= l && l <= r && r <= right_bound);
    Value acc = Monoid::identity();
    query_rec(root, left_bound, right_bound, l, r, acc);
    return acc;
  }

  [[nodiscard]] auto prod(Root root, I64 l, I64 r) const -> Value {
    return query(root, l, r);
  }

  /// @brief Aggregate over the full managed interval.
  [[nodiscard]] auto prod_all(Root root) const -> Value {
    return value_of(root, left_bound, right_bound);
  }

  /// @brief Point assignment. Returns the new root.
  [[nodiscard]] auto set(Root root, I64 idx, const Value& value) -> Root {
    my_assert(left_bound <= idx && idx < right_bound);
    return set_rec(root, left_bound, right_bound, idx, value);
  }

  /// @brief Replaces one point with `combine(current, value)`.
  [[nodiscard]] auto multiply(Root root, I64 idx, const Value& value) -> Root {
    my_assert(left_bound <= idx && idx < right_bound);
    return multiply_rec(root, left_bound, right_bound, idx, value);
  }

  /// @brief Largest `r` such that `check(query(root, l, r))` holds.
  template <typename F>
  [[nodiscard]] auto max_right(Root root, I64 l, F check) const -> I64 {
    my_assert(left_bound <= l && l <= right_bound);
    my_assert(check(Monoid::identity()));
    Value acc = Monoid::identity();
    return max_right_rec(root, left_bound, right_bound, l, acc, check);
  }

  /// @brief Smallest `l` such that `check(query(root, l, r))` holds.
  template <typename F>
  [[nodiscard]] auto min_left(Root root, I64 r, F check) const -> I64 {
    my_assert(left_bound <= r && r <= right_bound);
    my_assert(check(Monoid::identity()));
    Value acc = Monoid::identity();
    return min_left_rec(root, left_bound, right_bound, r, acc, check);
  }

  /// @brief Enumerates only the materialized leaves as `(index, value)`.
  template <typename F>
  void enumerate(Root root, F f) const {
    enumerate_rec(root, left_bound, right_bound, f);
  }

private:
  [[nodiscard]] auto build_rec(const Vec<Value>& values, I64 l, I64 r) -> Root {
    if (l + 1 == r) return make_node(values[as<I32>(l)]);
    const I64 mid = (l + r) >> 1;
    const Root left = build_rec(values, l, mid);
    const Root right = build_rec(values, mid, r);
    const Root root = make_node(Monoid::combine(nodes[left].value, nodes[right].value));
    nodes[root].left = left;
    nodes[root].right = right;
    return root;
  }

  [[nodiscard]] auto value_of(Root root, I64 l, I64 r) const -> Value {
    return (root == -1 ? default_value(l, r) : nodes[root].value);
  }

  [[nodiscard]] auto make_node(const Value& value) -> Root {
    nodes.push_back(Node{.left = -1, .right = -1, .value = value});
    return isz(nodes) - 1;
  }

  [[nodiscard]] auto make_default_node(I64 l, I64 r) -> Root {
    return make_node(default_value(l, r));
  }

  [[nodiscard]] auto clone_or_create(Root root, I64 l, I64 r) -> Root {
    if (root == -1) return make_default_node(l, r);
    if constexpr (!Persistent) return root;
    nodes.push_back(nodes[root]);
    return isz(nodes) - 1;
  }

  [[nodiscard]] auto set_rec(Root root, I64 l, I64 r, I64 idx, const Value& value) -> Root {
    root = clone_or_create(root, l, r);
    if (l + 1 == r) {
      nodes[root].value = value;
      return root;
    }
    const I64 mid = (l + r) >> 1;
    if (idx < mid) nodes[root].left = set_rec(nodes[root].left, l, mid, idx, value);
    else nodes[root].right = set_rec(nodes[root].right, mid, r, idx, value);
    nodes[root].value = Monoid::combine(
        value_of(nodes[root].left, l, mid),
        value_of(nodes[root].right, mid, r));
    return root;
  }

  [[nodiscard]] auto multiply_rec(Root root, I64 l, I64 r, I64 idx, const Value& value) -> Root {
    root = clone_or_create(root, l, r);
    if (l + 1 == r) {
      nodes[root].value = Monoid::combine(nodes[root].value, value);
      return root;
    }
    const I64 mid = (l + r) >> 1;
    if (idx < mid) nodes[root].left = multiply_rec(nodes[root].left, l, mid, idx, value);
    else nodes[root].right = multiply_rec(nodes[root].right, mid, r, idx, value);
    nodes[root].value = Monoid::combine(
        value_of(nodes[root].left, l, mid),
        value_of(nodes[root].right, mid, r));
    return root;
  }

  void query_rec(Root root, I64 l, I64 r, I64 ql, I64 qr, Value& acc) const {
    ql = std::max(ql, l);
    qr = std::min(qr, r);
    if (ql >= qr) return;
    if (root == -1) {
      acc = Monoid::combine(acc, default_value(ql, qr));
      return;
    }
    if (ql == l && qr == r) {
      acc = Monoid::combine(acc, nodes[root].value);
      return;
    }
    const I64 mid = (l + r) >> 1;
    query_rec(nodes[root].left, l, mid, ql, qr, acc);
    query_rec(nodes[root].right, mid, r, ql, qr, acc);
  }

  template <typename F>
  [[nodiscard]] auto max_right_rec(
      Root root,
      I64 l,
      I64 r,
      I64 ql,
      Value& acc,
      const F& check) const -> I64 {
    if (r <= ql) return right_bound;
    const Value segment = value_of(root, l, r);
    if (ql <= l) {
      const Value nxt = Monoid::combine(acc, segment);
      if (check(nxt)) {
        acc = nxt;
        return right_bound;
      }
    }
    if (l + 1 == r) return l;
    const I64 mid = (l + r) >> 1;
    const Root left = (root == -1 ? -1 : nodes[root].left);
    const Root right = (root == -1 ? -1 : nodes[root].right);
    const I64 left_result = max_right_rec(left, l, mid, ql, acc, check);
    if (left_result < mid) return left_result;
    return max_right_rec(right, mid, r, ql, acc, check);
  }

  template <typename F>
  [[nodiscard]] auto min_left_rec(
      Root root,
      I64 l,
      I64 r,
      I64 qr,
      Value& acc,
      const F& check) const -> I64 {
    if (qr <= l) return left_bound;
    const Value segment = value_of(root, l, r);
    if (r <= qr) {
      const Value nxt = Monoid::combine(segment, acc);
      if (check(nxt)) {
        acc = nxt;
        return left_bound;
      }
    }
    if (l + 1 == r) return r;
    const I64 mid = (l + r) >> 1;
    const Root left = (root == -1 ? -1 : nodes[root].left);
    const Root right = (root == -1 ? -1 : nodes[root].right);
    const I64 right_result = min_left_rec(right, mid, r, qr, acc, check);
    if (right_result > mid) return right_result;
    return min_left_rec(left, l, mid, qr, acc, check);
  }

  template <typename F>
  void enumerate_rec(Root root, I64 l, I64 r, F& f) const {
    if (root == -1) return;
    if (l + 1 == r) {
      f(l, nodes[root].value);
      return;
    }
    const I64 mid = (l + r) >> 1;
    enumerate_rec(nodes[root].left, l, mid, f);
    enumerate_rec(nodes[root].right, mid, r, f);
  }
};

#endif
