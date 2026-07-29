#ifndef CP_MODULES_DATA_STRUCTURES_SLIDING_WINDOW_AGGREGATION_HPP
#define CP_MODULES_DATA_STRUCTURES_SLIDING_WINDOW_AGGREGATION_HPP

#include "../_Common.hpp"

#include <cassert>

template <typename Monoid>
struct SlidingWindowAggregation {
  using Value = typename Monoid::value_type;

  Vec<Value> left_stack;
  Vec<Value> right_stack;
  Vec<Value> left_prod;
  Vec<Value> right_prod;

  SlidingWindowAggregation() : left_prod(1, Monoid::unit()), right_prod(1, Monoid::unit()) {}

  [[nodiscard]] bool empty() const { return left_stack.empty() && right_stack.empty(); }

  [[nodiscard]] I32 size() const { return isz(left_stack) + isz(right_stack); }

  void clear() {
    left_stack.clear();
    right_stack.clear();
    left_prod.assign(1, Monoid::unit());
    right_prod.assign(1, Monoid::unit());
  }

  void push(const Value& x) {
    right_stack.eb(x);
    right_prod.eb(Monoid::op(right_prod.back(), x));
  }

  void pop() {
    assert(!empty());
    if (left_stack.empty()) refill_left();
    left_stack.pop_back();
    left_prod.pop_back();
  }

  [[nodiscard]] Value front() {
    assert(!empty());
    if (left_stack.empty()) refill_left();
    return left_stack.back();
  }

  [[nodiscard]] Value prod() const { return Monoid::op(left_prod.back(), right_prod.back()); }

private:
  void refill_left() {
    while (!right_stack.empty()) {
      Value x = right_stack.back();
      right_stack.pop_back();
      right_prod.pop_back();
      left_stack.eb(x);
      left_prod.eb(Monoid::op(x, left_prod.back()));
    }
  }
};

#endif
