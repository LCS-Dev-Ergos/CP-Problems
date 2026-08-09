//===----------------------------------------------------------------------===//
/**
 * @file: problem_B_sub.cpp
 * @generated: 2026-08-09 16:56:04
 * @source: problem_B.cpp
 * @author: C.L.
 *
 * @brief: Codeforces Round 1116 (Div. 1) - Problem B
 */
//===----------------------------------------------------------------------===//
/* Included library and Compiler Optimizations */

#ifndef CP_TEMPLATE_PROFILE_STRICT
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#define CP_IO_PROFILE_SIMPLE

//===----------------------------------------------------------------------===//
/* Compiler Pragmas */

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC optimize("O3,unroll-loops,inline-functions")
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-result"
  #if defined(__x86_64__) && !defined(__MINGW32__) && !defined(__MINGW64__)
    #pragma GCC target("avx2,bmi,bmi2,popcnt,lzcnt,sse4.2,fma")
  #endif
  #if defined(__aarch64__) && !defined(__MINGW32__) && !defined(__MINGW64__)
    #pragma GCC target("+simd,+crypto,+fp16")
  #endif
#endif

#ifdef __clang__
  #pragma clang optimize on
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wunused-result"
#endif

//===----------------------------------------------------------------------===//
/* Standard Library Includes */

#include <bits/stdc++.h>

//===----------------------------------------------------------------------===//
/* Toolchain Feature Detection */

#ifndef HAS_INT128
  #ifdef __SIZEOF_INT128__
    #define HAS_INT128 1
  #else
    #define HAS_INT128 0
  #endif
#endif

#ifndef HAS_FLOAT128
  #ifdef __FLOAT128__
    #define HAS_FLOAT128 1
  #else
    #define HAS_FLOAT128 0
  #endif
#endif

#if defined(__has_include) && __has_include(<ext/pb_ds/assoc_container.hpp>) \
 && __has_include(<ext/pb_ds/tree_policy.hpp>)
  #ifndef PBDS_AVAILABLE
    #define PBDS_AVAILABLE 1
  #endif
#elif !defined(PBDS_AVAILABLE)
  #define PBDS_AVAILABLE 0
#endif

//===----------------------------------------------------------------------===//
/* Debug and Assertion Policy */

#ifdef LOCAL
  #include "debug.h"
#else
  #define debug(...)
  #define debug_if(...)
  #define debug_tree(...)
  #define debug_tree_verbose(...)
  #define debug_line()
  #define my_assert(cond) ((void)0)
  #define COUNT_CALLS(...)
#endif

#ifdef __clang__
  #pragma clang diagnostic pop
#endif

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif

//===----------------------------------------------------------------------===//
/* Scalar Type Aliases */

using I8  = std::int8_t;
using I16 = std::int16_t;
using I32 = std::int32_t;
using I64 = std::int64_t;
using U8  = std::uint8_t;
using U16 = std::uint16_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;

#if HAS_INT128
__extension__ using I128 = __int128;
__extension__ using U128 = unsigned __int128;
#else
using I128 = std::int64_t;
using U128 = std::uint64_t;
#endif

using F32 = float;
using F64 = double;
using F80 = long double;

#if HAS_FLOAT128
using F128 = __float128;
#else
using F128 = long double;
#endif

template <typename T>
using Limits = std::numeric_limits<T>;

using Size = std::size_t;
using Diff = std::ptrdiff_t;
using Byte = std::byte;

//===----------------------------------------------------------------------===//
/* Extended Type Traits */

namespace cp {

template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

namespace detail {

template <class T>
inline constexpr bool is_extended_integral_v = false;

template <class T>
struct make_unsigned {
  using type = std::make_unsigned_t<T>;
};

template <>
struct make_unsigned<bool> {
  using type = U8;
};

#if HAS_INT128
template <>
inline constexpr bool is_extended_integral_v<I128> = true;
template <>
inline constexpr bool is_extended_integral_v<U128> = true;

template <>
struct make_unsigned<I128> {
  using type = U128;
};
template <>
struct make_unsigned<U128> {
  using type = U128;
};
#endif

} // namespace detail

template <class T>
using make_unsigned_t = typename detail::make_unsigned<remove_cvref_t<T>>::type;

namespace detail {

template <class T, bool = std::is_integral_v<remove_cvref_t<T>> && !std::is_same_v<remove_cvref_t<T>, bool>>
struct loop_arg {
  using type = remove_cvref_t<T>;
};

template <class T>
struct loop_arg<T, true> {
  using type = std::make_signed_t<remove_cvref_t<T>>;
};

} // namespace detail

template <class... Ts>
using loop_t = std::common_type_t<I32, typename detail::loop_arg<Ts>::type...>;

template <class... Ts>
struct loop {
  using T = loop_t<Ts...>;

