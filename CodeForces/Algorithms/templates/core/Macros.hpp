#pragma once
#include "templates/core/Debug.hpp"
#include "templates/core/TypeTraits.hpp"

#ifndef CP_ENABLE_SHORT_MACROS
  #define CP_ENABLE_SHORT_MACROS 1
#endif

// SUM/MIN/MAX; set to 0 to keep IdiomAliases.hpp out of the submission.
#ifndef CP_ENABLE_RANGE_REDUCERS
  #define CP_ENABLE_RANGE_REDUCERS 1
#endif

#if CP_ENABLE_RANGE_REDUCERS
  #include "templates/core/IdiomAliases.hpp"
#endif

//===----------------------------------------------------------------------===//
/* Advanced Macro System */

// Advanced FOR loop system:
#define CP_LOOP(...) cp::loop<__VA_ARGS__>
#define CP_VAL(x, ...) CP_LOOP(__VA_ARGS__)::val(x)

#define FOR1(a) \
  for (auto _ = CP_VAL(0, decltype(a)); _ < CP_VAL(a, decltype(a)); ++_)
#define FOR2(i, a) \
  for (auto i = CP_VAL(0, decltype(a)); i < CP_VAL(a, decltype(a)); ++i)
#define FOR3(i, a, b) \
  for (auto i = CP_VAL(a, decltype(a), decltype(b)); i < CP_VAL(b, decltype(a), decltype(b)); ++i)
#define FOR4(i, a, b, c)                                          \
  for (auto i = CP_VAL(a, decltype(a), decltype(b), decltype(c)); \
       i < CP_VAL(b, decltype(a), decltype(b), decltype(c));      \
       i += CP_VAL(c, decltype(a), decltype(b), decltype(c)))
#define FOR1_R(a) \
  for (auto _ = CP_VAL(a, decltype(a)); _-- > 0;)
#define FOR2_R(i, a) \
  for (auto i = CP_VAL(a, decltype(a)); i-- > 0;)
#define FOR3_R(i, a, b) \
  for (auto i = CP_VAL(b, decltype(a), decltype(b)); i-- > CP_VAL(a, decltype(a), decltype(b));)

// Overload resolution for FOR macros:
#define overload4(a, b, c, d, e, ...) e
#define overload3(a, b, c, d, ...) d
#define FOR(...) overload4(__VA_ARGS__, FOR4, FOR3, FOR2, FOR1)(__VA_ARGS__)
#define FOR_R(...) overload3(__VA_ARGS__, FOR3_R, FOR2_R, FOR1_R)(__VA_ARGS__)
#define ROF(...) FOR_R(__VA_ARGS__)
#define FORD3(i, a, b) \
  for (auto i = CP_VAL(a, decltype(a), decltype(b)); i >= CP_VAL(b, decltype(a), decltype(b)); --i)
#define FORD4(i, a, b, c)                                         \
  for (auto i = CP_VAL(a, decltype(a), decltype(b), decltype(c)); \
       i >= CP_VAL(b, decltype(a), decltype(b), decltype(c));     \
       i -= CP_VAL(c, decltype(a), decltype(b), decltype(c)))
#define FORD(...) overload4(__VA_ARGS__, FORD4, FORD3)(__VA_ARGS__)

// Range-based iteration macros:
#define ALL(x) std::ranges::begin(x), std::ranges::end(x)
#define RALL(x) std::ranges::rbegin(x), std::ranges::rend(x)

// Container utility macros:
#if CP_ENABLE_SHORT_MACROS
  #define all(x) (x).begin(), (x).end()
  #define rall(x) (x).rbegin(), (x).rend()
  #define sz(x) cp::sz64(x)
  #define isz(x) cp::sz32(x)
  #define len(x) sz(x)
  #define eb emplace_back
  #define elif else if
#endif

#if CP_ENABLE_RANGE_REDUCERS

namespace cp {

// Narrow integers accumulate in I64/U64; any other element type keeps its own.
template <class R>
[[gnu::always_inline]] constexpr auto sum_range(const R& r) {
  using V   = std::ranges::range_value_t<remove_cvref_t<R>>;
  using Acc = Conditional<Int<V> && !std::same_as<V, bool> && sizeof(V) < sizeof(I64),
              Conditional<Signed<V>, I64, U64>, V>;
  return std::accumulate(std::ranges::begin(r), std::ranges::end(r), Acc{});
}

} // namespace cp

#endif

// Advanced container operations:
#define UNIQUE(x) (std::ranges::sort(x), x.erase(std::ranges::unique(x).begin(), x.end()), x.shrink_to_fit())
#define LB(c, x) (I64) std::distance((c).begin(), std::ranges::lower_bound(c, x))
#define UB(c, x) (I64) std::distance((c).begin(), std::ranges::upper_bound(c, x))

#if CP_ENABLE_RANGE_REDUCERS
  #define SUM(x) cp::sum_range(x)
  #define MIN(x)                                       \
    ([&]() -> decltype(auto) {                         \
      auto&& _cp_min_range = (x);                      \
      if (std::ranges::empty(_cp_min_range)) {         \
        my_assert(false && "MIN(): empty range.");     \
        std::abort();                                  \
      }                                                \
      return *std::ranges::min_element(_cp_min_range); \
    }())
  #define MAX(x)                                       \
    ([&]() -> decltype(auto) {                         \
      auto&& _cp_max_range = (x);                      \
      if (std::ranges::empty(_cp_max_range)) {         \
        my_assert(false && "MAX(): empty range.");     \
        std::abort();                                  \
      }                                                \
      return *std::ranges::max_element(_cp_max_range); \
    }())
#endif

// Y-combinator for recursive lambdas:
template <class F>
struct YCombinator {
  F fn;

  template <class... Args>
  decltype(auto) operator()(Args&&... args) const {
    return fn(*this, std::forward<Args>(args)...);
  }
};

template <class F>
YCombinator(F) -> YCombinator<F>;

template <class F>
[[gnu::always_inline]] constexpr auto fix(F&& fn) {
  return YCombinator<std::decay_t<F>>{std::forward<F>(fn)};
}
