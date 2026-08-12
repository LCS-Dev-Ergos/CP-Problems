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
  INT(n);
  VecI64 a(n), d(n - 1);
  IN(a);

  FOR(i, n - 1) d[i] = a[i + 1] - a[i];

  auto even = [](I64 x) { return x % 2 == 0; };
  for (I32 l = 0; l < n - 1;) {
    I32 r = l + 1;
    while (r < n - 1 && even(d[r]) == even(d[l])) ++r;
    sort(d.begin() + l, d.begin() + r);
    l = r;
  }

  FOR(i, 1, n) a[i] = a[i - 1] + d[i - 1];
  OUT(a);
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
