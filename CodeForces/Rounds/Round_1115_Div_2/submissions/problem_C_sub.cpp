//===----------------------------------------------------------------------===//
/**
 * @file: problem_C_sub.cpp
 * @generated: 2026-08-11 14:34:00
 * @source: problem_C.cpp
 * @author: C.L.
 *
 * @brief: Codeforces Round 1115 (Div. 2) - Problem C
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
// Separate opt-in: fast-math drops NaN/Inf and reassociates FP.
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

// Config.hpp already downgraded the flag when <bits/stdc++.h> is unreachable.
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

// __SIZEOF_FLOAT128__ is GCC's spelling, __FLOAT128__ is Clang's.
#ifndef HAS_FLOAT128
  #if defined(__SIZEOF_FLOAT128__) || defined(__FLOAT128__)
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
/* Extended Type Traits */

template <typename To>
[[gnu::always_inline]] constexpr To as(auto&& x) noexcept {
  return static_cast<To>(std::forward<decltype(x)>(x));
}

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
struct LoopArg {
  using type = remove_cvref_t<T>;
};

template <class T>
struct LoopArg<T, true> {
  using type = std::make_signed_t<remove_cvref_t<T>>;
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
[[gnu::always_inline]] constexpr I64 sz64(const R& x) noexcept(noexcept(std::ssize(x))) {
  return std::ssize(x);
}

template <class R>
[[gnu::always_inline]] constexpr I32 sz32(const R& x) noexcept(noexcept(std::ssize(x))) {
  return std::ssize(x);
}

} // namespace cp

#define CP_ENABLE_SHORT_MACROS 1

//===----------------------------------------------------------------------===//
/* Advanced Macro System */

// Advanced FOR loop system:
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

//===----------------------------------------------------------------------===//
/* Lightweight Stopwatch Utility */

struct Stopwatch {
  // Not high_resolution_clock: it may alias system_clock and jump backwards.
  using Clock = std::chrono::steady_clock;
  Clock::time_point start;

  Stopwatch() : start(Clock::now()) {}
  void reset() { start = Clock::now(); }
  [[gnu::always_inline]] F64 elapsed() const { return std::chrono::duration<F64>(Clock::now() - start).count(); }
  [[gnu::always_inline]] bool within(F64 limit) const { return elapsed() < limit; }
};

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

//===----------------------------------------------------------------------===//
/* Composite I/O Overloads */

template <class T, class U>
void read(std::pair<T, U>&);
template <class T, class A>
void read(std::vector<T, A>&);
template <class... Args>
void read(std::tuple<Args...>&);

template <class T, class U>
void write(const std::pair<T, U>&);
template <class T, class A>
void write(const std::vector<T, A>&);
template <class... Args>
void write(const std::tuple<Args...>&);

template <class T, class U>
inline void read(std::pair<T, U>& p) {
  read(p.first);
  read(p.second);
}

template <class T, class A>
inline void read(std::vector<T, A>& v) {
  for (auto& x : v)
    read(x);
}

template <class... Args>
inline void read(std::tuple<Args...>& t) {
  std::apply([](auto&... args) { (read(args), ...); }, t);
}

template <class T, class U>
inline void write(const std::pair<T, U>& p) {
  write(p.first);
  write(' ');
  write(p.second);
}

template <class T, class A>
inline void write(const std::vector<T, A>& v) {
  for (std::size_t i = 0; i < v.size(); ++i) {
    if (i)
      write(' ');
    write(v[i]);
  }
}

template <class... Args>
inline void write(const std::tuple<Args...>& t) {
  bool first = true;
  std::apply(
      [&first](const auto&... args) {
        ((first ? (first = false, void()) : write(' '), write(args)), ...);
      },
      t);
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
/* Main Solver Function */

using namespace std;

void solve() {
  INT(n, m);
  VecI64 v(n);
  VecI32 a(n * m);
  IN(v, a);

  MultiSet<I32> top;
  I32 ans = m;

  FOR_R(i, n) {
    FOR(j, m) {
      I32 x = a[i * m + j];
      if (isz(top) < m) {
        top.insert(x);
      } else if (x > *top.begin()) {
        top.erase(top.begin());
        top.insert(x);
      }
    }

    I64 sum = 0;
    I32 cnt = 0;
    for (auto it = top.rbegin(); it != top.rend() && cnt < ans; ++it) {
      sum += *it;
      ++cnt;
      if (sum >= v[i]) {
        ans = cnt;
        break;
      }
    }
  }

  OUT(ans);
}

//===----------------------------------------------------------------------===//
/* Main Function */

auto main() -> int {
#ifdef LOCAL
  Stopwatch timer;
#endif

  INT(T);
  FOR(T)
  solve();

  return 0;
}

//===----------------------------------------------------------------------===//
