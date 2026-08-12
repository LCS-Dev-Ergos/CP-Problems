#ifndef CP_MODULES_GEOMETRY_RECTANGLE_UNION_AREA_HPP
#define CP_MODULES_GEOMETRY_RECTANGLE_UNION_AREA_HPP

#include "_Common.hpp"
#include "modules/data_structures/segment_tree/ActedLazySegTree.hpp"

/**
 * @file RectangleUnionArea.hpp
 * @brief Union area of axis-aligned half-open rectangles in O(N log N).
 */
template <typename Coord>
struct RectangleCoverValue {
  I32 cover = infinity<I32>;
  Coord length{};
};

template <typename Coord>
struct RectangleCoverActedMonoid {
  using value_type = RectangleCoverValue<Coord>;
  using action_type = I32;

  static constexpr auto value_identity() -> value_type { return {}; }
  static constexpr auto action_identity() -> action_type { return 0; }
  static constexpr auto combine(const value_type& a, const value_type& b) -> value_type {
    if (a.cover < b.cover) return a;
    if (a.cover > b.cover) return b;
    return {a.cover, a.length + b.length};
  }
  static constexpr auto compose(action_type existing, action_type fresh) -> action_type {
    return existing + fresh;
  }
  static constexpr auto apply(value_type value, action_type action, I32) -> value_type {
    value.cover += action;
    return value;
  }
};

template <typename Coord = I64, typename Area = I64>
struct RectangleUnionArea {
  struct Rectangle {
    Coord x1;
    Coord y1;
    Coord x2;
    Coord y2;
  };

  Vec<Rectangle> rects;

  void add_rect(Coord x1, Coord y1, Coord x2, Coord y2) {
    my_assert(x1 < x2 && y1 < y2);
    rects.push_back({x1, y1, x2, y2});
  }

  [[nodiscard]] auto area() const -> Area {
    if (rects.empty()) return {};
    struct Event {
      Coord x;
      Coord y1;
      Coord y2;
      I32 delta;
    };

    Vec<Coord> ys;
    Vec<Event> events;
    ys.reserve(2 * rects.size());
    events.reserve(2 * rects.size());
    for (const Rectangle& rect : rects) {
      ys.push_back(rect.y1);
      ys.push_back(rect.y2);
      events.push_back({rect.x1, rect.y1, rect.y2, 1});
      events.push_back({rect.x2, rect.y1, rect.y2, -1});
    }
    std::sort(all(ys));
    ys.erase(std::unique(all(ys)), ys.end());
    std::sort(all(events), [](const Event& a, const Event& b) { return a.x < b.x; });

    Vec<RectangleCoverValue<Coord>> leaves(isz(ys) - 1);
    FOR(i, isz(leaves)) leaves[i] = {0, ys[i + 1] - ys[i]};
    ActedLazySegTree<RectangleCoverActedMonoid<Coord>> seg(leaves);
    const Coord total_y = ys.back() - ys.front();
    Coord covered{};
    Coord prev_x = events.front().x;
    Area ans{};

    I32 i = 0;
    while (i < isz(events)) {
      const Coord x = events[i].x;
      ans += Area(x - prev_x) * Area(covered);
      while (i < isz(events) && events[i].x == x) {
        const I32 l = I32(std::lower_bound(all(ys), events[i].y1) - ys.begin());
        const I32 r = I32(std::lower_bound(all(ys), events[i].y2) - ys.begin());
        seg.apply(l, r, events[i].delta);
        ++i;
      }
      const auto root = seg.prod_all();
      covered = total_y - (root.cover == 0 ? root.length : Coord{});
      prev_x = x;
    }
    return ans;
  }
};

#endif
