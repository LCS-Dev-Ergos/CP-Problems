#ifndef CP_MODULES_DATA_STRUCTURES_SEG_TREE_BEATS_HPP
#define CP_MODULES_DATA_STRUCTURES_SEG_TREE_BEATS_HPP

#include "../_Common.hpp"
#include "modules/algebra/ActedMonoid.hpp"

/**
 * @brief Lazy segment tree that may refuse an action and recurse instead.
 *
 * @details Segment tree beats extends the lazy segment tree to actions that are
 * not summarisable on every node, such as @c chmin. The value type therefore
 * carries one extra @c bool @c fail member, and @c ActedMonoid::apply sets it
 * when the action cannot be applied to the node in constant time. Whenever a
 * node reports a failure the tree pushes the pending tag one level down and
 * recomputes the node from its children, which pays for the refusal by
 * descending only where the action really is ambiguous.
 *
 * A conforming acted monoid has to satisfy three extra rules on top of the
 * plain @ref ActedLazySegTree contract:
 *
 * - the value type exposes @c bool @c fail, cleared by @c value_identity and
 *   by every @c combine result;
 * - @c apply never sets @c fail on a segment of length one, since a single
 *   element is always representable;
 * - @c apply is only ever called on a value whose @c fail is already clear.
 *
 * The amortised cost of the classical chmin/chmax/add tree is
 * O(n log^2 n) for a mixed sequence, and O(n log n) when only one of the two
 * clamping directions is used. See @ref BeatsSumMinMax for that instantiation.
 */
template <typename ActedMonoid>
struct SegTreeBeats {
  using Value = typename ActedMonoid::value_type;
  using Action = typename ActedMonoid::action_type;

  I32 n = 0;
  I32 log = 0;
  I32 size = 1;
  Vec<Value> tree;
  Vec<Action> lazy;
  Vec<char> has_lazy;
  VecI32 seg_len;

  SegTreeBeats() = default;
  explicit SegTreeBeats(I32 size_) { build(size_); }

  template <typename F>
  SegTreeBeats(I32 size_, F init) {
    build(size_, init);
  }

  explicit SegTreeBeats(const Vec<Value>& values) { build(values); }

  /// @brief Initializes a tree of size @p size_ with the value identity.
  void build(I32 size_) {
    build(size_, [](I32) -> Value { return ActedMonoid::value_identity(); });
  }

  /// @brief Initializes a tree from a vector of leaf values.
  void build(const Vec<Value>& values) {
    build(isz(values), [&](I32 i) -> Value { return values[i]; });
  }

  /// @brief Initializes a tree from a leaf generator.
  template <typename F>
  void build(I32 size_, F init) {
    n = std::max<I32>(size_, 0);
    log = 0;
    size = 1;
    while (size < std::max<I32>(n, 1)) {
      size <<= 1;
      ++log;
    }
    tree.assign(size << 1, ActedMonoid::value_identity());
    lazy.assign(size, ActedMonoid::action_identity());
    has_lazy.assign(size, false);
    seg_len.assign(size << 1, 1);
    for (I32 i = size - 1; i >= 1; --i) {
      seg_len[i] = seg_len[i << 1] + seg_len[i << 1 | 1];
    }
    FOR(i, n) tree[size + i] = init(i);
    for (I32 i = size - 1; i >= 1; --i) {
      pull(i);
    }
  }

  /// @brief Point assignment after pushing pending actions on the access path.
  void set(I32 idx, const Value& value) {
    my_assert(0 <= idx && idx < n);
    idx += size;
    push_path(idx);
    tree[idx] = value;
    rebuild_path(idx);
  }

  /// @brief Returns the current value at one point.
  [[nodiscard]] auto get(I32 idx) -> Value {
    my_assert(0 <= idx && idx < n);
    idx += size;
    push_path(idx);
    return tree[idx];
  }

  /// @brief Materializes the full current leaf array.
  [[nodiscard]] auto get_all() -> Vec<Value> {
    FOR(i, 1, size) push(i);
    return {tree.begin() + size, tree.begin() + size + n};
  }