  template <class X>
  [[gnu::always_inline]] static constexpr T val(X x) noexcept {
    return x;
  }
};

template <class R>
[[gnu::always_inline]] constexpr I64 sz64(const R& x) noexcept(noexcept(std::ssize(x))) {
  return std::ssize(x);
}

template <class R>
[[gnu::always_inline]] constexpr I32 sz32(const R& x) noexcept(noexcept(std::ssize(x))) {
  return std::ssize(x);
}

} // namespace cp

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
#define all(x) (x).begin(), (x).end()
#define rall(x) (x).rbegin(), (x).rend()
#define sz(x) cp::sz64(x)
#define isz(x) cp::sz32(x)
#define len(x) sz(x)
#define eb emplace_back
#define elif else if

// Advanced container operations:
#define UNIQUE(x) (std::ranges::sort(x), x.erase(std::ranges::unique(x).begin(), x.end()), x.shrink_to_fit())
#define LB(c, x) (I64) std::distance((c).begin(), std::ranges::lower_bound(c, x))
#define UB(c, x) (I64) std::distance((c).begin(), std::ranges::upper_bound(c, x))
#define SUM(x) std::accumulate(all(x), std::iter_value_t<decltype((x).begin())>{})
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

// Type-safe cast alias:
template <typename To>
[[gnu::always_inline]] constexpr To as(auto&& x) noexcept {
  return static_cast<To>(std::forward<decltype(x)>(x));
}

//===----------------------------------------------------------------------===//
/* Lightweight Stopwatch Utility */

struct Stopwatch {
  using Clock = std::chrono::high_resolution_clock;
  Clock::time_point start;

  Stopwatch() : start(Clock::now()) {}
  void reset() { start = Clock::now(); }
  [[gnu::always_inline]] F64 elapsed() const { return std::chrono::duration<F64>(Clock::now() - start).count(); }
  [[gnu::always_inline]] bool within(F64 limit) const { return elapsed() < limit; }
};

//===----------------------------------------------------------------------===//
/* Container and Utility Aliases */

using String     = std::string;
using StringView = std::string_view;

template <class T>
using Vec = std::vector<T>;
template <class T>
using Deque = std::deque<T>;
template <class T>
using List = std::list<T>;
template <class T, Size N>
using Array = std::array<T, N>;
template <Size N>
using BitSet = std::bitset<N>;

template <class T>
using Set = std::set<T>;
template <class T>
using MultiSet = std::multiset<T>;
template <class K, class V>
using Map = std::map<K, V>;
template <class K, class V>
using MultiMap = std::multimap<K, V>;
template <class T>
using UnorderedSet = std::unordered_set<T>;
template <class K, class V>
using UnorderedMap = std::unordered_map<K, V>;

template <class T, class Compare>
using OrderedSetBy = std::set<T, Compare>;
template <class T, class Compare>
using OrderedMultiSetBy = std::multiset<T, Compare>;
template <class K, class V, class Compare>
using OrderedMapBy = std::map<K, V, Compare>;
template <class K, class V, class Compare>
using OrderedMultiMapBy = std::multimap<K, V, Compare>;
template <class T, class Hash, class Eq = std::equal_to<T>>
using HashedSetBy = std::unordered_set<T, Hash, Eq>;
template <class K, class V, class Hash, class Eq = std::equal_to<K>>
using HashedMapBy = std::unordered_map<K, V, Hash, Eq>;

