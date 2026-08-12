//===----------------------------------------------------------------------===//
/**
 * @file: problem_F_sub.cpp
 * @generated: 2026-08-12 16:52:36
 * @source: problem_F.cpp
 * @author: C.L.
 *
 * @brief: Codeforces Round 1115 (Div. 2) - Problem F
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

#ifndef CP_FLOAT_PRECISION
  #define CP_FLOAT_PRECISION 10
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
/* Runtime Contracts */

namespace cp::detail {

[[noreturn]] [[gnu::cold]] inline void contract_violation() noexcept { __builtin_trap(); }

[[gnu::always_inline]] constexpr void expect(bool condition) noexcept {
  if (!condition) [[unlikely]]
    contract_violation();
}

} // namespace cp::detail

#define CP_EXPECT(condition, ...) cp::detail::expect(bool(condition))

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
[[gnu::always_inline]] constexpr I64 sz64(const R& x) noexcept(noexcept(std::ssize(x))) {
  return std::ssize(x);
}

template <class R>
[[gnu::always_inline]] constexpr I32 sz32(const R& x) noexcept(noexcept(std::ssize(x))) {
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
#define all(x) (x).begin(), (x).end()
#define rall(x) (x).rbegin(), (x).rend()
#define sz(x) cp::sz64(x)
#define isz(x) cp::sz32(x)
#define len(x) sz(x)
#define pb push_back
#define eb emplace_back
#define elif else if

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
  std::cout << std::fixed << std::setprecision(CP_FLOAT_PRECISION);
}

struct IOSetup {
  IOSetup() { setup(); }
};

inline IOSetup io_setup;

template <class T>
void read(T& x) {
  CP_EXPECT(bool(std::cin >> x), "I/O: failed to read a value.");
}

template <class T>
void write(const T& x) {
  CP_EXPECT(bool(std::cout << x), "I/O: failed to write a value.");
}

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
/* Data Structures & Algorithms for the Problem */

template <typename Weight = I64>
struct Graph {
  struct Edge {
    I32 from;
    I32 to;
    Weight weight;
    I32 id;

    Edge(I32 f, I32 t, Weight w = 1, I32 i = -1) : from(f), to(t), weight(w), id(i) {}

    bool operator<(const Edge& other) const { return weight < other.weight; }
  };

  I32 n;
  I32 m = 0;
  Vec2D<Edge> adj;
  Vec<Edge> edges;
  bool directed;

  Graph(I32 n_, bool directed_ = false) : n(n_), adj(n_), directed(directed_) {}

  void add_edge(I32 from, I32 to, Weight weight = 1) {
    adj[from].emplace_back(from, to, weight, m);
    edges.emplace_back(from, to, weight, m);
    if (!directed) {
      adj[to].emplace_back(to, from, weight, m);
      edges.emplace_back(to, from, weight, m);
    }
    ++m;
  }
};

template <typename Weight = I64>
struct TreeCore {
  I32 n                  = 0;
  const Graph<Weight>* g = nullptr;
  VecI32 parent;
  VecI32 depth;
  Vec<Weight> depth_weight;
  VecI32 heavy;
  VecI32 head;
  VecI32 pos;
  VecI32 tout;
  VecI32 subtree_size;
  VecI32 order;
  VecI32 component;
  VecI32 root;
  VecI32 edge_to_parent;
  VecI32 edge_to_vertex;

  TreeCore() = default;
  explicit TreeCore(const Graph<Weight>& graph, I32 first_root = 0) { build(graph, first_root); }

