#ifndef CP_MODULES_DATA_STRUCTURES_RECTANGLE_ADD_POINT_SUM_HPP
#define CP_MODULES_DATA_STRUCTURES_RECTANGLE_ADD_POINT_SUM_HPP

#include "../_Common.hpp"
#include "modules/data_structures/fenwick/FenwickTree.hpp"
#include "modules/data_structures/utility/IndexCompression.hpp"

/**
 * @brief Offline rectangle updates with point-value queries.
 *
 * @details This is the transpose of PointAddRectangleSum: updates add a weight
 * over a whole rectangle, queries read the accumulated value at one point.
 *
 * Adding @c w over [xl, xr) x [yl, yr) is rewritten as four corner weights
 * placed at (xl, yl), (xr, yl), (xl, yr) and (xr, yr) with signs +, -, - and +.
 * With that rewrite the value at a point is simply the signed total of every
 * corner below and to the left of it, so calc() sweeps upwards in y, inserts
 * corners into a Fenwick tree indexed by compressed x, and reads a prefix.
 *
 * Total cost is O((N + Q) log Q) for N updates and Q queries. Only the query
 * abscissae are compressed, since a corner can only ever be observed through
 * one of them.
 *
 * calc() reads the collected data without consuming it, so it may be called
 * more than once and updates may be added between calls.
 *
 * @tparam T Weight type; needs to be an additive group (`+`, unary `-`, `T{}`).
 * @tparam XY Coordinate type, only ever compared and never used in arithmetic.
 *
 * @code
 * RectangleAddPointSum<I64> solver;
 * solver.add_query(0, 4, 0, 8, 5);  // +5 over [0, 4) x [0, 8)
 * solver.sum_query(3, 7);           // value at (3, 7), here 5
 * const VecI64 answer = solver.calc();
 * @endcode
 */
template <typename T, typename XY = I32>
struct RectangleAddPointSum {
  struct RectangleUpdate {
    XY xl;
    XY xr;
    XY yl;
    XY yr;
    T weight;
  };

  struct PointQuery {
    XY x;
    XY y;
  };

  Vec<RectangleUpdate> rectangles;
  Vec<PointQuery> points;

  RectangleAddPointSum() = default;

  /// @brief Number of collected rectangle updates.
  [[nodiscard]] auto update_count() const -> I32 { return isz(rectangles); }

  /// @brief Number of collected point queries.
  [[nodiscard]] auto query_count() const -> I32 { return isz(points); }

  /// @brief Adds @p weight over the half-open box [xl, xr) x [yl, yr).
  void add_query(XY xl, XY xr, XY yl, XY yr, T weight) {
    my_assert(!(xr < xl));
    my_assert(!(yr < yl));
    rectangles.push_back({xl, xr, yl, yr, std::move(weight)});
  }

  /// @brief Registers a query for the value at the point (@p x, @p y).
  void sum_query(XY x, XY y) { points.push_back({x, y}); }

  /// @brief Answers every registered query, in the order they were registered.
  [[nodiscard]] auto calc() const -> Vec<T> {
    const I32 n = isz(rectangles);
    const I32 q = isz(points);
    Vec<T> answer(size_t(q), T{});
    if (n == 0 || q == 0) return answer;

    // A corner is only ever observed at a query abscissa, so those are the
    // coordinates worth compressing.
    Vec<XY> abscissae(q);
    FOR(i, q) abscissae[i] = points[i].x;
    const IndexCompression<XY> compress(abscissae);
    const I32 width = compress.size();

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
    std::ranges::sort(corners, [](const Corner& a, const Corner& b) { return a.y < b.y; });

    VecI32 by_ordinate(q);
    FOR(i, q) by_ordinate[i] = i;
    std::ranges::sort(by_ordinate, [&](I32 a, I32 b) { return points[a].y < points[b].y; });

    const I32 corner_count = isz(corners);
    FenwickTree<T> bit(width);
    I32 cursor = 0;
    for (const I32 i : by_ordinate) {
      const PointQuery& point = points[i];
      // A corner at ordinate b is in force from y == b upwards, hence `<=`.
      while (cursor < corner_count && !(point.y < corners[cursor].y)) {
        const Corner& corner = corners[cursor];
        const I32 column = compress.rank(corner.x);
        // A corner to the right of every query abscissa is never observable.
        if (column < width) {
          const T& weight = rectangles[corner.source].weight;
          if (corner.additive) bit.add(column, weight);
          else bit.add(column, -weight);
        }
        ++cursor;
      }
      answer[i] = bit.sum(compress.rank(point.x) + 1);
    }
    return answer;
  }
};

#endif