template <class T>
using Stack = std::stack<T, std::deque<T>>;
template <class T>
using Queue = std::queue<T, std::deque<T>>;
template <class T>
using PriorityQueue = std::priority_queue<T, std::vector<T>>;
template <class T>
using MinPriorityQueue = std::priority_queue<T, std::vector<T>, std::greater<T>>;

template <class T, class Container>
using StackIn = std::stack<T, Container>;
template <class T, class Container>
using QueueIn = std::queue<T, Container>;
template <class T, class Container, class Compare>
using PriorityQueueBy = std::priority_queue<T, Container, Compare>;
template <class T, class Container = std::vector<T>>
using MinPriorityQueueIn = std::priority_queue<T, Container, std::greater<T>>;

template <class T, class U>
using Pair = std::pair<T, U>;
template <class... Args>
using Tuple = std::tuple<Args...>;
template <class T>
using Optional = std::optional<T>;
template <class... Ts>
using Variant = std::variant<Ts...>;
template <class Signature>
using Function = std::function<Signature>;
template <class T>
using Span = std::span<T>;

template <class T>
using Vec2D = Vec<Vec<T>>;
template <class T>
using Vec3D = Vec<Vec2D<T>>;
template <class T>
using Vec4D = Vec<Vec3D<T>>;

using VecU8   = Vec<U8>;
using VecU16  = Vec<U16>;
using VecU32  = Vec<U32>;
using VecU64  = Vec<U64>;
using VecI8   = Vec<I8>;
using VecI16  = Vec<I16>;
using VecI32  = Vec<I32>;
using VecI64  = Vec<I64>;
using VecF64  = Vec<F64>;
using VecF80  = Vec<F80>;
using VecBool = Vec<bool>;
using VecStr  = Vec<String>;

template <class T, class U>
using VecPair = Vec<Pair<T, U>>;

using PairI32    = Pair<I32, I32>;
using PairI64    = Pair<I64, I64>;
using PairF80    = Pair<F80, F80>;
using VecPairI32 = Vec<PairI32>;
using VecPairI64 = Vec<PairI64>;

//===----------------------------------------------------------------------===//
/* Lightweight I/O Utilities */

namespace cp_io {

inline void setup() {
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  std::cout << std::fixed << std::setprecision(10);
}

struct IOSetup {
  IOSetup() { setup(); }
};

inline IOSetup io_setup;

template <class T>
void read(T& x) { std::cin >> x; }

template <class T>
void write(const T& x) { std::cout << x; }

#define CP_IO_COMPOSITE_CONTEXT 1
template <class T, class U>
inline void read(Pair<T, U>& p) {
  read(p.first);
  read(p.second);
}

template <class T>
inline void read(Vec<T>& v) {
  for (auto& x : v)
    read(x);
}

template <typename... Args>
inline void read(std::tuple<Args...>& t) {
  std::apply([](auto&... args) { (read(args), ...); }, t);
}

template <class T, class U>
inline void write(const Pair<T, U>& p) {
  write(p.first);
  write(' ');
  write(p.second);
}

template <class T>
inline void write(const Vec<T>& v) {
  for (I64 i = 0; i < sz(v); ++i) {
    if (i)
      write(' ');
    write(v[i]);
  }
}

template <typename... Args>
inline void write(const std::tuple<Args...>& t) {
  I32 i = 0;
  std::apply([&i](const auto&... args) { ((i++ > 0 ? (write(' '), 0) : 0, write(args)), ...); }, t);
}
#undef CP_IO_COMPOSITE_CONTEXT

template <class Head, class... Tail>
  requires(sizeof...(Tail) > 0)
inline void read(Head& head, Tail&... tail) {
  read(head);
  read(tail...);
}

template <class Head, class... Tail>
  requires(sizeof...(Tail) > 0)
inline void write(const Head& head, const Tail&... tail) {
  write(head);
  write(' ');
  write(tail...);
}

inline void writeln() { write('\n'); }

template <class... Args>
inline void writeln(const Args&... args) {
  if constexpr (sizeof...(args) > 0)
    write(args...);
  writeln();
}

} // namespace cp_io

