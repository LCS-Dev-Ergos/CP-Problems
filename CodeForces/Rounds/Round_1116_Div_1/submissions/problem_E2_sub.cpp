//===----------------------------------------------------------------------===//
/**
 * @file: problem_E2_sub.cpp
 * @generated: 2026-08-09 18:48:40
 * @source: problem_E2.cpp
 * @author: C.L.
 *
 * @brief: Codeforces Round 1116 (Div. 1) - Problem E2
 */
//===----------------------------------------------------------------------===//
/* Included library and Compiler Optimizations */

#ifndef CP_TEMPLATE_PROFILE_STRICT
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#define CP_IO_PROFILE_FAST_MINIMAL

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

// Minimal Fast_IO shim: selects the no-extension variant of Fast_IO.hpp.
#define CP_FAST_IO_VARIANT 0

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
/* Buffered I/O (variant-driven) */

template <I64 MOD>
struct ModInt;

namespace cp {
template <class T, class Tag>
class StrongType;
} // namespace cp

namespace fast_io {

template <class T>
concept FastIntegral = cp::Int<T> && !cp::Same<T, bool> && !cp::Same<T, char>;

template <class T>
concept FastFloating = cp::Float<T>;

template <class T>
inline void read_integer(T&);

template <class T>
inline void write_integer(T);

} // namespace fast_io

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

