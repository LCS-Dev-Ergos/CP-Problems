#ifndef CP_MODULES_GEOMETRY_POINT_IN_CONVEX_POLYGON_HPP
#define CP_MODULES_GEOMETRY_POINT_IN_CONVEX_POLYGON_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"
#include "Point2D.hpp"

/**
 * @brief O(log n) point-in-convex-polygon test for CCW polygon.
 *
 * @details Polygon must be strictly convex or convex with boundary collinearity,
 * ordered counter-clockwise, and without duplicated final vertex.
 */
template <typename T>
bool point_in_convex_polygon(const Vec<Point2D<T>>& poly, const Point2D<T>& p, bool strict = false) {
  I32 n = isz(poly);
  if (n < 3) return false;

  auto orient = [](const Point2D<T>& a, const Point2D<T>& b, const Point2D<T>& c) -> F80 {
    return as<F80>(orientation(a, b, c));
  };

  F80 o1 = orient(poly[0], poly[1], p);
  F80 o2 = orient(poly[0], poly[n - 1], p);
  if (strict) {
    if (o1 <= EPS || o2 >= -EPS) return false;
  } else {
    if (o1 < -EPS || o2 > EPS) return false;
  }

  I32 l = 1, r = n - 1;
  while (r - l > 1) {
    I32 mid = (l + r) >> 1;
    if (orient(poly[0], poly[mid], p) >= 0) {
      l = mid;
    } else {
      r = mid;
    }
  }

  const auto& a = poly[0];
  const auto& b = poly[l];
  const auto& c = poly[r];

  F80 s = std::abs(orient(a, b, c));
  F80 s1 = std::abs(orient(a, b, p));
  F80 s2 = std::abs(orient(b, c, p));
  F80 s3 = std::abs(orient(c, a, p));

  if (std::abs((s1 + s2 + s3) - s) > EPS) return false;

  if (!strict) return true;
  if (on_segment(p, a, b) || on_segment(p, b, c) || on_segment(p, c, a)) return false;
  return true;
}

#endif
