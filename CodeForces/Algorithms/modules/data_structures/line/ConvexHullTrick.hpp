#ifndef CP_MODULES_DATA_STRUCTURES_LINE_CONVEX_HULL_TRICK_HPP
#define CP_MODULES_DATA_STRUCTURES_LINE_CONVEX_HULL_TRICK_HPP

#include "../_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/**
 * @brief Monotonic Convex Hull Trick (minimum queries).
 *
 * @details Assumptions:
 *  - lines are added with non-decreasing slopes.
 *  - queries are performed with non-decreasing 'x'.
 */
struct ConvexHullTrick {
  struct Line {
    I64 m = 0;
    I64 b = 0;
    I64 eval(I64 x) const { return as<I64>(as<I128>(m) * x + b); }
  };

  Vec<Line> hull;
  I32 ptr = 0;

  static bool bad(const Line& l1, const Line& l2, const Line& l3) {
    // (b3 - b1)/(m1 - m3) <= (b2 - b1)/(m1 - m2) using cross multiplication.
    I128 left  = as<I128>(l3.b - l1.b) * as<I128>(l1.m - l2.m);
    I128 right = as<I128>(l2.b - l1.b) * as<I128>(l1.m - l3.m);
    return left <= right;
  }

  void clear() { hull.clear(); ptr = 0; }

  /// @brief Adds line y = m*x + b. Slopes must be non-decreasing.
  void add_line(I64 m, I64 b) {
    Line ln{m, b};
    while (hull.size() >= 2 && bad(hull[hull.size() - 2], hull[hull.size() - 1], ln)) {
      hull.pop_back();
      if (ptr > isz(hull) - 1) {
        ptr = std::max<I32>(0, isz(hull) - 1);
      }
    }
    hull.push_back(ln);
  }

  /// @brief Queries minimum y at x. Query x values must be non-decreasing.
  I64 query(I64 x) {
    my_assert(!hull.empty());
    while (ptr + 1 < isz(hull) && hull[ptr + 1].eval(x) <= hull[ptr].eval(x)) {
      ++ptr;
    }
    return hull[ptr].eval(x);
  }
};

#endif
