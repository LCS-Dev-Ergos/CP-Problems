#ifndef CP_MODULES_OPTIMIZATION_SLOPE_TRICK_HPP
#define CP_MODULES_OPTIMIZATION_SLOPE_TRICK_HPP

#include "_Common.hpp"

/**
 * @brief Convex piecewise-linear function with unit slope changes.
 *
 * @details The function is stored as
 *
 *     f(x) = min_f + sum_{l in left} max(0, l - x) + sum_{r in right} max(0, x - r),
 *
 * so @c left and @c right hold the breakpoints of the decreasing and the
 * increasing part, each contributing one unit of slope. Keeping them in two
 * heaps makes every operation that a DP over a convex objective needs cost a
 * logarithmic factor: adding @c |x-a| or a hinge, translating the function,
 * taking a sliding window minimum, or discarding one of the two arms.
 *
 * Both heaps carry a lazy additive offset, which is what makes @ref shift and
 * @ref sliding_window_minimum constant time. The minimum is attained on the
 * whole interval between the two top breakpoints.
 */
struct SlopeTrick {
  static constexpr I64 LEFT_EMPTY = neg_infinity<I64>;
  static constexpr I64 RIGHT_EMPTY = infinity<I64>;

  PriorityQueue<I64> left;      ///< Breakpoints of the decreasing part, max on top.
  MinPriorityQueue<I64> right;  ///< Breakpoints of the increasing part, min on top.
  I64 add_left = 0;
  I64 add_right = 0;
  I64 min_f = 0;

  SlopeTrick() = default;

  /// @brief Number of breakpoints, counted with multiplicity.
  [[nodiscard]] auto size() const -> I32 { return isz(left) + isz(right); }

  /// @brief Minimum value of the function.
  [[nodiscard]] auto min() const -> I64 { return min_f; }

  /// @brief Left end, right end and value of the minimising interval.
  [[nodiscard]] auto argmin() const -> Tuple<I64, I64, I64> {
    return {top_left(), top_right(), min_f};
  }

  /// @brief f(x) += a.
  void add_const(I64 a) { min_f += a; }

  /// @brief f(x) += max(0, a - x).
  void add_a_minus_x(I64 a) {
    min_f += std::max<I64>(0, a - top_right());
    push_right(a);
    push_left(pop_right());
  }

  /// @brief f(x) += max(0, x - a).
  void add_x_minus_a(I64 a) {
    min_f += std::max<I64>(0, top_left() - a);
    push_left(a);
    push_right(pop_left());
  }

  /// @brief f(x) += |x - a|.
  void add_abs(I64 a) {
    add_a_minus_x(a);
    add_x_minus_a(a);
  }

  /**
   * @brief f(x) += a * x + b.
   *
   * @details Costs O(|a| log n). The slope at either end of the domain moves
   * by @p a, so the arm being flattened must already own at least @c |a|
   * breakpoints; otherwise the result is not representable in this form.
   */
  void add_linear(I64 a, I64 b) {
    min_f += b;
    my_assert(a <= I64(isz(left)) && -a <= I64(isz(right)));
    I64 steepen_right = std::max<I64>(a, 0);
    I64 steepen_left = std::max<I64>(-a, 0);
    FOR(steepen_right) {
      const I64 x = pop_left();
      min_f += x;
      push_right(x);
    }
    FOR(steepen_left) {
      const I64 x = pop_right();
      min_f -= x;
      push_left(x);
    }
  }

  /// @brief f(x) <- min over y <= x of f(y), by dropping the increasing part.
  void clear_right() { right = MinPriorityQueue<I64>(); }

  /// @brief f(x) <- min over y >= x of f(y), by dropping the decreasing part.
  void clear_left() { left = PriorityQueue<I64>(); }

  /// @brief f(x) <- f(x - a).
  void shift(I64 a) {
    add_left += a;
    add_right += a;
  }

  /// @brief f(x) <- min over y in [x - hi, x - lo] of f(y).
  void sliding_window_minimum(I64 lo, I64 hi) {
    my_assert(lo <= hi);
    add_left += lo;
    add_right += hi;
  }

  /// @brief Value at @p x, in O(n log n); the heaps are left untouched.
  [[nodiscard]] auto eval(I64 x) const -> I64 {
    I64 y = min_f;
    PriorityQueue<I64> l = left;
    MinPriorityQueue<I64> r = right;
    while (!l.empty()) {
      y += std::max<I64>(0, (l.top() + add_left) - x);
      l.pop();
    }
    while (!r.empty()) {
      y += std::max<I64>(0, x - (r.top() + add_right));
      r.pop();
    }
    return y;
  }

  /// @brief Largest breakpoint of the decreasing part, or a left sentinel.
  [[nodiscard]] auto top_left() const -> I64 {
    return left.empty() ? LEFT_EMPTY : left.top() + add_left;
  }

  /// @brief Smallest breakpoint of the increasing part, or a right sentinel.
  [[nodiscard]] auto top_right() const -> I64 {
    return right.empty() ? RIGHT_EMPTY : right.top() + add_right;
  }

  void push_left(I64 x) { left.push(x - add_left); }
  void push_right(I64 x) { right.push(x - add_right); }

  auto pop_left() -> I64 {
    const I64 res = top_left();
    if (!left.empty()) left.pop();
    return res;
  }

  auto pop_right() -> I64 {
    const I64 res = top_right();
    if (!right.empty()) right.pop();
    return res;
  }
};

#endif