  /// @brief Half-open range query on [l, r).
  [[nodiscard]] auto query(I32 l, I32 r) -> Value {
    my_assert(0 <= l && l <= r && r <= n);
    if (l == r) return ActedMonoid::value_identity();
    l += size;
    r += size;
    push_path(l);
    push_path(r - 1);
    Value left = ActedMonoid::value_identity();
    Value right = ActedMonoid::value_identity();
    while (l < r) {
      if (l & 1) left = ActedMonoid::combine(left, tree[l++]);
      if (r & 1) right = ActedMonoid::combine(tree[--r], right);
      l >>= 1;
      r >>= 1;
    }
    return ActedMonoid::combine(left, right);
  }

  [[nodiscard]] auto prod(I32 l, I32 r) -> Value { return query(l, r); }

  /// @brief Returns the aggregate of the whole array.
  [[nodiscard]] auto query_all() const -> Value { return tree[1]; }

  [[nodiscard]] auto prod_all() const -> Value { return tree[1]; }

  /// @brief Applies one lazy action to the half-open range [l, r).
  void apply(I32 l, I32 r, const Action& action) {
    my_assert(0 <= l && l <= r && r <= n);
    if (l == r) return;
    l += size;
    r += size;
    push_path(l);
    push_path(r - 1);
    const I32 l0 = l;
    const I32 r0 = r;
    while (l < r) {
      if (l & 1) apply_node(l++, action);
      if (r & 1) apply_node(--r, action);
      l >>= 1;
      r >>= 1;
    }
    for (I32 h = 1; h <= log; ++h) {
      if (((l0 >> h) << h) != l0) pull(l0 >> h);
      if (((r0 >> h) << h) != r0) pull((r0 - 1) >> h);
    }
  }

  /// @brief Alias matching the `LazySegTree::update(l, r, tag)` surface.
  void update(I32 l, I32 r, const Action& action) { apply(l, r, action); }

private:
  /// @brief Recomputes one internal node from its children.
  void pull(I32 idx) {
    tree[idx] = ActedMonoid::combine(tree[idx << 1], tree[idx << 1 | 1]);
  }

  /**
   * @brief Applies one action to one node, recursing when the node refuses it.
   *
   * @details The tag is composed before the refusal is handled, so the push
   * that follows carries the action that could not be summarised down to the
   * children, where it is retried on shorter segments.
   */
  void apply_node(I32 idx, const Action& action) {
    tree[idx] = ActedMonoid::apply(tree[idx], action, seg_len[idx]);
    if (idx >= size) {
      my_assert(!tree[idx].fail);
      return;
    }
    if (has_lazy[idx]) lazy[idx] = ActedMonoid::compose(lazy[idx], action);
    else {
      lazy[idx] = action;
      has_lazy[idx] = true;
    }
    if (tree[idx].fail) {
      push(idx);
      pull(idx);
    }
  }

  /// @brief Pushes one pending lazy tag to the two children of @p idx.
  void push(I32 idx) {
    if (!has_lazy[idx]) return;
    const Action action = lazy[idx];
    lazy[idx] = ActedMonoid::action_identity();
    has_lazy[idx] = false;
    apply_node(idx << 1, action);
    apply_node(idx << 1 | 1, action);
  }

  /// @brief Pushes every pending action on the root-to-node path.
  void push_path(I32 idx) {
    for (I32 h = log; h >= 1; --h) {
      push(idx >> h);
    }
  }

  /// @brief Rebuilds aggregates on the node-to-root path.
  void rebuild_path(I32 idx) {
    while (idx > 1) {
      idx >>= 1;
      pull(idx);
    }
  }
};

/// @brief Segment summary carrying the two smallest and two largest values.
template <typename T>
struct BeatsSumMinMaxValue {
  T sum;
  T min;
  T max;
  T second_min;  ///< Smallest value strictly above @c min, or @c max when none.
  T second_max;  ///< Largest value strictly below @c max, or @c min when none.
  I32 min_count;
  I32 max_count;
  bool fail;
};

/// @brief Value monoid for @ref BeatsSumMinMax.
template <typename T>
struct BeatsSumMinMaxMonoid {
  using value_type = BeatsSumMinMaxValue<T>;

  static constexpr bool commute = true;