namespace fast_io {
#define CP_IO_COMPAT_FAST_IO_NAMESPACE_DEFINED 1

template <class T>
inline void read_integer(T& x) { cp_io::read(x); }
inline void read_char(char& x) { cp_io::read(x); }
inline void read_string(std::string& x) { cp_io::read(x); }

template <class T>
inline void write_integer(T x) { cp_io::write(x); }
inline void write_char(char c) { std::cout.put(c); }
inline void write_string(const std::string& s) { cp_io::write(s); }
inline void flush_output() { std::cout.flush(); }

using cp_io::read;
using cp_io::write;
using cp_io::writeln;
} // namespace fast_io

#define CP_IO_IMPL_READ(...) cp_io::read(__VA_ARGS__)
#define CP_IO_IMPL_WRITELN(...) cp_io::writeln(__VA_ARGS__)
#define CP_IO_IMPL_FLUSH() std::cout.flush()

//===----------------------------------------------------------------------===//
/* Shared I/O Macro and Answer Helper Definitions */

#define IN(...) CP_IO_IMPL_READ(__VA_ARGS__)
#define OUT(...) CP_IO_IMPL_WRITELN(__VA_ARGS__)
#define FLUSH() CP_IO_IMPL_FLUSH()

#define CP_IO_DECL_MACROS_DEFINED 1
#define INT(...)   \
  I32 __VA_ARGS__; \
  IN(__VA_ARGS__)
#define LL(...)    \
  I64 __VA_ARGS__; \
  IN(__VA_ARGS__)
#define ULL(...)   \
  U64 __VA_ARGS__; \
  IN(__VA_ARGS__)
#define STR(...)           \
  std::string __VA_ARGS__; \
  IN(__VA_ARGS__)
#define CHR(...)    \
  char __VA_ARGS__; \
  IN(__VA_ARGS__)
#define DBL(...)   \
  F64 __VA_ARGS__; \
  IN(__VA_ARGS__)

#define CP_IO_ANSWER_HELPERS_DEFINED 1
inline void YES(bool condition = true) {
  if (condition)
    OUT("YES");
  else
    OUT("NO");
}
inline void NO(bool condition = true) { YES(!condition); }
inline void Yes(bool condition = true) {
  if (condition)
    OUT("Yes");
  else
    OUT("No");
}
inline void No(bool condition = true) { Yes(!condition); }

//===----------------------------------------------------------------------===//
/* Library Function Aliases */

namespace cp {

template <bool B, class T, class F>
using Conditional = std::conditional_t<B, T, F>;

template <class T>
using cvref_t = std::remove_cvref_t<T>;

template <class T, class U>
concept Same = std::same_as<cvref_t<T>, cvref_t<U>>;

// clang-format off
template <class T>
concept Int = std::integral<cvref_t<T>>
#if HAS_INT128
  || std::same_as<cvref_t<T>, I128>
  || std::same_as<cvref_t<T>, U128>
#endif
    ;

template <class T>
concept Float = std::floating_point<cvref_t<T>>;

template <class T>
concept Signed = Int<T> && (std::is_signed_v<cvref_t<T>>
#if HAS_INT128
  || std::same_as<cvref_t<T>, I128>
#endif
);
// clang-format on

template <class T>
concept Unsigned = Int<T> && !Signed<T>;

template <class T>
concept Enum = std::is_enum_v<cvref_t<T>>;

template <class F, class... Args>
concept Predicate = std::predicate<F, Args...>;

template <class T>
concept Hashable = requires(const cvref_t<T>& value) {
  { std::hash<cvref_t<T>>{}(value) } -> std::convertible_to<std::size_t>;
};

} // namespace cp

//===----------------------------------------------------------------------===//
/* Integer Mathematical Utilities */

namespace cp::detail {

template <cp::Unsigned T>
[[gnu::always_inline]] constexpr T mul_mod_unsigned(T a, T b, T mod) {
#if HAS_INT128
  if constexpr (sizeof(T) <= sizeof(U64)) {
    return as<T>((as<U128>(a) * as<U128>(b)) % as<U128>(mod));
  }
#endif

  T result = 0;
  while (b > 0) {
    if ((b & 1U) != 0U) {
      if (result >= mod - a)
        result -= (mod - a);
      else
        result += a;
    }
    b >>= 1U;
    if (b == 0)
      break;
    if (a >= mod - a)
      a -= (mod - a);
    else
      a += a;
  }
  return result;
}

} // namespace cp::detail

