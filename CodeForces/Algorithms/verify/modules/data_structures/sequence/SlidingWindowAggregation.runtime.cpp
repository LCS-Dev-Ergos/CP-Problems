#include "modules/data_structures/sequence/SlidingWindowAggregation.hpp"

#include <cassert>

struct AddMonoid {
  using value_type = I64;

  static constexpr value_type unit() { return 0; }
  static constexpr value_type op(value_type a, value_type b) { return a + b; }
};

int main() {
  SlidingWindowAggregation<AddMonoid> swag;
  assert(swag.empty());

  swag.push(3);
  swag.push(1);
  swag.push(4);
  assert(swag.size() == 3);
  assert(swag.front() == 3);
  assert(swag.prod() == 8);

  swag.pop();
  assert(swag.front() == 1);
  assert(swag.prod() == 5);

  swag.push(2);
  assert(swag.prod() == 7);

  swag.pop();
  swag.pop();
  assert(swag.front() == 2);
  assert(swag.prod() == 2);

  swag.clear();
  assert(swag.empty());
  return 0;
}