  /// @brief Empty segment, recognised by an inverted min/max pair.
  static constexpr auto identity() -> value_type {
    return {T(0), infinity<T>, neg_infinity<T>, infinity<T>, neg_infinity<T>, 0, 0, false};
  }

  static constexpr auto from_element(const T& x) -> value_type {
    return {x, x, x, x, x, 1, 1, false};
  }

  static constexpr auto combine(const value_type& a, const value_type& b) -> value_type {
    if (a.min > a.max) return b;
    if (b.min > b.max) return a;

    value_type z{};
    z.sum = a.sum + b.sum;
    z.min = std::min(a.min, b.min);
    z.max = std::max(a.max, b.max);
    z.min_count = (a.min == z.min ? a.min_count : 0) + (b.min == z.min ? b.min_count : 0);
    z.max_count = (a.max == z.max ? a.max_count : 0) + (b.max == z.max ? b.max_count : 0);

    // The runner-up is the tightest of the four candidates that still sits
    // strictly inside the new extremes.
    z.second_min = z.max;
    if (z.min < a.min && a.min < z.second_min) z.second_min = a.min;
    if (z.min < a.second_min && a.second_min < z.second_min) z.second_min = a.second_min;
    if (z.min < b.min && b.min < z.second_min) z.second_min = b.min;
    if (z.min < b.second_min && b.second_min < z.second_min) z.second_min = b.second_min;

    z.second_max = z.min;
    if (z.max > a.max && a.max > z.second_max) z.second_max = a.max;
    if (z.max > a.second_max && a.second_max > z.second_max) z.second_max = a.second_max;
    if (z.max > b.max && b.max > z.second_max) z.second_max = b.max;
    if (z.max > b.second_max && b.second_max > z.second_max) z.second_max = b.second_max;

    z.fail = false;
    return z;
  }

  static constexpr auto unit() -> value_type { return identity(); }
  static constexpr auto op(const value_type& a, const value_type& b) -> value_type {
    return combine(a, b);
  }
};

/**
 * @brief Action monoid for @ref BeatsSumMinMax: `x -> max(min(x + add, cap), floor)`.
 *
 * @details The sentinels @c infinity<T> and @c neg_infinity<T> mean "no clamp"
 * and are deliberately left unshifted by @c add, so a tag that only adds stays
 * recognisable as such no matter how it is composed.
 */
template <typename T>
struct BeatsAddChminChmaxMonoid {
  using value_type = Tuple<T, T, T>;  ///< (add, chmin bound, chmax bound).

  static constexpr bool commute = false;

  static constexpr auto identity() -> value_type {
    return {T(0), infinity<T>, neg_infinity<T>};
  }

  static constexpr auto combine(const value_type& existing, const value_type& fresh) -> value_type {
    auto [add, cap, flr] = existing;
    const auto [add2, cap2, flr2] = fresh;
    add += add2;
    if (cap != infinity<T>) cap += add2;
    if (flr != neg_infinity<T>) flr += add2;
    cap = std::min(cap, cap2);
    flr = std::min(flr, cap2);
    flr = std::max(flr, flr2);
    return {add, cap, flr};
  }

  static constexpr auto unit() -> value_type { return identity(); }
  static constexpr auto op(const value_type& a, const value_type& b) -> value_type {
    return combine(a, b);
  }
};

/// @brief Beats action applying add, then chmin, then chmax to a segment.
template <typename T>
struct BeatsSumMinMaxAction {
  using value_type = BeatsSumMinMaxValue<T>;
  using action_type = Tuple<T, T, T>;