namespace fast_io {

static constexpr U32 BUFFER_SIZE = 1U << 20; // 1 MB
alignas(64) inline char input_buffer[BUFFER_SIZE];
alignas(64) inline char output_buffer[BUFFER_SIZE];
alignas(64) inline char number_buffer[160];

struct FourDigitTable {
  char digits[10'000][4];

  constexpr FourDigitTable() : digits{} {
    for (U32 value = 0; value < 10'000; ++value) {
      U32 x = value;
      for (I32 pos = 3; pos >= 0; --pos) {
        digits[value][pos] = as<char>('0' + (x % 10));
        x /= 10;
      }
    }
  }
};

inline constexpr FourDigitTable four_digit_table{};

inline U32 input_pos  = 0;
inline U32 input_end  = 0;
inline U32 output_pos = 0;

/* ------------------------------- INPUT API -------------------------------- */

inline void load_input() {
  // Full reload: assumes any single token fits within BUFFER_SIZE.
  input_end = as<U32>(std::fread(input_buffer, 1, BUFFER_SIZE, stdin));
  input_pos = 0;
}

[[gnu::always_inline]] inline char next_input_char() {
  if (input_pos >= input_end) {
    load_input();
    if (input_pos >= input_end)
      return 0;
  }
  return input_buffer[input_pos++];
}

inline void flush_output() {
  if (output_pos == 0)
    return;
  std::fwrite(output_buffer, 1, output_pos, stdout);
  output_pos = 0;
}

inline void read_char(char& c) {
  do {
    c = next_input_char();
  } while (c <= ' ' && c != 0);
}

template <typename T>
inline void read_integer(T& x) {
  char c;
  do {
    c = next_input_char();
  } while (c <= ' ' && c != 0);

  bool negative = false;
  if constexpr (cp::Signed<T>) {
    if (c == '-') {
      negative = true;
      c        = next_input_char();
    }
  }

  x = 0;
  while (c > ' ') {
    x = x * 10 + (c - '0');
    c = next_input_char();
  }

  if constexpr (cp::Signed<T>) {
    if (negative)
      x = -x;
  }
}

inline void read_string(std::string& s) {
  s.clear();
  s.reserve(32);
  char c;
  do {
    c = next_input_char();
  } while (c <= ' ' && c != 0);

  while (c > ' ') {
    s.push_back(c);
    c = next_input_char();
  }
}

template <typename T>
inline void read_floating(T& x) {
  char token[64];
  U32 len = 0;
  char c;
  do {
    c = next_input_char();
  } while (c <= ' ' && c != 0);
  while (c > ' ' && len + 1 < sizeof(token)) {
    token[len++] = c;
    c            = next_input_char();
  }
  token[len] = '\0';

  char* end = nullptr;
  if constexpr (cp::Same<T, F32>)
    x = std::strtof(token, &end);
  else if constexpr (cp::Same<T, F64>)
    x = std::strtod(token, &end);
  else
    x = std::strtold(token, &end);
}

template <FastIntegral T>
inline void read(T& x) { read_integer(x); }

template <FastFloating T>
inline void read(T& x) { read_floating(x); }
inline void read(char& x) { read_char(x); }
inline void read(std::string& x) { read_string(x); }

/* ------------------------------- OUTPUT API ------------------------------- */

template <typename T>
inline void write_integer(T x) {
  using UnsignedT = cp::make_unsigned_t<T>;
  UnsignedT ux;
  if constexpr (cp::Signed<T>) {
    ux = x < 0 ? as<UnsignedT>(-(x + 1)) + 1 : as<UnsignedT>(x);
  } else {
    ux = as<UnsignedT>(x);
  }

  I32 begin = I32(sizeof(number_buffer));
  while (ux >= 10'000) {
    const U32 chunk = as<U32>(ux % 10'000);
    ux /= 10'000;
    begin -= 4;
    std::memcpy(number_buffer + begin, four_digit_table.digits[chunk], 4);
  }

  const U32 head = as<U32>(ux);
  if (head >= 1'000) {
    begin -= 4;
    std::memcpy(number_buffer + begin, four_digit_table.digits[head], 4);
  } else if (head >= 100) {
    begin -= 3;
    std::memcpy(number_buffer + begin, four_digit_table.digits[head] + 1, 3);
  } else if (head >= 10) {
    begin -= 2;
    number_buffer[begin]     = as<char>('0' + head / 10);
    number_buffer[begin + 1] = as<char>('0' + head % 10);
  } else {
    number_buffer[--begin] = as<char>('0' + head);
  }

  if constexpr (cp::Signed<T>) {
    if (x < 0)
      number_buffer[--begin] = '-';
  }

  [[assume(begin >= 0)]];
  const U32 len = as<U32>(I32(sizeof(number_buffer)) - begin);
  if (output_pos + len >= BUFFER_SIZE)
    flush_output();
  std::memcpy(output_buffer + output_pos, number_buffer + begin, len);
  output_pos += len;
}

#define CP_FLOAT_PRECISION 10

template <typename T>
inline void write_floating(T x) {
  char local_buffer[128];
  const int n = std::snprintf(local_buffer, sizeof(local_buffer), "%.*Lf", CP_FLOAT_PRECISION, as<F80>(x));
  if (n <= 0)
    return;

  U32 len = as<U32>(std::min(n, as<int>(sizeof(local_buffer) - 1)));
  if (output_pos + len >= BUFFER_SIZE)
    flush_output();
  if (len >= BUFFER_SIZE) {
    std::fwrite(local_buffer, 1, len, stdout);
    return;
  }
  std::memcpy(output_buffer + output_pos, local_buffer, len);
  output_pos += len;
}

inline void write_char(char c) {
  if (output_pos >= BUFFER_SIZE)
    flush_output();
  output_buffer[output_pos++] = c;
}

inline void write_string(std::string_view s) {
  const char* data = s.data();
  U32 remaining    = as<U32>(s.size());
  while (remaining > 0) {
    if (output_pos >= BUFFER_SIZE)
      flush_output();
    U32 space = BUFFER_SIZE - output_pos;
    U32 chunk = (remaining < space) ? remaining : space;
    std::memcpy(output_buffer + output_pos, data, chunk);
    output_pos += chunk;
    data += chunk;
    remaining -= chunk;
  }
}

template <FastIntegral T>
inline void write(T x) { write_integer(x); }

template <FastFloating T>
inline void write(T x) { write_floating(x); }

inline void write(char x) { write_char(x); }
inline void write(const std::string& x) { write_string(x); }
inline void write(const char* x) { write_string(x); }

} // namespace fast_io

/* ------------------------------- EXTENSIONS ------------------------------- */

namespace fast_io {

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

// Single-arg fallbacks: error on types lacking a concrete overload.
template <class T>
  requires(!FastIntegral<T> && !FastFloating<T>)
void read(T&) = delete;

template <class T>
  requires(!FastIntegral<T> && !FastFloating<T>)
void write(const T&) = delete;

struct IOFlusher {
  ~IOFlusher() { flush_output(); }
};

inline IOFlusher io_flusher;

} // namespace fast_io

#define CP_IO_IMPL_READ(...) fast_io::read(__VA_ARGS__)
#define CP_IO_IMPL_WRITELN(...) fast_io::writeln(__VA_ARGS__)
#define CP_IO_IMPL_FLUSH() fast_io::flush_output()

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
/* Data Structures & Algorithms for the Problem */

template <typename Monoid>
struct DualSegmentTree {
  using Value = typename Monoid::value_type;

  I32 n    = 0;
  I32 log  = 0;
  I32 size = 1;
  Vec<Value> lazy;
  Vec<char> has_lazy;

