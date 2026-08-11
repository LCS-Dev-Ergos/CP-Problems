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
#include "modules/Number_Theory.hpp"

//===----------------------------------------------------------------------===//
/* Global State */

Map<I64, VecI64> factor_cache;

auto get_prime_factors(I64 x) -> const VecI64& {
  if (auto it = factor_cache.find(x); it != factor_cache.end()) {
    return it->second;
  }

  VecI64 factors = factorize(x);
  factors.erase(std::unique(all(factors)), factors.end());
  return factor_cache.emplace(x, std::move(factors)).first->second;
}

//===----------------------------------------------------------------------===//
/* Main Solver Function */

///@Note: This solution reached TLE on test 3. The correct one is in problem_E2.cpp

void solve() {
  INT(n);

  VecI32 subtree_size(n + 1, 0);
  VecI32 best_edges(n + 1, 0);
  Vec2D<I64> active_primes(n + 1);
  Vec2D<I32> dp(n + 1);

  FOR(step, n) {
    const I32 u = n - step;

    I64 value;
    I32 child_count;
    IN(value, child_count);

    const VecI64& primes = get_prime_factors(value);
    active_primes[u] = primes;

    VecI32 gain(sz(primes), 0);
    I32 base_edges = 0;
    I32 total_size = 1;

    FOR(_, child_count) {
      INT(v);
      base_edges += best_edges[v];
      total_size += subtree_size[v];

      I64 i = 0;
      I64 j = 0;
      while (i < sz(primes) && j < sz(active_primes[v])) {
        if (primes[i] < active_primes[v][j]) {
          ++i;
        } elif (primes[i] > active_primes[v][j]) {
          ++j;
        } else {
          chmax(gain[i], dp[v][j] + 1 - best_edges[v]);
          ++i;
          ++j;
        }
      }
    }

    subtree_size[u] = total_size;
    dp[u].assign(sz(primes), base_edges);
    FOR(i, sz(primes)) {
      dp[u][i] += gain[i];
    }

    best_edges[u] = MAX(dp[u]);
    OUT(subtree_size[u] - best_edges[u]);
    FLUSH();
  }
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
