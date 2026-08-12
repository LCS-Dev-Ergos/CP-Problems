#ifndef CP_MODULES_DATA_STRUCTURES_DYNAMIC_LAZY_SEG_TREE_HPP
#define CP_MODULES_DATA_STRUCTURES_DYNAMIC_LAZY_SEG_TREE_HPP

#include "../_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/// @brief Dynamic lazy segment tree on an arbitrary integer interval.
template <typename ActedMonoid, bool Persistent = false>
struct DynamicLazySegTree {
  using Value  = typename ActedMonoid::value_type;
  using Action = typename ActedMonoid::action_type;
  using Root = I32;

  struct Node {
    Root left  = -1;
    Root right = -1;
    Value value = ActedMonoid::value_identity();
    Action lazy = ActedMonoid::action_identity();
    char has_lazy = false;
  };

  I64 left_bound  = 0;
  I64 right_bound = 0;
  Function<Value(I64, I64)> default_value;
  Vec<Node> nodes;

  explicit DynamicLazySegTree(
      I64 left_bound_,
      I64 right_bound_,
      Function<Value(I64, I64)> default_value_ =
          [](I64, I64) -> Value { return ActedMonoid::value_identity(); })
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

  /// @brief Returns the current value at one point.
  [[nodiscard]] auto get(Root root, I64 idx) const -> Value {
    return query(root, idx, idx + 1);
  }