  void build(const Graph<Weight>& graph, I32 first_root = 0) {
    g = &graph;
    n = graph.n;
    parent.assign(n, -2);
    depth.assign(n, 0);
    depth_weight.assign(n, Weight{});
    heavy.assign(n, -1);
    head.assign(n, -1);
    pos.assign(n, -1);
    tout.assign(n, -1);
    subtree_size.assign(n, 0);
    order.assign(n, -1);
    component.assign(n, -1);
    root.assign(n, -1);
    edge_to_parent.assign(n, -1);
    edge_to_vertex.assign(std::max<I32>(graph.m, 0), -1);

    VecI32 roots;
    if (0 <= first_root && first_root < n)
      roots.push_back(first_root);
    FOR(v, n) {
      if (v != first_root)
        roots.push_back(v);
    }

    VecI32 visit_order;
    visit_order.reserve(n);
    for (I32 start : roots) {
      if (parent[start] != -2)
        continue;
      parent[start]    = -1;
      component[start] = start;
      root[start]      = start;

      VecI32 st{start};
      while (!st.empty()) {
        const I32 v = st.back();
        st.pop_back();
        visit_order.push_back(v);
        for (const auto& e : graph.adj[v]) {
          if (e.to == parent[v] || parent[e.to] != -2)
            continue;
          parent[e.to]         = v;
          depth[e.to]          = depth[v] + 1;
          depth_weight[e.to]   = depth_weight[v] + e.weight;
          component[e.to]      = start;
          root[e.to]           = start;
          edge_to_parent[e.to] = e.id;
          if (0 <= e.id && e.id < isz(edge_to_vertex))
            edge_to_vertex[e.id] = e.to;
          st.push_back(e.to);
        }
      }
    }

    for (I32 i = isz(visit_order) - 1; i >= 0; --i) {
      const I32 v     = visit_order[i];
      subtree_size[v] = 1;
      I32 best_size   = 0;
      for (const auto& e : graph.adj[v]) {
        if (parent[e.to] != v)
          continue;
        subtree_size[v] += subtree_size[e.to];
        if (subtree_size[e.to] > best_size) {
          best_size = subtree_size[e.to];
          heavy[v]  = e.to;
        }
      }
    }

    I32 timer = 0;
    for (I32 start : roots) {
      if (start < 0 || start >= n || head[start] != -1)
        continue;
      Vec<PairI32> chains{{start, start}};
      while (!chains.empty()) {
        auto [v, h] = chains.back();
        chains.pop_back();
        for (I32 cur = v; cur != -1; cur = heavy[cur]) {
          head[cur]      = h;
          pos[cur]       = timer;
          order[timer++] = cur;
          for (const auto& e : graph.adj[cur]) {
            if (parent[e.to] == cur && e.to != heavy[cur]) {
              chains.push_back({e.to, e.to});
            }
          }
        }
      }
    }
    FOR(v, n)
    tout[v] = pos[v] + subtree_size[v];
  }

  [[nodiscard]] bool same_tree(I32 u, I32 v) const { return component[u] == component[v]; }

  [[nodiscard]] PairI32 subtree_range(I32 v) const { return {pos[v], tout[v]}; }

  [[nodiscard]] bool in_subtree(I32 u, I32 v) const {
    return same_tree(u, v) && pos[v] <= pos[u] && pos[u] < tout[v];
  }

  [[nodiscard]] I32 lca(I32 u, I32 v) const {
    if (!same_tree(u, v))
      return -1;
    while (head[u] != head[v]) {
      if (depth[head[u]] > depth[head[v]])
        u = parent[head[u]];
      else
        v = parent[head[v]];
    }
    return depth[u] < depth[v] ? u : v;
  }

  [[nodiscard]] I32 dist(I32 u, I32 v) const {
    const I32 w = lca(u, v);
    if (w == -1)
      return -1;
    return depth[u] + depth[v] - 2 * depth[w];
  }

  [[nodiscard]] Weight dist_weighted(I32 u, I32 v) const {
    const I32 w = lca(u, v);
    if (w == -1)
      return Weight{};
    return depth_weight[u] + depth_weight[v] - Weight(2) * depth_weight[w];
  }

  [[nodiscard]] I32 kth_ancestor(I32 v, I32 k) const {
    if (k < 0 || k > depth[v])
      return -1;
    I32 target = depth[v] - k;
    while (depth[head[v]] > target)
      v = parent[head[v]];
    return order[pos[v] - (depth[v] - target)];
  }

  [[nodiscard]] I32 jump(I32 u, I32 v, I32 k) const {
    const I32 w = lca(u, v);
    if (w == -1)
      return -1;
    const I32 up   = depth[u] - depth[w];
    const I32 down = depth[v] - depth[w];
    if (k < 0 || k > up + down)
      return -1;
    if (k <= up)
      return kth_ancestor(u, k);
    return kth_ancestor(v, up + down - k);
  }

