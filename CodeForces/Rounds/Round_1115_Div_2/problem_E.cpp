#ifndef CP_TEMPLATE_PROFILE_STRICT
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#define NEED_MACROS
#define NEED_TIMER
#define NEED_TYPES
#define NEED_CONSTANTS
#define CP_IO_PROFILE_SIMPLE

#include "templates/Base.hpp"

//===----------------------------------------------------------------------===//
/* Main Solver Function */

void solve() {
  LL(n);

  Array<I32, 16> dp{}, ndp{};
  dp[0] = 1;

  FOR(i, 60) {
    ndp.fill(0);
    I32 ni = n >> i & 1;

    FOR(st, 16) {
      if (!dp[st]) continue;
      I32 q = st & 1;
      I32 p = st >> 1 & 1;
      I32 u = st >> 2 & 1;
      I32 v = st >> 3;

      FOR(x, 2) FOR(y, 2) {
        I32 z = x ^ y;
        I32 sum = x + z + q;
        if ((sum & 1) != p) continue;

        I32 ns = (sum >> 1) | (y << 1) |
                 ((x + u > ni) << 2) | ((z + v > ni) << 3);
        ndp[ns] += dp[st];
        if (ndp[ns] >= MOD) ndp[ns] -= MOD;
      }
    }

    dp = ndp;
  }

  I32 total = dp[0] + dp[3];
  if (total >= MOD) total -= MOD;
  I64 ans = I64(total - 1 + MOD) * INV2 % MOD;
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
