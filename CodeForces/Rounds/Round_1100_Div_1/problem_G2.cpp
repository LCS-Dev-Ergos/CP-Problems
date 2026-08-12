#ifndef CP_TEMPLATE_PROFILE_STRICT
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#define NEED_MACROS
#define NEED_TIMER
#define NEED_CORE
#define NEED_IO
#define CP_IO_PROFILE_SIMPLE

#include "templates/Base.hpp"
#include "modules/data_structures/segment_tree/SegmentTree.hpp"
#include "modules/data_structures/line/RangeLineHull.hpp"

//===----------------------------------------------------------------------===//
/* Solver Helpers */

/* Warning: This solution is not correct. */

using namespace std;

constexpr I64 NEG = -4'000'000'000'000'000'000LL;

I64 clip(I128 value) {
  if (value < I128(NEG)) return NEG;
  if (value > I128(-NEG)) return -NEG;
  return I64(value);
}

struct MaxOp {
  I64 operator()(I64 a, I64 b) const {
    return max(a, b);
  }
};

struct State {
  I64 t = 0;
  I64 mx = 0;
  I64 val = 0;
};

I64 solve_dir(const I32 n, const I64 k, const I32 x, const VecI64& h, const VecI64& d) {
  VecI64 pref(n + 1, 0);
  FOR(i, 1, n + 1) pref[i] = pref[i - 1] + h[i];

  VecI64 lt(n + 2, 0), lmx(n + 2, 0), lval(n + 2, 0);
  lmx[x] = h[x];
  for (I32 l = x - 1; l >= 1; --l) {
    lt[l]  = max(lt[l + 1] + 1, d[l]);
    lmx[l] = max(lmx[l + 1], h[l]);
    lval[l] = lval[l + 1] + lmx[l + 1] * (lt[l] - lt[l + 1] - 1) + h[l];
  }

  Vec<State> left;
  left.push_back({0, h[x], 0});
  I64 cur = h[x];
  for (I32 l = x - 1; l >= 1; --l) {
    if (h[l] <= cur) continue;
    cur = h[l];
    const I64 t = lt[l] + x - l;
    const I64 b = lval[l] + pref[x] - pref[l];
    left.push_back({t, lmx[l], b});
  }

  VecI64 rh{h[x]};
  VecI64 dist{0};
  VecI64 early{0};
  VecI64 gain{0};
  VecI64 gate{0};
  VecI64 w{0};

  I64 rt  = 0;
  I64 rmx = h[x];
  I64 sum = 0;
  for (I32 r = x + 1; r <= n; ++r) {
    rt = max(rt + 1, d[r - 1]);
    sum += h[r];
    if (h[r] <= rmx) continue;

    const I64 old = rmx;
    rmx = h[r];

    const I64 len = r - x;
    const I64 g = rt - len;
    rh.push_back(rmx);
    dist.push_back(len);
    early.push_back(rt);
    gain.push_back(sum);
    gate.push_back(g);
    w.push_back(w.back() + g * (rmx - old));
  }

  const I32 m = int(rh.size()) - 1;
  I64 ans = 0;

  Vec<RangeLineHull::Line> lines;
  VecI64 flat;
  if (m > 0) {
    lines.reserve(m);
    flat.reserve(m);
    FOR(i, 1, m + 1) {
      const I64 hi = rh[i];
      const I64 base = clip(I128(gain[i]) + I128(k - dist[i]) * hi);
      lines.push_back({-hi, base});
      flat.push_back(base - w[i]);
    }
  }

  RangeLineHull hullSeg;
  SegmentTree<I64, MaxOp> maxSeg(0, NEG, MaxOp{});
  if (m > 0) {
    hullSeg.init(lines);
    maxSeg = SegmentTree<I64, MaxOp>(flat, NEG, MaxOp{});
  }

  const I32 by_early = int(upper_bound(all(early), k) - early.begin()) - 1;

  for (const auto& s : left) {
    const I64 t = s.t;
    const I64 mx = s.mx;
    const I64 base = s.val;
    if (t > k) continue;

    ans = max(ans, base + (k - t) * mx);
    if (m == 0) continue;

    const I32 l = int(upper_bound(all(rh), mx) - rh.begin());
    if (l > m) continue;

    const I32 by_dist = int(upper_bound(all(dist), k - t) - dist.begin()) - 1;
    const I32 r = min(by_early, by_dist);
    if (r < l) continue;

    const I32 p = int(upper_bound(all(gate), t) - gate.begin()) - 1;

    const I64 wait = max<I64>(0, gate[l] - t);
    I64 loss = 0;
    if (l > p) {
      loss = w[l] - w[p] - t * (rh[l] - rh[p]);
    }
    const I64 corr = loss - wait * (rh[l] - mx);

    I64 best = NEG;
    const I32 mid = min(r, p);
    if (l <= mid) {
      best = max(best, hullSeg.query(l - 1, mid, t));
    }

    const I32 tail = max(l, p + 1);
    if (tail <= r) {
      const I64 suf = maxSeg.query(tail - 1, r);
      const I64 add = w[p] - t * rh[p];
      best = max(best, suf + add);
    }

    if (best > NEG / 2) {
      ans = max(ans, base + corr + best);
    }
  }

  return ans;
}

//===----------------------------------------------------------------------===//
/* Main Solver Function */

void solve() {
  INT(n, k, x);
  VecI64 H(n + 1, 0);
  VecI64 D(n, 0);
  FOR(i, 1, n + 1) IN(H[i]);
  FOR(i, 1, n) IN(D[i]);

  I64 ans = solve_dir(n, k, x, H, D);

  VecI64 rev_h(n + 1, 0);
  VecI64 rev_d(n, 0);
  FOR(i, 1, n + 1) rev_h[i] = H[n - i + 1];
  FOR(i, 1, n) rev_d[i] = D[n - i];

  ans = max(ans, solve_dir(n, k, n - x + 1, rev_h, rev_d));
  OUT(ans);
}

//===----------------------------------------------------------------------===//
/* Main Function */

auto main() -> int {
#ifdef LOCAL
  Stopwatch timer;
#endif

  INT(T);
  FOR(T) solve();

  return 0;
}

//===----------------------------------------------------------------------===//
