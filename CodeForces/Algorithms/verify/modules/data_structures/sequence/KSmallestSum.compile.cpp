#include "modules/data_structures/sequence/KSmallestSum.hpp"

int main() {
  KSmallestSum<I32, I64> small(2);
  for (const I32 x : {5, 1, 4, 2}) small.insert(x);
  KLargestSum<I32, I64> large(2);
  for (const I32 x : {5, 1, 4, 2}) large.insert(x);
  return small.sum() == 3 && large.sum() == 9 ? 0 : 1;
}