  static auto apply(const value_type& value, const action_type& action, I32 len) -> value_type {
    my_assert(!value.fail);
    value_type x = value;
    if (x.min > x.max) return x;  // Empty segment.

    const auto [add, cap, flr] = action;
    x.sum += T(len) * add;
    x.min += add;
    x.max += add;
    x.second_min += add;
    x.second_max += add;
    if (cap == infinity<T> && flr == neg_infinity<T>) return x;

    const T before_min = x.min;
    const T before_max = x.max;
    x.min = std::max(std::min(x.min, cap), flr);
    x.max = std::max(std::min(x.max, cap), flr);

    if (x.min == x.max) {
      // Everything collapsed onto a single value.
      x.sum = x.max * T(len);
      x.second_min = x.second_max = x.max;
      x.min_count = x.max_count = len;
    } else if (x.second_max <= x.min) {
      // Only the two extremes survive, all of them known by count.
      x.second_max = x.min;
      x.second_min = x.max;
      x.min_count = len - x.max_count;
      x.sum = x.max * T(x.max_count) + x.min * T(x.min_count);
    } else if (x.second_min >= x.max) {
      x.second_max = x.min;
      x.second_min = x.max;
      x.max_count = len - x.min_count;
      x.sum = x.max * T(x.max_count) + x.min * T(x.min_count);
    } else if (x.min < x.second_min && x.max > x.second_max) {
      // The clamp stopped strictly short of both runners-up, so it moved only
      // the extremal values and the sum shifts by a multiple of their counts.
      x.sum += (x.min - before_min) * T(x.min_count) + (x.max - before_max) * T(x.max_count);
    } else {
      x.fail = true;
    }
    return x;
  }
};

/// @brief Acted monoid for range add / chmin / chmax with sum, min and max queries.
template <typename T>
using BeatsSumMinMaxActedMonoid =
    ActedMonoid<BeatsSumMinMaxMonoid<T>, BeatsAddChminChmaxMonoid<T>, BeatsSumMinMaxAction<T>>;

/**
 * @brief Range add, range chmin, range chmax; query sum, minimum and maximum.
 *
 * @details The canonical segment tree beats instantiation. Element values have
 * to stay strictly inside the sentinel band, so @c |value| must remain well
 * below @c infinity<T>; the class asserts this on construction and on @ref set.
 */
template <typename T>
struct BeatsSumMinMax {
  using Value = BeatsSumMinMaxValue<T>;

  SegTreeBeats<BeatsSumMinMaxActedMonoid<T>> seg;

  BeatsSumMinMax() = default;

  explicit BeatsSumMinMax(const Vec<T>& values) {
    seg.build(isz(values), [&](I32 i) -> Value { return leaf(values[i]); });
  }

  template <typename F>
  BeatsSumMinMax(I32 n, F init) {
    seg.build(n, [&](I32 i) -> Value { return leaf(init(i)); });
  }

  [[nodiscard]] auto size() const -> I32 { return seg.n; }

  void set(I32 idx, T x) { seg.set(idx, leaf(x)); }

  [[nodiscard]] auto get(I32 idx) -> T { return seg.get(idx).min; }

  /// @brief `a[i] += x` for every `i` in [l, r).
  void add(I32 l, I32 r, T x) { seg.apply(l, r, {x, infinity<T>, neg_infinity<T>}); }

  /// @brief `a[i] = min(a[i], x)` for every `i` in [l, r).
  void chmin(I32 l, I32 r, T x) { seg.apply(l, r, {T(0), x, neg_infinity<T>}); }

  /// @brief `a[i] = max(a[i], x)` for every `i` in [l, r).
  void chmax(I32 l, I32 r, T x) { seg.apply(l, r, {T(0), infinity<T>, x}); }

  /// @brief Sum, minimum and maximum over [l, r).
  [[nodiscard]] auto query(I32 l, I32 r) -> Tuple<T, T, T> {
    const Value x = seg.query(l, r);
    return {x.sum, x.min, x.max};
  }

  [[nodiscard]] auto sum(I32 l, I32 r) -> T { return seg.query(l, r).sum; }
  [[nodiscard]] auto min(I32 l, I32 r) -> T { return seg.query(l, r).min; }
  [[nodiscard]] auto max(I32 l, I32 r) -> T { return seg.query(l, r).max; }

  /// @brief Materializes the current array.
  [[nodiscard]] auto get_all() -> Vec<T> {
    const Vec<Value> leaves = seg.get_all();
    Vec<T> out(leaves.size());
    FOR(i, isz(out)) out[i] = leaves[i].min;
    return out;
  }

private:
  static auto leaf(T x) -> Value {
    my_assert(neg_infinity<T> < x && x < infinity<T>);
    return BeatsSumMinMaxMonoid<T>::from_element(x);
  }
};

#endif
