#ifndef CP_MODULES_GEOMETRY_LINE2D_HPP
#define CP_MODULES_GEOMETRY_LINE2D_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"
#include "Point2D.hpp"

/// @brief 2D line in implicit form 'a * x + b * y = c'.
template <typename T = F64>
struct Line2D {
  T a, b, c;

  Line2D(T a = 0, T b = 0, T c = 0) : a(a), b(b), c(c) {}

  /// @brief Builds line passing through points p1 and p2.
  Line2D(const Point2D<T>& p1, const Point2D<T>& p2) {
    a = p2.y - p1.y;
    b = p1.x - p2.x;
    c = a * p1.x + b * p1.y;
  }

  /// @brief Euclidean distance from point to line.
  T distance(const Point2D<T>& p) const { return abs(a * p.x + b * p.y - c) / sqrt(a * a + b * b); }

  /// @brief Orthogonal projection of point onto this line.
  Point2D<T> project(const Point2D<T>& p) const {
    T d = a * a + b * b;
    T x = (b * (b * p.x - a * p.y) + a * c) / d;
    T y = (a * (-b * p.x + a * p.y) + b * c) / d;
    return Point2D<T>(x, y);
  }
};

/// @brief Segment intersection test with intersection point extraction.
template <typename T, typename U>
bool segment_intersect(const Point2D<T>& a, const Point2D<T>& b,
                       const Point2D<T>& c, const Point2D<T>& d,
                       Point2D<U>& intersection) {
  static_assert(std::is_floating_point_v<U>, "intersection output type must be floating-point");
  using Calc = F80;

  const auto orient = [](
      const Point2D<T>& p, const Point2D<T>& q, const Point2D<T>& r
    ) -> Calc {
      const Calc x1 = as<Calc>(q.x) - as<Calc>(p.x);
      const Calc y1 = as<Calc>(q.y) - as<Calc>(p.y);
      const Calc x2 = as<Calc>(r.x) - as<Calc>(p.x);
      const Calc y2 = as<Calc>(r.y) - as<Calc>(p.y);
      return x1 * y2 - y1 * x2;
    };

  const auto on_segment_calc = [&](
      const Point2D<T>& p, const Point2D<T>& s, const Point2D<T>& t
    ) -> bool {
      if (std::abs(orient(s, t, p)) > EPS) return false;
      const Calc dx1 = as<Calc>(p.x) - as<Calc>(s.x);
      const Calc dy1 = as<Calc>(p.y) - as<Calc>(s.y);
      const Calc dx2 = as<Calc>(p.x) - as<Calc>(t.x);
      const Calc dy2 = as<Calc>(p.y) - as<Calc>(t.y);
      return dx1 * dx2 + dy1 * dy2 <= EPS;
    };

  Calc d1 = orient(c, d, a);
  Calc d2 = orient(c, d, b);
  Calc d3 = orient(a, b, c);
  Calc d4 = orient(a, b, d);

  if (((d1 > EPS && d2 < -EPS) || (d1 < -EPS && d2 > EPS)) &&
      ((d3 > EPS && d4 < -EPS) || (d3 < -EPS && d4 > EPS))) {
    Calc t = d1 / (d1 - d2);
    intersection = Point2D<U>(
      as<U>(as<Calc>(a.x) + (as<Calc>(b.x) - as<Calc>(a.x)) * t),
      as<U>(as<Calc>(a.y) + (as<Calc>(b.y) - as<Calc>(a.y)) * t)
    );
    return true;
  }

  // Check for collinear cases.
  if (std::abs(d1) <= EPS && on_segment_calc(a, c, d)) {
    intersection = Point2D<U>(as<U>(a.x), as<U>(a.y));
    return true;
  }
  if (std::abs(d2) <= EPS && on_segment_calc(b, c, d)) {
    intersection = Point2D<U>(as<U>(b.x), as<U>(b.y));
    return true;
  }
  if (std::abs(d3) <= EPS && on_segment_calc(c, a, b)) {
    intersection = Point2D<U>(as<U>(c.x), as<U>(c.y));
    return true;
  }
  if (std::abs(d4) <= EPS && on_segment_calc(d, a, b)) {
    intersection = Point2D<U>(as<U>(d.x), as<U>(d.y));
    return true;
  }

  return false;
}

#endif