  [[nodiscard]] I32 edge_child(I32 edge_id) const {
    if (edge_id < 0 || edge_id >= isz(edge_to_vertex))
      return -1;
    return edge_to_vertex[edge_id];
  }

  [[nodiscard]] Vec<PairI32> path_segments(I32 u, I32 v, bool edge_mode = false) const {
    Vec<PairI32> up;
    Vec<PairI32> down;
    if (!same_tree(u, v))
      return up;

    auto add_up = [&](I32 l, I32 r) {
      if (l >= r)
        up.push_back({l, r});
    };
    auto add_down = [&](I32 l, I32 r) {
      if (l <= r)
        down.push_back({l, r});
    };

    while (head[u] != head[v]) {
      if (depth[head[u]] > depth[head[v]]) {
        add_up(pos[u], pos[head[u]]);
        u = parent[head[u]];
      } else {
        add_down(pos[head[v]], pos[v]);
        v = parent[head[v]];
      }
    }

    if (depth[u] > depth[v]) {
      add_up(pos[u], pos[v] + (edge_mode ? 1 : 0));
    } else {
      add_down(pos[u] + (edge_mode ? 1 : 0), pos[v]);
    }

    std::ranges::reverse(down);
    up.insert(up.end(), all(down));
    return up;
  }

  [[nodiscard]] VecI32 restore_path(I32 u, I32 v) const {
    VecI32 path;
    for (auto [l, r] : path_segments(u, v, false)) {
      if (l <= r) {
        FOR(i, l, r + 1)
        path.push_back(order[i]);
      } else {
        for (I32 i = l; i >= r; --i)
          path.push_back(order[i]);
      }
    }
    return path;
  }
};

//===----------------------------------------------------------------------===//
/* Main Solver Function */

using namespace std;

void solve() {
  INT(n);
  VecI32 c(n), k(n);
  IN(c, k);

  Vec2D<I32> at(n);
  FOR(v, n) {
    --c[v];
    at[c[v]].push_back(v);
  }

  Graph<> g(n);
  FOR(n - 1) {
    INT(u, v);
    g.add_edge(--u, --v);
  }

  TreeCore<> tr(g, 0);
  VecI64 ans(n, -1);

  FOR(col, n) {
    if (at[col].empty())
      continue;

    auto nodes  = at[col];
    auto by_pos = [&](I32 u, I32 v) { return tr.pos[u] < tr.pos[v]; };
    sort(all(nodes), by_pos);

    I32 m = isz(nodes);
    FOR(i, 1, m)
    nodes.push_back(tr.lca(nodes[i - 1], nodes[i]));
    sort(all(nodes), by_pos);
    nodes.erase(unique(all(nodes)), nodes.end());

    I32 q = isz(nodes);
    VecI32 par(q, -1), dist(q), st{0};
    auto anc = [&](I32 u, I32 v) {
      return tr.pos[u] <= tr.pos[v] && tr.pos[v] < tr.pos[u] + tr.subtree_size[u];
    };

    FOR(i, 1, q) {
      while (!anc(nodes[st.back()], nodes[i]))
        st.pop_back();
      par[i]  = st.back();
      dist[i] = tr.depth[nodes[i]] - tr.depth[nodes[par[i]]];
      st.push_back(i);
    }

    VecI32 sub(q);
    FOR(i, q)
    sub[i] = c[nodes[i]] == col;

    Vec<PairI32> gain;
    gain.reserve(q - 1);
    I64 cur = 0;
    for (I32 i = q - 1; i > 0; --i) {
      I32 b = min(sub[i], m - sub[i]);
      cur += I64(b) * dist[i];
      gain.push_back({b, dist[i]});
      sub[par[i]] += sub[i];
    }

    sort(gain.rbegin(), gain.rend());
    I32 need = k[col] - 1;
    for (auto [b, cnt] : gain) {
      I32 take = min(need, cnt);
      cur -= I64(b) * take;
      need -= take;
      if (!need)
        break;
    }
    ans[col] = cur;
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
