#ifndef CP_MODULES_DATA_STRUCTURES_OFFLINE_DYNAMIC_QUERIES_HPP
#define CP_MODULES_DATA_STRUCTURES_OFFLINE_DYNAMIC_QUERIES_HPP

#include "../_Common.hpp"

/**
 * @file OfflineDynamicQueries.hpp
 * @brief Schedules add/remove lifetimes over time for a rollback structure.
 *
 * @details Each event is added to O(log Q) segment-tree nodes. During run(),
 * the caller's state is snapshotted, updated, observed at every time point and
 * rolled back. Equal events may overlap only if the caller gives them distinct
 * keys.
 */
template <typename Event>
struct OfflineDynamicQueries {
private:
  I32 q = 0;
  Vec2D<Event> seg;
  Map<Event, I32> active;
  bool closed = false;

  void add_node(I32 p, I32 l, I32 r, I32 ql, I32 qr, const Event& event) {
    if (qr <= l || r <= ql) return;
    if (ql <= l && r <= qr) {
      seg[p].push_back(event);
      return;
    }
    const I32 mid = l + (r - l) / 2;
    add_node(2 * p, l, mid, ql, qr, event);
    add_node(2 * p + 1, mid, r, ql, qr, event);
  }

public:
  explicit OfflineDynamicQueries(I32 time_count = 0) { init(time_count); }

  void init(I32 time_count) {
    my_assert(time_count >= 0);
    q = time_count;
    seg.assign(std::max(1, 4 * q), {});
    active.clear();
    closed = false;
  }

  void add_interval(I32 l, I32 r, const Event& event) {
    my_assert(!closed);
    my_assert(0 <= l && l <= r && r <= q);
    if (l < r) add_node(1, 0, q, l, r, event);
  }

  void add(I32 time, const Event& event) {
    my_assert(!closed);
    my_assert(0 <= time && time < q);
    my_assert(!active.contains(event));
    active[event] = time;
  }

  void remove(I32 time, const Event& event) {
    my_assert(!closed);
    my_assert(0 <= time && time <= q);
    const auto it = active.find(event);
    my_assert(it != active.end() && it->second <= time);
    add_interval(it->second, time, event);
    active.erase(it);
  }

  void close_all() {
    if (closed) return;
    for (const auto& [event, time] : active) add_interval(time, q, event);
    active.clear();
    closed = true;
  }

  template <typename FApply, typename FSnapshot, typename FRollback, typename FAnswer>
  void run(FApply apply, FSnapshot snapshot, FRollback rollback, FAnswer answer) {
    close_all();
    if (q == 0) return;
    auto dfs = [&](auto&& self, I32 p, I32 l, I32 r) -> void {
      const auto snap = snapshot();
      for (const Event& event : seg[p]) apply(event);
      if (r - l == 1) {
        answer(l);
      } else {
        const I32 mid = l + (r - l) / 2;
        self(self, 2 * p, l, mid);
        self(self, 2 * p + 1, mid, r);
      }
      rollback(snap);
    };
    dfs(dfs, 1, 0, q);
  }

  [[nodiscard]] auto time_count() const -> I32 { return q; }
};

#endif
