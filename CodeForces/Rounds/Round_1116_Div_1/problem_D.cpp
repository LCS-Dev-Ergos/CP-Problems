#ifndef CP_TEMPLATE_PROFILE_STRICT
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#define NEED_MACROS
#define NEED_TIMER
#define CP_IO_PROFILE_SIMPLE

#include "templates/Base.hpp"
#include "templates/core/Types.hpp"

//===----------------------------------------------------------------------===//
/* Main Solver Function */

void solve() {
  INT(n);
  Vec<I64> a(n);
  IN(a);

  auto vld = [&](I32 t) -> bool {
    PriorityQueue<I64> pq(all(a));

    for (I32 k = t - 1; k >= 0 && !pq.empty(); --k) {
      I64 x = pq.top();
      pq.pop();

      if (k >= 30) continue;
      x -= I64(1) << k;
      if (x > 0) pq.push(x);
    }

    return pq.empty();
  };

  I32 l = n - 1;
  I32 r = n + 30;

  while (r - l > 1) {
    I32 mid = (l + r) / 2;
    if (vld(mid)) {
      r = mid;
    } else {
      l = mid;
    }
  }

  OUT(r);
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
