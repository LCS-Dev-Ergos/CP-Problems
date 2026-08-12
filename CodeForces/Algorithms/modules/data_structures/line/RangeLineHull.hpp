#ifndef CP_MODULES_DATA_STRUCTURES_LINE_RANGE_LINE_HULL_HPP
#define CP_MODULES_DATA_STRUCTURES_LINE_RANGE_LINE_HULL_HPP

#include "../_Common.hpp"

/**
 * @brief Static segment tree of convex hulls for max line queries on ranges.
 *
 * @details
 * Stores an array of lines "y = m*x + b".
 * Query returns `max(m*x + b)` over lines in a half-open interval [l, r).
 *
 * Contract: line slopes in the input array must be non-increasing.
 * This is common when records are processed with increasing values and queried
 * as lines with negative slopes.
 */
struct RangeLineHull {
  struct Line {
    I64 m = 0;
    I64 b = neg_infinity<I64>;

    I64 eval(I64 x) const {
      const I128 y = I128(m) * x + b;
      if (y < I128(neg_infinity<I64>)) return neg_infinity<I64>;
      if (y > I128(infinity<I64>)) return infinity<I64>;
      return I64(y);
    }
  };

  struct Hull {
    Vec<Line> lines;

    static bool bad(const Line& a, const Line& b, const Line& c) {
      const I128 lhs = I128(a.b - b.b) * (c.m - b.m);
      const I128 rhs = I128(b.b - c.b) * (b.m - a.m);
      return lhs >= rhs;
    }

    void add(Line line) {
      while (lines.size() >= 2 && bad(lines[lines.size() - 2], lines.back(), line)) {
        lines.pop_back();
      }
      lines.push_back(line);
    }

    void build(const Vec<Line>& src, int l, int r) {
      lines.clear();
      lines.reserve(r - l + 1);
      for (int i = r; i >= l; --i) add(src[i]);
    }

    I64 query(I64 x) const {
      if (lines.empty()) return neg_infinity<I64>;

      int lo = 0;
      int hi = int(lines.size()) - 1;
      while (lo < hi) {
        const int mid = (lo + hi) / 2;
        if (lines[mid].eval(x) <= lines[mid + 1].eval(x)) {
          lo = mid + 1;
        } else {
          hi = mid;
        }
      }
      return lines[lo].eval(x);
    }
  };

  int n = 0;
  Vec<Line> src;
  Vec<Hull> tree;

  RangeLineHull() = default;

  explicit RangeLineHull(const Vec<Line>& lines) { init(lines); }

  void init(const Vec<Line>& lines) {
    src = lines;
    n = int(src.size());
    tree.assign(4 * std::max(n, 1) + 5, Hull{});
    if (n > 0) build(1, 0, n - 1);
  }

  I64 query(int l, int r, I64 x) const {
    if (n == 0) return neg_infinity<I64>;
    l = std::max(l, 0);
    r = std::min(r, n);
    if (l >= r) return neg_infinity<I64>;
    return query(1, 0, n - 1, l, r - 1, x);
  }

private:
  void build(int u, int l, int r) {
    tree[u].build(src, l, r);
    if (l == r) return;
    const int mid = (l + r) / 2;
    build(u * 2, l, mid);
    build(u * 2 + 1, mid + 1, r);
  }

  I64 query(int u, int l, int r, int ql, int qr, I64 x) const {
    if (qr < l || r < ql) return neg_infinity<I64>;
    if (ql <= l && r <= qr) return tree[u].query(x);
    const int mid = (l + r) / 2;
    return std::max(
        query(u * 2, l, mid, ql, qr, x),
        query(u * 2 + 1, mid + 1, r, ql, qr, x));
  }
};

#endif
