#ifndef CP_MODULES_DATA_STRUCTURES_SEGMENT_TREE_HULL_HPP
#define CP_MODULES_DATA_STRUCTURES_SEGMENT_TREE_HULL_HPP

#include "../_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/**
 * @brief Segment tree with range assignment and max query over static line hulls.
 *
 * @details
 *  - Each node stores a convex hull built from a contiguous segment of lines.
 *  - Update operation assigns one x value to a range, and each covered node keeps
 *    the value `max(m*x + c)` over its local hull.
 *  - Query operation returns the maximum value over an index interval.
 *
 * @warning
 * Hull construction assumes lines are provided in an order compatible with
 * monotonic-hull merge (same contract used in contest solution for Problem D).
 *
 * Indexing convention: [1..n] inclusive.
 */
struct SegmentTreeHull {
  static constexpr I64 NEG_INF = neg_infinity<I64>;

  I32 n = 0;
  VecI64 lazy;
  Vec<char> has_lazy;
  VecI64 max_val;
  VecI32 hull_start;
  VecI32 hull_len;
  VecI64 hull_m;
  VecI64 hull_c;

  SegmentTreeHull() = default;

  /// @brief Initializes structure with @p n_lines lines (1-indexed input lines).
  void init(I32 n_lines, const Vec<PairI64>& lines) {
    n = std::max<I32>(0, n_lines);
    if (n == 0) {
      lazy.clear();
      has_lazy.clear();
      max_val.clear();
      hull_start.clear();
      hull_len.clear();
      hull_m.clear();
      hull_c.clear();
      return;
    }

    my_assert(isz(lines) >= n + 1);

    const I32 st_size = 4 * n + 8;
    lazy.assign(st_size, 0);
    has_lazy.assign(st_size, false);
    max_val.assign(st_size, NEG_INF);
    hull_start.assign(st_size, 0);
    hull_len.assign(st_size, 0);

    hull_m.clear();
    hull_c.clear();
    hull_m.reserve(n * 20);
    hull_c.reserve(n * 20);

    build(1, 1, n, lines);
  }

  /// @brief Assigns value @p x to range [l, r] (inclusive).
  void range_assign(I32 l, I32 r, I64 x) {
    if (n == 0) return;
    l = std::max<I32>(l, 1);
    r = std::min<I32>(r, n);
    if (l > r) return;
    update(1, 1, n, l, r, x);
  }

  /// @brief Returns max value on range [l, r] (inclusive).
  [[nodiscard]] I64 range_max(I32 l, I32 r) {
    if (n == 0) return NEG_INF;
    l = std::max<I32>(l, 1);
    r = std::min<I32>(r, n);
    if (l > r) return NEG_INF;
    return query(1, 1, n, l, r);
  }

  /// @brief Returns max value on the whole domain [1, n].
  [[nodiscard]] I64 all_max() const {
    if (n == 0) return NEG_INF;
    return max_val[1];
  }

private:
  void build(I32 u, I32 l, I32 r, const Vec<Pair<I64, I64>>& lines) {
    lazy[u] = 0;
    has_lazy[u] = false;
    max_val[u]  = NEG_INF;

    if (l == r) {
      hull_start[u] = isz(hull_m);
      hull_len[u] = 1;
      hull_m.eb(lines[l].first);
      hull_c.eb(lines[l].second);
      return;
    }

    const I32 mid = (l + r) / 2;
    build(2 * u, l, mid, lines);
    build(2 * u + 1, mid + 1, r, lines);

    const I32 start = isz(hull_m);
    I32 count = 0;
    FOR(i, l, r + 1) {
      const auto [m, c] = lines[i];
      while (count >= 2) {
        const I64 m0 = hull_m[start + count - 2];
        const I64 c0 = hull_c[start + count - 2];
        const I64 m1 = hull_m[start + count - 1];
        const I64 c1 = hull_c[start + count - 1];

        const I128 lhs = as<I128>(c - c1) * (m0 - m1);
        const I128 rhs = as<I128>(c1 - c0) * (m1 - m);
        if (lhs >= rhs) {
          --count;
          hull_m.pop_back();
          hull_c.pop_back();
        } else {
          break;
        }
      }
      hull_m.eb(m);
      hull_c.eb(c);
      ++count;
    }

    hull_start[u] = start;
    hull_len[u] = count;
  }

  [[nodiscard]] I64 eval_max(I32 u, I64 x) const {
    const I32 start = hull_start[u];
    const I32 len = hull_len[u];
    if (len == 1) {
      return as<I64>(as<I128>(hull_m[start]) * x + hull_c[start]);
    }

    I32 lo = 0;
    I32 hi = len - 2;
    I32 ans = len - 1;
    while (lo <= hi) {
      const I32 mid = (lo + hi) / 2;
      const I32 idx = start + mid;

      const I64 m1 = hull_m[idx];
      const I64 c1 = hull_c[idx];
      const I64 m2 = hull_m[idx + 1];
      const I64 c2 = hull_c[idx + 1];

      const I128 lhs = as<I128>(x) * (m1 - m2);
      const I128 rhs = as<I128>(c2) - c1;
      if (lhs >= rhs) {
        ans = mid;
        hi = mid - 1;
      } else {
        lo = mid + 1;
      }
    }

    const I32 best = start + ans;
    return as<I64>(as<I128>(hull_m[best]) * x + hull_c[best]);
  }

  void apply(I32 u, I64 x) {
    lazy[u] = x;
    has_lazy[u] = true;
    max_val[u] = eval_max(u, x);
  }

  void push_down(I32 u) {
    if (!has_lazy[u]) return;
    const I64 tag = lazy[u];
    apply(2 * u, tag);
    apply(2 * u + 1, tag);
    has_lazy[u] = false;
  }

  void update(I32 u, I32 l, I32 r, I32 ql, I32 qr, I64 x) {
    if (ql <= l && r <= qr) {
      apply(u, x);
      return;
    }
    push_down(u);
    const I32 mid = (l + r) / 2;
    if (ql <= mid) update(2 * u, l, mid, ql, qr, x);
    if (qr > mid) update(2 * u + 1, mid + 1, r, ql, qr, x);
    max_val[u] = std::max(max_val[2 * u], max_val[2 * u + 1]);
  }

  I64 query(I32 u, I32 l, I32 r, I32 ql, I32 qr) {
    if (ql <= l && r <= qr) return max_val[u];
    push_down(u);
    const I32 mid = (l + r) / 2;
    I64 ans = NEG_INF;
    if (ql <= mid) ans = query(2 * u, l, mid, ql, qr);
    if (qr > mid) ans = std::max(ans, query(2 * u + 1, mid + 1, r, ql, qr));
    return ans;
  }
};

#endif
