#ifndef CP_TEMPLATE_PROFILE_STRICT
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#define NEED_MACROS
#define NEED_TIMER
#define NEED_TYPES
#define CP_IO_PROFILE_SIMPLE

#include "templates/Base.hpp"

//===----------------------------------------------------------------------===//
/* Main Solver Function */

void solve() {
  INT(n, m);
  VecI64 v(n);
  VecI32 a(n * m);
  IN(v, a);

  MultiSet<I32> top;
  I32 ans = m;

  FOR_R(i, n) {
    FOR(j, m) {
      I32 x = a[i * m + j];
      if (isz(top) < m) {
        top.insert(x);
      } else if (x > *top.begin()) {
        top.erase(top.begin());
        top.insert(x);
      }
    }

    I64 sum = 0;
    I32 cnt = 0;
    for (auto it = top.rbegin(); it != top.rend() && cnt < ans; ++it) {
      sum += *it;
      ++cnt;
      if (sum >= v[i]) {
        ans = cnt;
        break;
      }
    }
  }

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