  DualSegmentTree() = default;
  explicit DualSegmentTree(I32 size_) { build(size_); }

  template <typename F>
  DualSegmentTree(I32 size_, F init) {
    build(size_, init);
  }

  explicit DualSegmentTree(const Vec<Value>& values) { build(values); }

  void build(I32 size_) {
    build(size_, [](I32) -> Value { return Monoid::identity(); });
  }

  void build(const Vec<Value>& values) {
    build(isz(values), [&](I32 i) -> Value { return values[i]; });
  }

  template <typename F>
  void build(I32 size_, F init) {
    n    = std::max<I32>(size_, 0);
    log  = 0;
    size = 1;
    while (size < std::max<I32>(n, 1)) {
      size <<= 1;
      ++log;
    }
    lazy.assign(size << 1, Monoid::identity());
    has_lazy.assign(size, false);
    FOR(i, n)
    lazy[size + i] = init(i);
  }

  [[nodiscard]] auto get(I32 idx) -> Value {
    my_assert(0 <= idx && idx < n);
    idx += size;
    push_path(idx);
    return lazy[idx];
  }

  [[nodiscard]] auto query(I32 idx) -> Value { return get(idx); }

  [[nodiscard]] auto get_all() -> Vec<Value> {
    FOR(i, 1, size)
    push(i);
    return {lazy.begin() + size, lazy.begin() + size + n};
  }

  void set(I32 idx, const Value& value) {
    my_assert(0 <= idx && idx < n);
    idx += size;
    push_path(idx);
    lazy[idx] = value;
  }

  void apply(I32 l, I32 r, const Value& action) {
    my_assert(0 <= l && l <= r && r <= n);
    if (l == r)
      return;
    l += size;
    r += size;
    push_path(l);
    push_path(r - 1);
    while (l < r) {
      if (l & 1)
        all_apply(l++, action);
      if (r & 1)
        all_apply(--r, action);
      l >>= 1;
      r >>= 1;
    }
  }

private:
  void all_apply(I32 idx, const Value& action) {
    if (idx < size) {
      if (has_lazy[idx])
        lazy[idx] = Monoid::combine(lazy[idx], action);
      else {
        lazy[idx]     = action;
        has_lazy[idx] = true;
      }
      return;
    }
    lazy[idx] = Monoid::combine(lazy[idx], action);
  }

  void push(I32 idx) {
    if (!has_lazy[idx])
      return;
    all_apply(idx << 1, lazy[idx]);
    all_apply(idx << 1 | 1, lazy[idx]);
    lazy[idx]     = Monoid::identity();
    has_lazy[idx] = false;
  }

  void push_path(I32 idx) {
    for (I32 h = log; h >= 1; --h) {
      push(idx >> h);
    }
  }
};

//===----------------------------------------------------------------------===//
/* Temporal Tags */

struct H32 {
  I32 sum{};
  I32 pref{};
  I32 suf{};
  I32 best{};
};

struct H64 {
  I64 sum{};
  I64 pref{};
  I64 suf{};
  I64 best{};
  I32 len{};
};

auto cat(const H32& a, I32 na, const H32& b, I32 nb) -> H32 {
  if (na == 0)
    return b;
  if (nb == 0)
    return a;
  return {
      a.sum + b.sum,
      std::max(a.pref, a.sum + b.pref),
      std::max(b.suf, a.suf + b.sum),
      std::max({a.best, b.best, a.suf + b.pref}),
  };
}

auto cat(const H64& a, const H64& b) -> H64 {
  if (a.len == 0)
    return b;
  if (b.len == 0)
    return a;
  return {
      a.sum + b.sum,
      std::max(a.pref, a.sum + b.pref),
      std::max(b.suf, a.suf + b.sum),
      std::max({a.best, b.best, a.suf + b.pref}),
      a.len + b.len,
  };
}

auto one(I64 x) -> H64 { return {x, x, x, x, 1}; }

auto scale(const H32& h, I32 len, I64 k) -> H64 {
  return {
      I64(h.sum) * k,
      I64(h.pref) * k,
      I64(h.suf) * k,
      I64(h.best) * k,
      len,
  };
}

struct Hom {
  H32 h[2]{};
  I32 len{};
  I8 to[2]{1, -1};
};

struct Eval {
  I64 cur{};
  H64 h{};
};

auto eval(const Hom& f, I64 x) -> Eval {
  if (x == 0) {
    H64 h;
    h.len = f.len;
    return {0, h};
  }

  I32 s = x > 0 ? 0 : 1;
  I64 k = x > 0 ? x : -x;
  return {I64(f.to[s]) * k, scale(f.h[s], f.len, k)};
}

auto comp(const Hom& a, const Hom& b) -> Hom {
  Hom c;
  c.len = a.len + b.len;

  FOR(s, 2) {
    I32 x = a.to[s];
    H32 h;
    I8 to = 0;

    if (x > 0) {
      h  = b.h[0];
      to = b.to[0];
    } else if (x < 0) {
      h  = b.h[1];
      to = b.to[1];
    }

    c.h[s]  = cat(a.h[s], a.len, h, b.len);
    c.to[s] = to;
  }

  return c;
}

constexpr I64 NO_SET = Limits<I64>::max();

struct Tag {
  Hom pre{};
  H64 post{};
  I64 cur = NO_SET;

