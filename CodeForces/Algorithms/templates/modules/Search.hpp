#pragma once
#include "templates/core/Contracts.hpp"
#include "templates/core/CoreConcepts.hpp"

//===----------------------------------------------------------------------===//
/* Search Utilities */

template <typename F>
  requires cp::Predicate<F&, I64>
I64 binary_search(F&& predicate, I64 left, I64 right) {
  CP_EXPECT(left < right, "binary_search(): expected left < right.");
  while (left + 1 < right) {
    const I64 mid = std::midpoint(left, right);
    (predicate(mid) ? left : right) = mid;
  }
  return left;
}

template <typename F>
  requires cp::Predicate<F&, F64>
F64 binary_search_real(F&& predicate, F64 left, F64 right, I32 iterations = 100) {
  CP_EXPECT(std::isfinite(left) && std::isfinite(right) && left <= right,
            "binary_search_real(): expected a finite ordered interval.");
  CP_EXPECT(iterations >= 0, "binary_search_real(): iterations must be non-negative.");
  for (I32 i = 0; i < iterations; ++i) {
    const F64 mid = std::midpoint(left, right);
    (predicate(mid) ? left : right) = mid;
  }
  return std::midpoint(left, right);
}