template <cp::Signed T>
[[gnu::always_inline]] constexpr T safe_mod(T x, T mod) {
  my_assert(mod > 0);
  x %= mod;
  if (x < 0)
    x += mod;
  return x;
}

template <cp::Signed T>
[[gnu::always_inline]] constexpr std::pair<T, T> inv_gcd(T a, T b) {
  my_assert(b > 0);
  a = safe_mod(a, b);
  if (a == 0)
    return {b, 0};

  T s = b, t = a;
  T m0 = 0, m1 = 1;

  while (t) {
    T u = s / t;
    s -= t * u;
    m0 -= m1 * u;
    std::swap(s, t);
    std::swap(m0, m1);
  }

  if (m0 < 0)
    m0 += b / s;
  return {s, m0};
}

template <cp::Signed T>
[[gnu::always_inline]] constexpr T mod_inv(T a, T mod) {
  auto [g, x] = inv_gcd(a, mod);
  my_assert(g == 1 && "mod_inv(): inverse does not exist when gcd(value, mod) != 1.");
  return g == 1 ? x : 0;
}

template <cp::Signed T>
[[gnu::always_inline]] constexpr bool merge_congruences(T& r1, T& m1, T r2, T m2) {
  my_assert(m1 > 0 && m2 > 0);
  r1 = safe_mod(r1, m1);
  r2 = safe_mod(r2, m2);

  T target    = safe_mod(r2 - r1, m2);
  auto [g, x] = inv_gcd(m1, m2);
  if (target % g != 0)
    return false;

  const T m2_g = m2 / g;
  const T lhs  = safe_mod(target / g, m2_g);
  const T rhs  = safe_mod(x, m2_g);
  using U      = cp::make_unsigned_t<T>;
  const T step = as<T>(cp::detail::mul_mod_unsigned(as<U>(lhs), as<U>(rhs), as<U>(m2_g)));

  r1 += step * m1;
  m1 *= m2_g;
  r1 = safe_mod(r1, m1);
  return true;
}

template <cp::Int T>
[[gnu::always_inline]] constexpr T div_floor(T a, T b) {
  my_assert(b != 0);
  if constexpr (cp::Signed<T>) {
    T q = a / b;
    T r = a % b;
    if (r != 0 && ((r > 0) != (b > 0)))
      --q;
    return q;
  } else {
    return a / b;
  }
}

template <cp::Int T>
[[gnu::always_inline]] constexpr T div_ceil(T a, T b) {
  my_assert(b != 0);
  if constexpr (cp::Signed<T>) {
    T q = a / b;
    T r = a % b;
    if (r != 0 && ((r > 0) == (b > 0)))
      ++q;
    return q;
  } else {
    return a / b + (a % b != 0);
  }
}

template <cp::Int T>
[[gnu::always_inline]] constexpr T mod_floor(T a, T b) {
  return a - b * div_floor(a, b);
}

template <cp::Int T>
[[gnu::always_inline]] constexpr std::pair<T, T> divmod(T a, T b) {
  T q = div_floor(a, b);
  return {q, a - q * b};
}

template <cp::Int T>
[[gnu::always_inline]] constexpr T power(T base, T exp) {
  T result = 1;
  while (exp > 0) {
    if (exp & 1)
      result *= base;
    base *= base;
    exp >>= 1;
  }
  return result;
}

template <cp::Int T>
[[gnu::always_inline]] constexpr T mod_pow(T base, T exp, T mod) {
  my_assert(mod != 0);
  if constexpr (cp::Signed<T>) {
    my_assert(mod > 0);
    my_assert(exp >= 0);
    if (mod <= 0)
      return 0;
    if (exp < 0)
      return 0;
  }

  using U  = cp::make_unsigned_t<T>;
  U umod   = as<U>(mod);
  U uexp   = as<U>(exp);
  U ubase  = as<U>(mod_floor(base, mod));
  U result = as<U>(1) % umod;

  while (uexp > 0) {
    if ((uexp & 1U) != 0U) {
      result = cp::detail::mul_mod_unsigned(result, ubase, umod);
    }
    ubase = cp::detail::mul_mod_unsigned(ubase, ubase, umod);
    uexp >>= 1U;
  }
  return as<T>(result);
}

