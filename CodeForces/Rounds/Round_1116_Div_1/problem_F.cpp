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

#include "modules/number_theory/NTT.hpp"

//===----------------------------------------------------------------------===//
/* Polynomial and Combinatorics */

constexpr I32 MAXN  = 200'000;
constexpr I32 NAIVE = 4'096;

VecI64 fact(MAXN + 1);
VecI64 ifact(MAXN + 1);

auto mul(VecI64 a, VecI64 b) -> VecI64 {
  if (I64(sz(a)) * sz(b) > NAIVE) {
    return NTT::multiply(std::move(a), std::move(b));
  }

  VecI64 c(sz(a) + sz(b) - 1);
  FOR(i, sz(a)) {
    FOR(j, sz(b)) {
      c[i + j] = (c[i + j] + a[i] * b[j]) % MOD2;
    }
  }
  return c;
}

auto build(const VecI64& a, I32 l, I32 r) -> VecI64 {
  if (l == r) return {1, a[l]};
  I32 mid  = (l + r) / 2;
  VecI64 x = build(a, l, mid);
  VecI64 y = build(a, mid + 1, r);
  return mul(std::move(x), std::move(y));
}

void init_comb() {
  fact[0] = 1;
  FOR(i, 1, MAXN + 1) fact[i] = fact[i - 1] * i % MOD2;

  ifact[MAXN] = mod_pow(fact[MAXN], MOD2 - 2, MOD2);
  for (I32 i = MAXN; i > 0; --i) {
    ifact[i - 1] = ifact[i] * i % MOD2;
  }
}

//===----------------------------------------------------------------------===//
/* Main Solver Function */

void solve() {
  INT(n);
  VecI64 a;
  a.reserve(n);

  FOR(n) {
    LL(x);
    if (x != 0) a.push_back(x);
  }

  VecI64 e = a.empty() ? VecI64{1} : build(a, 0, sz(a) - 1);
  auto get = [&](I32 i) -> I64 {
    return i < sz(e) ? e[i] : 0;
  };

  I64 sum = 0;
  FOR(j, n) {
    I64 d = fact[j] * fact[n - j - 1] % MOD2 * ifact[n] % MOD2;
    sum += d;
    if (sum >= MOD2) sum -= MOD2;
  }

  constexpr I64 INV2 = (MOD2 + 1) / 2;
  I64 l   = sum * INV2 % MOD2;
  I64 cur = 0;

  FOR(j, (n - 1) / 2 + 1) {
    I64 invc = fact[j] * fact[n - j] % MOD2 * ifact[n] % MOD2;
    I64 s = (2LL * j - n + MOD2) % MOD2;
    I64 c = (2 * invc + 2 * s % MOD2 * l) % MOD2;
    cur   = (cur + c * get(j) % MOD2 * get(n - j)) % MOD2;

    I64 d = fact[j] * fact[n - j - 1] % MOD2 * ifact[n] % MOD2;
    l = (l - d + MOD2) % MOD2;
  }

  if (n % 2 == 0) {
    I32 m = n / 2;
    I64 invc = fact[m] * fact[m] % MOD2 * ifact[n] % MOD2;
    cur = (cur + invc * get(m) % MOD2 * get(m)) % MOD2;
  }

  I64 ans = 2 * fact[n - 1] % MOD2 * cur % MOD2;
  OUT(ans);
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
