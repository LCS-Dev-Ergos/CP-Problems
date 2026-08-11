#if !defined(CP_TEMPLATE_PROFILE_RELAXED)
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#define NEED_CORE
#define NEED_TIMER
#define NEED_RANGE_REDUCERS
#define CP_IO_PROFILE_SIMPLE

#include "templates/Base.hpp"

//===----------------------------------------------------------------------===//
/* Main Solver Function */

void solve() {
  static const VecI32 primes = [] {
    VecI32 ps;
    VecBool is_prime(2001, true);
    is_prime[0] = false;
    is_prime[1] = false;
    FOR(i, 2, 2001) {
      if (!is_prime[i]) continue;
      ps.eb(i);
      if (i * i >= 2001) continue;
      FOR(j, i * i, 2001, i) is_prime[j] = false;
    }
    return ps;
  }();

  INT(n);
  VecI64 a(n), b(n);
  IN(a);
  IN(b);

  VecI64 g(n - 1);
  FOR(i, n - 1) g[i] = std::gcd(a[i], a[i + 1]);

  VecI64 need(n);
  need[0] = g[0];
  need[n - 1] = g[n - 2];
  FOR(i, 1, n - 1) need[i] = g[i - 1] / std::gcd(g[i - 1], g[i]) * g[i];

  Vec2D<I64> cand(n);
  FOR(i, n) {
    cand[i].eb(a[i]);
    if (need[i] <= b[i]) {
      cand[i].eb(need[i]);
      I32 add = 0;
      for (I32 p : primes) {
        if (as<I64>(p) > b[i] / need[i]) break;
        cand[i].eb(need[i] * p);
        ++add;
        if (add == 35) break;
      }
    }
    std::ranges::sort(cand[i]);
    cand[i].erase(std::unique(all(cand[i])), cand[i].end());
  }

  constexpr I32 NEG = -1'000'000'000;
  VecI32 dp_prev(cand[0].size(), 0), dp_cur;
  FOR(j, sz(cand[0])) dp_prev[j] = (cand[0][j] != a[0]);

  FOR(i, 1, n) {
    const I64 pair_g = g[i - 1];
    VecI64 prev_red(cand[i - 1].size()), cur_red(cand[i].size());
    FOR(j, sz(cand[i - 1])) prev_red[j] = cand[i - 1][j] / pair_g;
    FOR(j, sz(cand[i])) cur_red[j] = cand[i][j] / pair_g;

    dp_cur.assign(cand[i].size(), NEG);
    FOR(j, sz(cand[i])) {
      const I32 cost = (cand[i][j] != a[i]);

      FOR(k, sz(cand[i - 1])) {
        if (dp_prev[k] == NEG) continue;
        if (std::gcd(prev_red[k], cur_red[j]) != 1) continue;
        dp_cur[j] = std::max(dp_cur[j], dp_prev[k] + cost);
      }
    }
    dp_prev.swap(dp_cur);
  }

  OUT(MAX(dp_prev));
}

//===----------------------------------------------------------------------===//
/* Main Function */

auto main() -> int {
#ifdef LOCAL
  Timer timer;
  // init_debug_log();
#endif

  INT(T);
  FOR(T) solve();

  return 0;
}

//===----------------------------------------------------------------------===//
