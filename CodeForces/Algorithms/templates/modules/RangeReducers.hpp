#pragma once
#include "templates/core/Contracts.hpp"
#include "templates/core/CoreConcepts.hpp"
#include "templates/core/ScalarTypes.hpp"
#include "templates/core/TypeTraits.hpp"

//===----------------------------------------------------------------------===//
/* Range Reducers */

// clang-format off
namespace cp {

// Narrow integers accumulate in I64/U64; any other element type keeps its own.
template <class R>
[[gnu::always_inline]] constexpr auto sum_range(const R& r) {
  using V   = std::ranges::range_value_t<RemoveCvrefT<R>>;
  using Acc = Conditional<std::same_as<V, bool>, I64,
              Conditional<Int<V> && sizeof(V) < sizeof(I64),
              Conditional<Signed<V>, I64, U64>, V>>;
  return std::accumulate(std::ranges::begin(r), std::ranges::end(r), Acc{});
}

} // namespace cp

#define SUM(x) cp::sum_range(x)
#define MIN(x)                                       \
  ([&]() -> decltype(auto) {                         \
    auto&& _cp_min_range = (x);                      \
    if (std::ranges::empty(_cp_min_range)) {         \
      CP_EXPECT(false, "MIN(): empty range.");       \
    }                                                \
    return *std::ranges::min_element(_cp_min_range); \
  }())
#define MAX(x)                                       \
  ([&]() -> decltype(auto) {                         \
    auto&& _cp_max_range = (x);                      \
    if (std::ranges::empty(_cp_max_range)) {         \
      CP_EXPECT(false, "MAX(): empty range.");       \
    }                                                \
    return *std::ranges::max_element(_cp_max_range); \
  }())
// clang-format on
