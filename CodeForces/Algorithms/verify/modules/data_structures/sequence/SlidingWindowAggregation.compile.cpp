#include "modules/data_structures/sequence/SlidingWindowAggregation.hpp"

struct MinMonoid {
  using value_type = I32;

  static constexpr value_type unit() { return 1'000'000'000; }
  static constexpr value_type op(value_type a, value_type b) { return std::min(a, b); }
};

int main() {
  SlidingWindowAggregation<MinMonoid> swag;
  swag.push(7);
  swag.push(3);
  return swag.prod() == 3 ? 0 : 1;
}