template <cp::Int T>
[[gnu::always_inline]] inline T floor_sqrt(T x) {
  if constexpr (cp::Signed<T>) {
    my_assert(x >= 0);
    if (x < 0)
      return 0;
  }

  using U    = cp::make_unsigned_t<T>;
  const U ux = as<U>(x);
  if (ux <= 1)
    return as<T>(ux);

  if constexpr (sizeof(U) > sizeof(U64)) {
    constexpr int bits = int(sizeof(U) * 8);
    U lo = 1, hi = U{1} << ((bits + 1) / 2);
    while (lo + 1 < hi) {
      const U mid = lo + (hi - lo) / 2;
      if (mid <= ux / mid)
        lo = mid;
      else
        hi = mid;
    }
    return as<T>(lo);
  }

  U r = as<U>(std::sqrt(as<F80>(ux)));
  while ((r + 1) <= ux / (r + 1))
    ++r;
  while (r > ux / r)
    --r;
  return as<T>(r);
}

template <cp::Int T>
[[gnu::always_inline]] inline T ceil_sqrt(T x) {
  using U      = cp::make_unsigned_t<T>;
  const T root = floor_sqrt(x);
  const U uf   = as<U>(root);
  if (uf == 0)
    return 0;

  const U ux = as<U>(x);
  if (ux / uf == uf && ux % uf == 0)
    return root;
  return as<T>(uf + 1);
}

//===----------------------------------------------------------------------===//
/* Mathematical Constants and Infinity Values */

// High-precision mathematical constants:
constexpr F80 PI   = 3.1415926535897932384626433832795028841971693993751L;
constexpr F80 E    = 2.7182818284590452353602874713526624977572470937000L;
constexpr F80 PHI  = 1.6180339887498948482045868343656381177203091798058L;
constexpr F80 LN2  = 0.6931471805599453094172321214581765680755001343602L;
constexpr F80 EPS  = 1e-9L;
constexpr F80 DEPS = 1e-12L;

// Robust infinity system:
template <class T>
constexpr T infinity = std::numeric_limits<T>::max() / 4;

template <class T>
constexpr T neg_infinity = std::numeric_limits<T>::lowest() / 4;

template <>
inline constexpr I32 infinity<I32> = 1'010'000'000;
template <>
inline constexpr I64 infinity<I64> = 2'020'000'000'000'000'000LL;
template <>
inline constexpr U32 infinity<U32> = 2'020'000'000U;
template <>
inline constexpr U64 infinity<U64> = 4'040'000'000'000'000'000ULL;
template <>
inline constexpr F64 infinity<F64> = 1e18;
template <>
inline constexpr F80 infinity<F80> = 1e18L;

template <>
inline constexpr I32 neg_infinity<I32> = -infinity<I32>;
template <>
inline constexpr I64 neg_infinity<I64> = -infinity<I64>;
template <>
inline constexpr U32 neg_infinity<U32> = 0U;
template <>
inline constexpr U64 neg_infinity<U64> = 0ULL;
template <>
inline constexpr F64 neg_infinity<F64> = -infinity<F64>;
template <>
inline constexpr F80 neg_infinity<F80> = -infinity<F80>;

#if HAS_INT128
static_assert(sizeof(I128) > sizeof(I64), "I128 must be true 128-bit when HAS_INT128 is enabled.");
template <>
inline constexpr I128 infinity<I128> = I128(infinity<I64>) * 2'000'000'000'000'000'000LL;
template <>
inline constexpr I128 neg_infinity<I128> = -infinity<I128>;
#endif

constexpr I32 INF32  = infinity<I32>;
constexpr I64 INF64  = infinity<I64>;
constexpr I64 LINF   = INF64; // Legacy alias
constexpr I32 NINF32 = neg_infinity<I32>;
constexpr I64 NINF64 = neg_infinity<I64>;
constexpr I64 NLINF  = NINF64; // Legacy-style alias

