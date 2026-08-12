#include "modules/knapsack/Knapsack01.hpp"

int main() {
  const VecI32 weight = {3, 4, 5};
  const VecI64 value = {30, 50, 60};
  return knapsack_01(weight, value, 8) == 90 ? 0 : 1;
}
