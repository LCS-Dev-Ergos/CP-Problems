#ifndef CP_TEMPLATE_PROFILE_STRICT
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#define NEED_MACROS
#define NEED_MOD_INT
#define NEED_TIMER
#define CP_IO_PROFILE_SIMPLE

#include "templates/Base.hpp"
#include "templates/core/Types.hpp"

//===----------------------------------------------------------------------===//
/* Main Solver Function */

constexpr I32 MAXN = 1'000'000;

Vec<MInt2> fact(MAXN + 1);
Vec<MInt2> ifact(MAXN + 1);

void init_comb() {
  fact[0] = 1;
  FOR(i, 1, MAXN + 1) fact[i] = fact[i - 1] * i;

  ifact[MAXN] = fact[MAXN].inverse();
  for (I32 i = MAXN; i > 0; --i) {
    ifact[i - 1] = ifact[i] * i;
  }
}

auto comb(I32 n, I32 k) -> MInt2 {
  return fact[n] * ifact[k] * ifact[n - k];
}

void solve() {
  INT(n);
  STR(s);

  I32 cnt[2]{};
  I32 runs[2]{};

  FOR(i, n) {
    I32 c = s[i] - '0';
    ++cnt[c];
    if (i == 0 || s[i] != s[i - 1]) ++runs[c];
  }

  auto ways = [&](I32 c) -> MInt2 {
    if (cnt[c] == 0) return 1;
    return comb(cnt[c] - 1, runs[c] - 1);
  };

  OUT(ways(0) * ways(1));
}

//===----------------------------------------------------------------------===//
/* Main Function */

auto main() -> int {
#ifdef LOCAL
  Stopwatch timer;
#endif

  init_comb();

  INT(T);
  FOR(T) solve();

  return 0;
}

//===----------------------------------------------------------------------===//