// Powers of ten lookup table (10^k for k = 0..18):
constexpr I64 POW10[] = {
    1LL,
    10LL,
    100LL,
    1000LL,
    10000LL,
    100000LL,
    1000000LL,
    10000000LL,
    100000000LL,
    1000000000LL,
    10000000000LL,
    100000000000LL,
    1000000000000LL,
    10000000000000LL,
    100000000000000LL,
    1000000000000000LL,
    10000000000000000LL,
    100000000000000000LL,
    1000000000000000000LL,
};

// Modular arithmetic constants:
constexpr I64 MOD  = 1'000'000'007LL;
constexpr I64 MOD2 = 998'244'353LL;
constexpr I64 MOD3 = 1'000'000'009LL;
constexpr I64 INV2 = (MOD + 1) / 2;

//===----------------------------------------------------------------------===//
/* Advanced Modular Arithmetic */

struct Barrett {
  U32 m;
  U64 im;

  explicit constexpr Barrett(U32 mod) : m(mod), im(U64(-1) / mod + 1) {}

  constexpr U32 mod() const { return m; }

  constexpr U32 mul(U32 a, U32 b) const {
#if HAS_INT128
    U64 z = U64(a) * b;
    U64 x = U64((U128(z) * im) >> 64);
    U32 v = U32(z - x * m);
    if (m <= v)
      v += m;
    return v;
#else
    U64 res = U64(a) * b - U64(1.L * a * b / m - 0.5L) * m;
    res %= m;
    return U32(res);
#endif
  }
};

namespace cp::modint_detail {
// Implementation layer: keep policy/base machinery out of the contest API.

template <cp::Int T>
using SignedWide =
#if HAS_INT128
    Conditional<cp::Signed<T>, I128, U128>;
#else
    Conditional<cp::Signed<T>, I64, U64>;
#endif

template <I64 MOD>
struct StaticModPolicy {
  static_assert(MOD > 0, "ModInt requires MOD > 0.");
  using Value = U64;

  static constexpr I64 mod() { return MOD; }

  template <cp::Int T>
  static constexpr Value normalize(T x) {
    using Wide = SignedWide<T>;
    Wide r     = as<Wide>(x) % as<Wide>(MOD);
    if constexpr (cp::Signed<T>) {
      if (r < 0)
        r += as<Wide>(MOD);
    }
    return as<Value>(r);
  }

  static constexpr Value add(Value a, Value b) {
    a += b;
    if (a >= as<Value>(MOD))
      a -= as<Value>(MOD);
    return a;
  }

  static constexpr Value sub(Value a, Value b) { return a < b ? a + as<Value>(MOD) - b : a - b; }

  static constexpr Value mul(Value a, Value b) {
#if HAS_INT128
    return as<Value>(U128(a) * b % U128(MOD));
#else
    static_assert(MOD <= (1LL << 32), "ModInt multiplication may overflow U64 for MOD > 2^32 without __int128 support.");
    return a * b % as<Value>(MOD);
#endif
  }

  static constexpr Value inv(Value x) {
    // mod_inv already returns a canonical residue in [0, MOD).
    return as<Value>(mod_inv(as<I64>(x), MOD));
  }
};

template <U32 Id>
struct DynamicModPolicy {
  using Value              = U32;
  inline static Barrett bt = Barrett(998'244'353);

  static void set_mod(U32 mod) {
    my_assert(mod > 0);
    bt = Barrett(mod);
  }

  static U32 mod() { return bt.mod(); }

  template <cp::Int T>
  static Value normalize(T x) {
    const U32 m = mod();
    using Wide  = SignedWide<T>;
    Wide r      = as<Wide>(x) % as<Wide>(m);
    if constexpr (cp::Signed<T>) {
      if (r < 0)
        r += m;
    }
    return as<Value>(r);
  }

  static Value add(Value a, Value b) {
    const Value m = mod();
    return a >= m - b ? a - (m - b) : a + b;
  }

  static Value sub(Value a, Value b) {
    const Value m = mod();
    return a < b ? a + (m - b) : a - b;
  }

  static Value mul(Value a, Value b) { return bt.mul(a, b); }

  // mod_inv already returns a canonical residue in [0, mod()).
  static Value inv(Value x) { return as<Value>(mod_inv(as<I64>(x), as<I64>(mod()))); }
};

template <class Derived, class Policy>
struct ModIntBase {
  using Value = typename Policy::Value;
  Value value = 0;

