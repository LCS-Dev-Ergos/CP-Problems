#ifndef CP_MODULES_DATA_STRUCTURES_RECTANGLE_ADD_RECTANGLE_SUM_HPP
#define CP_MODULES_DATA_STRUCTURES_RECTANGLE_ADD_RECTANGLE_SUM_HPP

#include "../_Common.hpp"
#include "modules/data_structures/fenwick/FenwickTree.hpp"
#include "modules/data_structures/utility/IndexCompression.hpp"

/**
 * @brief Offline rectangle updates with rectangle-sum queries.
 *
 * @details Updates add a weight over a rectangle, queries ask for the total of
 * the resulting lattice values over another rectangle. Both rectangles are
 * half-open on each axis.
 *
 * As in RectangleAddPointSum, adding @c w over [xl, xr) x [yl, yr) becomes four
 * signed corners, after which the value at a lattice point is the signed total
 * of the corners below and to the left of it. Summing that over the quadrant
 * x < X, y < Y turns each corner (a, b, w) into the closed form
 *
 *     w * (X - a) * (Y - b)   for a < X and b < Y, and 0 otherwise,
 *
 * because the lattice points it reaches inside the quadrant are exactly
 * [a, X) x [b, Y). Expanding the product splits the quadrant total into four
 * running moments that do not depend on the query:
 *
 *     P(X, Y) = X*Y*sum(w) - X*sum(w*b) - Y*sum(w*a) + sum(w*a*b).
 *
 * So one Fenwick tree carrying the quadruple (w, w*b, w*a, w*a*b) answers every
 * quadrant total, and a rectangle query is the usual four-corner combination of
 * quadrants. Total cost is O((N + Q) log N) for N updates and Q queries, with
 * four Fenwick descents per query.
 *
 * @warning Unlike the other two rectangle structures, this one multiplies
 * coordinates into the accumulated values, so @p T has to hold products of the
 * form weight * x * y summed over all updates. With a 64-bit @p T and
 * coordinates near 1e9 that overflows quickly; use a modular @p T, or keep the
 * coordinates small. A modular @p T additionally requires non-negative
 * coordinates, since they are fed through @c T(coordinate).
 *
 * calc() reads the collected data without consuming it, so it may be called
 * more than once and updates may be added between calls.
 *
 * @tparam T Value type; needs `+=`, unary `-`, multiplication and `T{}`.
 * @tparam XY Coordinate type, convertible to @p T.
 *
 * @code
 * RectangleAddRectangleSum<I64> solver;
 * solver.add_query(0, 4, 0, 8, 5);   // +5 over [0, 4) x [0, 8)
 * solver.sum_query(2, 6, 3, 9);      // total over [2, 6) x [3, 9), here 50
 * const VecI64 answer = solver.calc();
 * @endcode
 */
template <typename T, typename XY = I64>
struct RectangleAddRectangleSum {
  struct RectangleUpdate {
    XY xl;
    XY xr;
    XY yl;
    XY yr;
    T weight;
  };

  struct RectangleQuery {
    XY xl;
    XY xr;
    XY yl;
    XY yr;
  };

  /// @brief Running totals of w, w*b, w*a and w*a*b over the inserted corners.
  struct Moments {
    T constant{};
    T by_ordinate{};
    T by_abscissa{};
    T by_product{};

    auto operator+=(const Moments& other) -> Moments& {
      constant += other.constant;
      by_ordinate += other.by_ordinate;
      by_abscissa += other.by_abscissa;
      by_product += other.by_product;
      return *this;
    }

    [[nodiscard]] friend auto operator+(Moments lhs, const Moments& rhs) -> Moments {
      lhs += rhs;
      return lhs;
    }

    [[nodiscard]] friend auto operator-(const Moments& value) -> Moments {
      return {-value.constant, -value.by_ordinate, -value.by_abscissa, -value.by_product};
    }
  };

  Vec<RectangleUpdate> rectangles;
  Vec<RectangleQuery> queries;

