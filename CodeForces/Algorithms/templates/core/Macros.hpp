#pragma once
#include "templates/core/Debug.hpp"
#include "templates/core/ScalarTypes.hpp"

#ifndef CP_ENABLE_SHORT_MACROS
  #define CP_ENABLE_SHORT_MACROS 1
#endif

//===----------------------------------------------------------------------===//
/* Advanced Macro System */

namespace cp {

namespace detail {

template <class T, bool = std::is_integral_v<std::remove_cvref_t<T>> &&
                              !std::is_same_v<std::remove_cvref_t<T>, bool>>
struct LoopArg {
  using type = std::remove_cvref_t<T>;
};

template <class T>
struct LoopArg<T, true> {
  using type = std::make_signed_t<std::remove_cvref_t<T>>;
};

} // namespace detail

template <class... Ts>
using LoopT = std::common_type_t<I32, typename detail::LoopArg<Ts>::type...>;

template <class... Ts>
struct Loop {
  using T = LoopT<Ts...>;

  template <class X>
  [[gnu::always_inline]] static constexpr T val(X x) noexcept {
    return x;
  }
};

template <class R>
[[gnu::always_inline]] constexpr I64 sz64(const R& x)
    noexcept(noexcept(std::ssize(x))) {
  return std::ssize(x);
}

template <class R>
[[gnu::always_inline]] constexpr I32 sz32(const R& x)
    noexcept(noexcept(std::ssize(x))) {
  return std::ssize(x);
}

} // namespace cp

#define CP_LOOP(...) cp::Loop<__VA_ARGS__>
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

// Advanced container operations:
namespace cp {

template <class R>
void unique_inplace(R& range) {
  std::ranges::sort(range);
  const auto tail = std::ranges::unique(range);
  range.erase(tail.begin(), tail.end());
  if constexpr (requires { range.shrink_to_fit(); })
    range.shrink_to_fit();
}

template <class R, class T>
I64 lower_bound_index(R&& range, const T& value) {
  return std::ranges::distance(std::ranges::begin(range), std::ranges::lower_bound(range, value));
}

template <class R, class T>
I64 upper_bound_index(R&& range, const T& value) {
  return std::ranges::distance(std::ranges::begin(range), std::ranges::upper_bound(range, value));
}

} // namespace cp

#define UNIQUE(x) cp::unique_inplace((x))
#define LB(c, x) cp::lower_bound_index((c), (x))
#define UB(c, x) cp::upper_bound_index((c), (x))

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