  constexpr ModIntBase() = default;

  template <cp::Int T>
  constexpr ModIntBase(T x) : value(Policy::normalize(x)) {}

  constexpr Value val() const { return value; }

  static constexpr auto mod() { return Policy::mod(); }

  constexpr Derived& self() { return static_cast<Derived&>(*this); }

  constexpr const Derived& self() const { return static_cast<const Derived&>(*this); }

  constexpr Derived operator-() const { return Derived(value == 0 ? 0 : Policy::sub(0, value)); }

  constexpr Derived& operator+=(const Derived& other) & {
    value = Policy::add(value, other.value);
    return self();
  }

  constexpr Derived& operator-=(const Derived& other) & {
    value = Policy::sub(value, other.value);
    return self();
  }

  constexpr Derived& operator*=(const Derived& other) & {
    value = Policy::mul(value, other.value);
    return self();
  }

  constexpr Derived& operator/=(const Derived& other) & { return *this *= other.inverse(); }

  friend constexpr Derived operator+(Derived lhs, const Derived& rhs) {
    lhs += rhs;
    return lhs;
  }

  friend constexpr Derived operator-(Derived lhs, const Derived& rhs) {
    lhs -= rhs;
    return lhs;
  }

  friend constexpr Derived operator*(Derived lhs, const Derived& rhs) {
    lhs *= rhs;
    return lhs;
  }

  friend constexpr Derived operator/(Derived lhs, const Derived& rhs) {
    lhs /= rhs;
    return lhs;
  }

  constexpr Derived pow(I64 exp) const {
    Derived result(1), base(self());
    while (exp > 0) {
      if (exp & 1)
        result *= base;
      base *= base;
      exp >>= 1;
    }
    return result;
  }

  explicit constexpr operator I64() const { return as<I64>(value); }

  constexpr Derived inverse() const { return Derived(Policy::inv(value)); }

  friend constexpr bool operator==(const Derived& lhs, const Derived& rhs) { return lhs.value == rhs.value; }

  friend constexpr std::strong_ordering operator<=>(const Derived& lhs, const Derived& rhs) {
    return lhs.value <=> rhs.value;
  }

  friend std::ostream& operator<<(std::ostream& os, const Derived& x) { return os << x.value; }

  friend std::istream& operator>>(std::istream& is, Derived& x) {
    I64 val;
    is >> val;
    x = Derived(val);
    return is;
  }
};

} // namespace cp::modint_detail

template <I64 MOD>
struct ModInt : cp::modint_detail::ModIntBase<ModInt<MOD>, cp::modint_detail::StaticModPolicy<MOD>> {
  using Base = cp::modint_detail::ModIntBase<ModInt<MOD>, cp::modint_detail::StaticModPolicy<MOD>>;
  using Base::Base;
};

template <U32 Id>
struct DynModInt : cp::modint_detail::ModIntBase<DynModInt<Id>, cp::modint_detail::DynamicModPolicy<Id>> {
  using Base = cp::modint_detail::ModIntBase<DynModInt<Id>, cp::modint_detail::DynamicModPolicy<Id>>;
  using Base::Base;

  static void set_mod(U32 mod) { cp::modint_detail::DynamicModPolicy<Id>::set_mod(mod); }
};

using MInt  = ModInt<MOD>;
using MInt2 = ModInt<MOD2>;
using DMInt = DynModInt<0>;

//===----------------------------------------------------------------------===//
/* Main Solver Function */

using namespace std;

constexpr I32 MAXN = 1'000'000;

Vec<MInt2> fact(MAXN + 1);
Vec<MInt2> ifact(MAXN + 1);

void init_comb() {
  fact[0] = 1;
  FOR(i, 1, MAXN + 1)
  fact[i] = fact[i - 1] * i;

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
    if (i == 0 || s[i] != s[i - 1])
      ++runs[c];
  }

  auto ways = [&](I32 c) -> MInt2 {
    if (cnt[c] == 0)
      return 1;
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
  FOR(T)
  solve();

  return 0;
}

//===----------------------------------------------------------------------===//
