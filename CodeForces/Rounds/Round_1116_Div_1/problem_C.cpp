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

#include "modules/Number_Theory.hpp"

//===----------------------------------------------------------------------===//
/* Main Solver Function */

void solve(bool first) {
  INT(n);
  VecStr a(n);
  IN(a);

  I64 w  = 0;
  I64 sr = 0;
  I64 sc = 0;

  FOR(r, n) {
    FOR(c, n) {
      if (a[r][c] == '.') continue;
      ++w;
      sr += r;
      sc += c;
    }
  }

  I64 inv = mod_inverse<I64>(w % n, n);
  I32 cr = sr % n * inv % n;
  I32 cc = sc % n * inv % n;

  if (!first) {
    OUT(cr + 1, cc + 1);
    return;
  }

  INT(rx, cx);
  --rx;
  --cx;

  auto norm = [&](I64 x) -> I32 {
    x %= n;
    if (x < 0) x += n;
    return x;
  };

  I32 dr = norm(w * (rx - cr));
  I32 dc = norm(w * (cx - cc));

  if (dr == 0 && dc == 0) {
    OUT(1, 1, 1, 1);
    return;
  }

  FOR(r, n) {
    FOR(c, n) {
      if (a[r][c] == '.') continue;
      I32 nr = (r + dr) % n;
      I32 nc = (c + dc) % n;
      if (a[nr][nc] == '#') continue;
      OUT(r + 1, c + 1, nr + 1, nc + 1);
      return;
    }
  }
}

//===----------------------------------------------------------------------===//
/* Main Function */

auto main() -> int {
#ifdef LOCAL
  Stopwatch timer;
#endif

  STR(mode);
  bool first = mode == "first";

  INT(T);
  FOR(T) solve(first);

  return 0;
}

//===----------------------------------------------------------------------===//
