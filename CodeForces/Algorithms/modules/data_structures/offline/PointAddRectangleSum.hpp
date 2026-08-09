#ifndef CP_MODULES_DATA_STRUCTURES_POINT_ADD_RECTANGLE_SUM_HPP
#define CP_MODULES_DATA_STRUCTURES_POINT_ADD_RECTANGLE_SUM_HPP

#include "../_Common.hpp"
#include "modules/data_structures/fenwick/FenwickTree.hpp"
#include "modules/data_structures/utility/IndexCompression.hpp"

/**
 * @brief Offline weighted points with rectangle-sum queries.
 *
 * @details Updates place a weight at a single point, queries ask for the total
 * weight inside an axis-aligned rectangle. Both are collected first and
 * answered together by calc(), which sweeps the plane upwards in y while a
 * Fenwick tree indexed by compressed x holds every point already crossed.
 *
 * A rectangle is split into the difference of two prefixes in y, so each query
 * contributes two sweep events. Rectangles are half-open on both axes, matching
 * every other range structure in the library: a point at (x, y) counts for
 * [xl, xr) x [yl, yr) exactly when xl <= x < xr and yl <= y < yr.
 *
 * Total cost is O((N + Q) log N) for N updates and Q queries. Coordinates are
 * compressed internally, so their magnitude is irrelevant and only the number
 * of distinct update abscissae drives the Fenwick size.
 *
 * calc() reads the collected data without consuming it, so it may be called
 * more than once and updates may be added between calls.
 *
 * @tparam T Weight type; needs to be an additive group (`+=`, `-=`, `T{}`).
 * @tparam XY Coordinate type, only ever compared and never used in arithmetic.
 *
 * @code
 * PointAddRectangleSum<I64> solver;
 * solver.add_query(3, 7, 5);        // weight 5 at (3, 7)
 * solver.sum_query(0, 4, 0, 8);     // total over [0, 4) x [0, 8)
 * const VecI64 answer = solver.calc();
 * @endcode
 */
template <typename T, typename XY = I32>
struct PointAddRectangleSum {
  struct PointUpdate {
    XY x;
    XY y;
    T weight;
  };

  struct RectangleQuery {
    XY xl;
    XY xr;
    XY yl;
    XY yr;
  };

  Vec<PointUpdate> points;
  Vec<RectangleQuery> rectangles;

  PointAddRectangleSum() = default;

  /// @brief Number of collected point updates.
  [[nodiscard]] auto update_count() const -> I32 { return isz(points); }

  /// @brief Number of collected rectangle queries.
  [[nodiscard]] auto query_count() const -> I32 { return isz(rectangles); }

  /// @brief Adds @p weight at the point (@p x, @p y).
  void add_query(XY x, XY y, T weight) { points.push_back({x, y, std::move(weight)}); }

  /// @brief Registers a query over the half-open box [xl, xr) x [yl, yr).
  void sum_query(XY xl, XY xr, XY yl, XY yr) {
    my_assert(!(xr < xl));
    my_assert(!(yr < yl));
    rectangles.push_back({xl, xr, yl, yr});
  }

  /// @brief Answers every registered query, in the order they were registered.
  [[nodiscard]] auto calc() const -> Vec<T> {
    const I32 n = isz(points);
    const I32 q = isz(rectangles);
    Vec<T> answer(size_t(q), T{});
    if (n == 0 || q == 0) return answer;

    // Only the update abscissae can ever carry weight, so the Fenwick tree is
    // sized by those alone; query bounds are resolved by rank against them.
    Vec<XY> abscissae(n);
    FOR(i, n) abscissae[i] = points[i].x;
    const IndexCompression<XY> compress(abscissae);

    // Visit the points in increasing y so the sweep inserts each one once.
    VecI32 by_ordinate(n);
    FOR(i, n) by_ordinate[i] = i;
    std::ranges::sort(by_ordinate, [&](I32 a, I32 b) { return points[a].y < points[b].y; });

    // Each rectangle becomes the prefix at yr minus the prefix at yl.
    struct SweepEvent {
      XY y;
      I32 query;
      bool additive;
    };
    Vec<SweepEvent> events;
    events.reserve(size_t(q) * 2);
    FOR(i, q) {
      events.push_back({rectangles[i].yl, i, false});
      events.push_back({rectangles[i].yr, i, true});
    }
    std::ranges::sort(events, [](const SweepEvent& a, const SweepEvent& b) { return a.y < b.y; });

    FenwickTree<T> bit(compress.size());
    I32 cursor = 0;
    for (const SweepEvent& event : events) {
      // Strict comparison keeps the y interval half-open on the lower end.
      while (cursor < n && points[by_ordinate[cursor]].y < event.y) {
        const PointUpdate& point = points[by_ordinate[cursor]];
        bit.add(compress.rank(point.x), point.weight);
        ++cursor;
      }
      const RectangleQuery& box = rectangles[event.query];
      const T partial = bit.sum(compress.rank(box.xl), compress.rank(box.xr));
      if (event.additive) answer[event.query] += partial;
      else answer[event.query] -= partial;
    }
    return answer;
  }
};

#endif
