#ifndef CP_MODULES_GEOMETRY_HALF_PLANE_HPP
#define CP_MODULES_GEOMETRY_HALF_PLANE_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"
#include "Polygon.hpp"

/// @brief Half-plane represented by directed line (left side is feasible).
template <typename T>
struct HalfPlane {
  Point2D<T> p1, p2;  // Half-plane to the left of p1->p2.

  HalfPlane(const Point2D<T>& a, const Point2D<T>& b) : p1(a), p2(b) {}

  /// @brief Checks if point lies in the half-plane.
  bool contains(const Point2D<T>& p) const {
    return orientation(p1, p2, p) >= -EPS;
  }

  /// @brief Direction angle of boundary line.
  T angle() const { return atan2(p2.y - p1.y, p2.x - p1.x); }
};

/// @brief Intersects a set of half-planes and returns feasible polygon.
template <typename T>
Polygon<T> half_plane_intersection(Vec<HalfPlane<T>> planes) {
  static_assert(std::is_floating_point_v<T>, "half_plane_intersection requires floating-point coordinates");
  sort(all(planes), [](const HalfPlane<T>& a, const HalfPlane<T>& b) {
    return a.angle() < b.angle();
  });

  if (planes.empty()) return Polygon<T>();

  auto intersect_segment_with_line = [](const Point2D<T>& a, const Point2D<T>& b, const HalfPlane<T>& hp) -> Point2D<T> {
    Point2D<T> ab = b - a;
    Point2D<T> dir = hp.p2 - hp.p1;
    T denom = ab.cross(dir);
    if (abs(denom) < EPS) return a;
    T t = (hp.p1 - a).cross(dir) / denom;
    return a + ab * t;
  };

  auto nearly_equal = [](const Point2D<T>& a, const Point2D<T>& b) {
    return abs(a.x - b.x) <= EPS && abs(a.y - b.y) <= EPS;
  };

  auto push_unique = [&](Vec<Point2D<T>>& poly, const Point2D<T>& p) {
    if (poly.empty() || !nearly_equal(poly.back(), p)) {
      poly.push_back(p);
    }
  };

  const T BOUND = as<T>(1e9);
  Vec<Point2D<T>> polygon = {
    {-BOUND, -BOUND},
    { BOUND, -BOUND},
    { BOUND,  BOUND},
    {-BOUND,  BOUND},
  };

  for (const auto& hp : planes) {
    if (polygon.empty()) break;
    Vec<Point2D<T>> clipped;
    I32 n = sz(polygon);

    FOR(i, n) {
      Point2D<T> curr = polygon[i];
      Point2D<T> next = polygon[(i + 1) % n];
      bool in_curr = hp.contains(curr);
      bool in_next = hp.contains(next);

      if (in_curr && in_next) {
        push_unique(clipped, next);
      } else if (in_curr && !in_next) {
        push_unique(clipped, intersect_segment_with_line(curr, next, hp));
      } else if (!in_curr && in_next) {
        push_unique(clipped, intersect_segment_with_line(curr, next, hp));
        push_unique(clipped, next);
      }
    }

    if (!clipped.empty() && nearly_equal(clipped.front(), clipped.back())) {
      clipped.pop_back();
    }
    polygon.swap(clipped);
  }

  return Polygon<T>(polygon);
}

#endif