  RectangleAddRectangleSum() = default;

  /// @brief Number of collected rectangle updates.
  [[nodiscard]] auto update_count() const -> I32 { return isz(rectangles); }

  /// @brief Number of collected rectangle queries.
  [[nodiscard]] auto query_count() const -> I32 { return isz(queries); }

  /// @brief Adds @p weight over the half-open box [xl, xr) x [yl, yr).
  void add_query(XY xl, XY xr, XY yl, XY yr, T weight) {
    my_assert(!(xr < xl));
    my_assert(!(yr < yl));
    rectangles.push_back({xl, xr, yl, yr, std::move(weight)});
  }

  /// @brief Registers a query over the half-open box [xl, xr) x [yl, yr).
  void sum_query(XY xl, XY xr, XY yl, XY yr) {
    my_assert(!(xr < xl));
    my_assert(!(yr < yl));
    queries.push_back({xl, xr, yl, yr});
  }

  /// @brief Answers every registered query, in the order they were registered.
  [[nodiscard]] auto calc() const -> Vec<T> {
    const I32 n = isz(rectangles);
    const I32 q = isz(queries);
    Vec<T> answer(size_t(q), T{});
    if (n == 0 || q == 0) return answer;

    struct Corner {
      XY x;
      XY y;
      bool additive;
      I32 source;
    };
    Vec<Corner> corners;
    corners.reserve(size_t(n) * 4);
    FOR(i, n) {
      const RectangleUpdate& box = rectangles[i];
      corners.push_back({box.xl, box.yl, true, i});
      corners.push_back({box.xr, box.yl, false, i});
      corners.push_back({box.xl, box.yr, false, i});
      corners.push_back({box.xr, box.yr, true, i});
    }
    std::ranges::sort(corners, [](const Corner& a, const Corner& b) { return a.x < b.x; });

    // A quadrant total only ever reads corners strictly below its own bound, so
    // the compressed axis is the set of corner ordinates.
    Vec<XY> ordinates(corners.size());
    FOR(i, isz(corners)) ordinates[i] = corners[i].y;
    const IndexCompression<XY> compress(ordinates);

    // Each query is the alternating sum of four quadrant totals.
    struct Quadrant {
      XY x;
      XY y;
      I32 query;
      bool additive;
    };
    Vec<Quadrant> quadrants;
    quadrants.reserve(size_t(q) * 4);
    FOR(i, q) {
      const RectangleQuery& box = queries[i];
      quadrants.push_back({box.xr, box.yr, i, true});
      quadrants.push_back({box.xl, box.yr, i, false});
      quadrants.push_back({box.xr, box.yl, i, false});
      quadrants.push_back({box.xl, box.yl, i, true});
    }
    std::ranges::sort(quadrants, [](const Quadrant& a, const Quadrant& b) { return a.x < b.x; });

    const I32 corner_count = isz(corners);
    FenwickTree<Moments> bit(compress.size());
    I32 cursor = 0;
    for (const Quadrant& quadrant : quadrants) {
      // Strict comparison: a corner sitting exactly on the bound is outside.
      while (cursor < corner_count && corners[cursor].x < quadrant.x) {
        const Corner& corner = corners[cursor];
        const T& raw = rectangles[corner.source].weight;
        const T weight = corner.additive ? raw : T(-raw);
        const T abscissa = T(corner.x);
        const T ordinate = T(corner.y);
        bit.add(compress.rank(corner.y),
                Moments{weight, weight * ordinate, weight * abscissa, weight * abscissa * ordinate});
        ++cursor;
      }

      const Moments totals = bit.sum(compress.rank(quadrant.y));
      const T x = T(quadrant.x);
      const T y = T(quadrant.y);
      const T total =
          x * y * totals.constant - x * totals.by_ordinate - y * totals.by_abscissa +
          totals.by_product;
      if (quadrant.additive) answer[quadrant.query] += total;
      else answer[quadrant.query] -= total;
    }
    return answer;
  }
};

#endif