  auto fixed() const -> bool { return cur != NO_SET; }
};

auto eval(const Tag& f, I64 x) -> Eval {
  Eval e = eval(f.pre, x);
  if (!f.fixed())
    return e;
  e.h   = cat(e.h, f.post);
  e.cur = f.cur;
  return e;
}

auto comp(const Tag& a, const Tag& b) -> Tag {
  Tag c;

  if (a.fixed()) {
    Eval e = eval(b, a.cur);
    c.pre  = a.pre;
    c.post = cat(a.post, e.h);
    c.cur  = e.cur;
  } else if (b.fixed()) {
    c.pre  = comp(a.pre, b.pre);
    c.post = b.post;
    c.cur  = b.cur;
  } else {
    c.pre = comp(a.pre, b.pre);
  }

  return c;
}

auto init_tag(I64 x) -> Tag {
  Tag f;
  f.post = one(x);
  f.cur  = x;
  return f;
}

auto set_tag(I64 x) -> Tag {
  Tag f;
  f.cur = x;
  return f;
}

auto neg_tag() -> Tag {
  Tag f;
  f.pre.to[0] = -1;
  f.pre.to[1] = 1;
  return f;
}

auto clamp_tag() -> Tag {
  Tag f;
  f.pre.to[1] = 0;
  return f;
}

auto append_tag() -> Tag {
  Tag f;
  f.pre.h[0] = {1, 1, 1, 1};
  f.pre.h[1] = {-1, -1, -1, -1};
  f.pre.len  = 1;
  return f;
}

struct TagMonoid {
  using value_type = Tag;

  static auto identity() -> Tag { return {}; }
  static auto combine(const Tag& a, const Tag& b) -> Tag { return comp(a, b); }
};

struct Seg : DualSegmentTree<TagMonoid> {
  using DualSegmentTree<TagMonoid>::DualSegmentTree;

  void apply_all(const Tag& f) {
    if (size == 1) {
      lazy[1] = TagMonoid::combine(lazy[1], f);
    } else if (has_lazy[1]) {
      lazy[1] = TagMonoid::combine(lazy[1], f);
    } else {
      lazy[1] = f;
      has_lazy[1] = true;
    }
  }
};

//===----------------------------------------------------------------------===//
/* Main Solver Function */

using namespace std;

void solve() {
  INT(n, q);
  Vec<I64> a(n);
  FOR(i, n)
  IN(a[i]);

  I32 m = 1;
  while (m < n)
    m <<= 1;

  Seg seg(m, [&](I32 i) -> Tag {
    return i < n ? init_tag(a[i]) : Tag{};
  });

  const Tag neg    = neg_tag();
  const Tag clamp  = clamp_tag();
  const Tag append = append_tag();
  U64 last = 0;

  auto dec = [&](U64 x) -> I32 {
    return I32((x ^ last) % U64(n));
  };

  FOR(q) {
    INT(op);

    if (op == 1) {
      ULL(u, v);
      LL(x);
      I32 l = dec(u);
      I32 r = dec(v);
      if (l > r)
        std::swap(l, r);
      seg.apply(l, r + 1, set_tag(x));
    } else if (op == 2 || op == 3) {
      ULL(u, v);
      I32 l = dec(u);
      I32 r = dec(v);
      if (l > r)
        std::swap(l, r);
      seg.apply(l, r + 1, op == 2 ? neg : clamp);
    } else {
      ULL(u);
      Tag f   = seg.get(dec(u));
      I64 ans = f.post.best;
      OUT(ans);
      last = U64(ans);
    }

    seg.apply_all(append);
  }
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
