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

//===----------------------------------------------------------------------===//
/* Main Solver Function */

void solve() {
  INT(n, k);
  STR(s);

  if (k < 1) return;

  I32 red = 0;
  I32 tot = 0;
  I32 m = 2 * n;

  FOR(i, m) {
    if (s[i] == '0') continue;
    ++tot;
    if (i & 1) ++red;
  }

  FOR(i, m) {
    if (s[i] == '1' && s[(i + 1) % m] == '0') {
      red += (i & 1) ? -1 : 1;
    }
  }

  OUT(red, tot - red);
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