  /// @brief Half-open range query on [l, r).
  [[nodiscard]] auto query(Root root, I64 l, I64 r) const -> Value {
    my_assert(left_bound <= l && l <= r && r <= right_bound);
    return query_rec(root, left_bound, right_bound, l, r, ActedMonoid::action_identity(), false);
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

  /// @brief Applies one lazy action to [l, r). Returns the new root.
  [[nodiscard]] auto apply(Root root, I64 l, I64 r, const Action& action) -> Root {
    my_assert(left_bound <= l && l <= r && r <= right_bound);
    return apply_rec(root, left_bound, right_bound, l, r, action);
  }

  /// @brief Largest `r` such that `check(query(root, l, r))` holds.
  template <typename F>
  [[nodiscard]] auto max_right(Root root, I64 l, F check) const -> I64 {
    my_assert(left_bound <= l && l <= right_bound);
    my_assert(check(ActedMonoid::value_identity()));
    Value acc = ActedMonoid::value_identity();
    return max_right_rec(
        root,
        left_bound,
        right_bound,
        l,
        acc,
        check,
        ActedMonoid::action_identity(),
        false);
  }

  /// @brief Smallest `l` such that `check(query(root, l, r))` holds.
  template <typename F>
  [[nodiscard]] auto min_left(Root root, I64 r, F check) const -> I64 {
    my_assert(left_bound <= r && r <= right_bound);
    my_assert(check(ActedMonoid::value_identity()));
    Value acc = ActedMonoid::value_identity();
    return min_left_rec(
        root,
        left_bound,
        right_bound,
        r,
        acc,
        check,
        ActedMonoid::action_identity(),
        false);
  }

  /// @brief Enumerates materialized leaves after pushing accumulated actions.
  template <typename F>
  void enumerate(Root root, F f) const {
    enumerate_rec(root, left_bound, right_bound, ActedMonoid::action_identity(), false, f);
  }

private:
  [[nodiscard]] auto build_rec(const Vec<Value>& values, I64 l, I64 r) -> Root {
    if (l + 1 == r) return make_node(values[as<I32>(l)]);
    const I64 mid = (l + r) >> 1;
    const Root left  = build_rec(values, l, mid);
    const Root right = build_rec(values, mid, r);
    const Root root  = make_node(ActedMonoid::combine(nodes[left].value, nodes[right].value));
    nodes[root].left  = left;
    nodes[root].right = right;
    return root;
  }

  [[nodiscard]] auto value_of(Root root, I64 l, I64 r) const -> Value {
    return (root == -1 ? default_value(l, r) : nodes[root].value);
  }

  [[nodiscard]] auto make_node(const Value& value) -> Root {
    nodes.push_back(Node{.left  = -1,
                         .right = -1,
                         .value = value,
                         .lazy  = ActedMonoid::action_identity(),
                         .has_lazy = false});
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

  [[nodiscard]] auto compose(const Action& existing, const Action& fresh) const -> Action {
    return ActedMonoid::compose(existing, fresh);
  }

  [[nodiscard]] auto apply_value(
      const Value& value,
      const Action& action,
      I64 l,
      I64 r) const -> Value {
    return ActedMonoid::apply(value, action, as<I32>(r - l));
  }

  void apply_at(Root root, I64 l, I64 r, const Action& action) {
    nodes[root].value = apply_value(nodes[root].value, action, l, r);
    if (nodes[root].has_lazy) nodes[root].lazy = compose(nodes[root].lazy, action);
    else {
      nodes[root].lazy = action;
      nodes[root].has_lazy = true;
    }
  }

  void push(Root root, I64 l, I64 r) {
    if (root == -1 || !nodes[root].has_lazy || l + 1 == r) return;
    const I64 mid = (l + r) >> 1;
    nodes[root].left  = clone_or_create(nodes[root].left, l, mid);
    nodes[root].right = clone_or_create(nodes[root].right, mid, r);
    apply_at(nodes[root].left, l, mid, nodes[root].lazy);
    apply_at(nodes[root].right, mid, r, nodes[root].lazy);
    nodes[root].lazy = ActedMonoid::action_identity();
    nodes[root].has_lazy = false;
  }

  void pull(Root root, I64 l, I64 r) {
    const I64 mid = (l + r) >> 1;
    nodes[root].value = ActedMonoid::combine(
        value_of(nodes[root].left, l, mid),
        value_of(nodes[root].right, mid, r));
  }

  [[nodiscard]] auto set_rec(Root root, I64 l, I64 r, I64 idx, const Value& value) -> Root {
    root = clone_or_create(root, l, r);
    if (l + 1 == r) {
      nodes[root].value = value;
      nodes[root].lazy = ActedMonoid::action_identity();
      nodes[root].has_lazy = false;
      return root;
    }
    push(root, l, r);
    const I64 mid = (l + r) >> 1;
    if (idx < mid) nodes[root].left = set_rec(nodes[root].left, l, mid, idx, value);
    else nodes[root].right = set_rec(nodes[root].right, mid, r, idx, value);
    pull(root, l, r);
    return root;
  }

  [[nodiscard]] auto multiply_rec(Root root, I64 l, I64 r, I64 idx, const Value& value) -> Root {
    root = clone_or_create(root, l, r);
    if (l + 1 == r) {
      nodes[root].value = ActedMonoid::combine(nodes[root].value, value);
      nodes[root].lazy  = ActedMonoid::action_identity();
      nodes[root].has_lazy = false;
      return root;
    }
    push(root, l, r);
    const I64 mid = (l + r) >> 1;
    if (idx < mid) nodes[root].left = multiply_rec(nodes[root].left, l, mid, idx, value);
    else nodes[root].right = multiply_rec(nodes[root].right, mid, r, idx, value);
    pull(root, l, r);
    return root;
  }

  [[nodiscard]] auto apply_rec(Root root, I64 l, I64 r, I64 ql, I64 qr, const Action& action)
      -> Root {
    ql = std::max(ql, l);
    qr = std::min(qr, r);
    if (ql >= qr) return root;
    root = clone_or_create(root, l, r);
    if (ql == l && qr == r) {
      apply_at(root, l, r, action);
      return root;
    }
    push(root, l, r);
    const I64 mid = (l + r) >> 1;
    nodes[root].left  = apply_rec(nodes[root].left, l, mid, ql, qr, action);
    nodes[root].right = apply_rec(nodes[root].right, mid, r, ql, qr, action);
    pull(root, l, r);
    return root;
  }

  [[nodiscard]] auto query_rec(
      Root root,
      I64 l,
      I64 r,
      I64 ql,
      I64 qr,
      const Action& carry,
      bool has_carry) const -> Value {
    ql = std::max(ql, l);
    qr = std::min(qr, r);
    if (ql >= qr) return ActedMonoid::value_identity();

    Value segment = value_of(root, l, r);
    if (has_carry) segment = apply_value(segment, carry, l, r);

    if (ql == l && qr == r) {
      return segment;
    }

    Action child_carry = carry;
    bool has_child_carry = has_carry;
    if (root != -1 && nodes[root].has_lazy) {
      if (has_child_carry) child_carry = compose(child_carry, nodes[root].lazy);
      else {
        child_carry = nodes[root].lazy;
        has_child_carry = true;
      }
    }

    const I64 mid = (l + r) >> 1;
    const Root left  = (root == -1 ? -1 : nodes[root].left);
    const Root right = (root == -1 ? -1 : nodes[root].right);
    const Value left_value  = query_rec(left, l, mid, ql, qr, child_carry, has_child_carry);
    const Value right_value = query_rec(right, mid, r, ql, qr, child_carry, has_child_carry);
    return ActedMonoid::combine(left_value, right_value);
  }

  template <typename F>
  [[nodiscard]] auto max_right_rec(
      Root root,
      I64 l,
      I64 r,
      I64 ql,
      Value& acc,
      const F& check,
      const Action& carry,
      bool has_carry) const -> I64 {
    if (r <= ql) return right_bound;

    Value segment = value_of(root, l, r);
    if (has_carry) segment = apply_value(segment, carry, l, r);

    if (ql <= l) {
      const Value nxt = ActedMonoid::combine(acc, segment);
      if (check(nxt)) {
        acc = nxt;
        return right_bound;
      }
    }
    if (l + 1 == r) return l;

    Action child_carry = carry;
    bool has_child_carry = has_carry;
    if (root != -1 && nodes[root].has_lazy) {
      if (has_child_carry) child_carry = compose(child_carry, nodes[root].lazy);
      else {
        child_carry = nodes[root].lazy;
        has_child_carry = true;
      }
    }

    const I64 mid = (l + r) >> 1;
    const Root left  = (root == -1 ? -1 : nodes[root].left);
    const Root right = (root == -1 ? -1 : nodes[root].right);
    const I64 left_result = max_right_rec(left, l, mid, ql, acc, check, child_carry, has_child_carry);
    if (left_result < mid) return left_result;
    return max_right_rec(right, mid, r, ql, acc, check, child_carry, has_child_carry);
  }

  template <typename F>
  [[nodiscard]] auto min_left_rec(
      Root root,
      I64 l,
      I64 r,
      I64 qr,
      Value& acc,
      const F& check,
      const Action& carry,
      bool has_carry) const -> I64 {
    if (qr <= l) return left_bound;

    Value segment = value_of(root, l, r);
    if (has_carry) segment = apply_value(segment, carry, l, r);

    if (r <= qr) {
      const Value nxt = ActedMonoid::combine(segment, acc);
      if (check(nxt)) {
        acc = nxt;
        return left_bound;
      }
    }
    if (l + 1 == r) return r;

    Action child_carry = carry;
    bool has_child_carry = has_carry;
    if (root != -1 && nodes[root].has_lazy) {
      if (has_child_carry) child_carry = compose(child_carry, nodes[root].lazy);
      else {
        child_carry = nodes[root].lazy;
        has_child_carry = true;
      }
    }

    const I64 mid = (l + r) >> 1;
    const Root left  = (root == -1 ? -1 : nodes[root].left);
    const Root right = (root == -1 ? -1 : nodes[root].right);
    const I64 right_result = min_left_rec(right, mid, r, qr, acc, check, child_carry, has_child_carry);
    if (right_result > mid) return right_result;
    return min_left_rec(left, l, mid, qr, acc, check, child_carry, has_child_carry);
  }

  template <typename F>
  void enumerate_rec(
      Root root,
      I64 l,
      I64 r,
      const Action& carry,
      bool has_carry,
      F& f) const {
    if (root == -1) return;
    if (l + 1 == r) {
      Value value = nodes[root].value;
      if (has_carry) value = apply_value(value, carry, l, r);
      f(l, value);
      return;
    }
    Action child_carry = carry;
    bool has_child_carry = has_carry;
    if (nodes[root].has_lazy) {
      if (has_child_carry) child_carry = compose(child_carry, nodes[root].lazy);
      else {
        child_carry = nodes[root].lazy;
        has_child_carry = true;
      }
    }
    const I64 mid = (l + r) >> 1;
    enumerate_rec(nodes[root].left, l, mid, child_carry, has_child_carry, f);
    enumerate_rec(nodes[root].right, mid, r, child_carry, has_child_carry, f);
  }
};

#endif
