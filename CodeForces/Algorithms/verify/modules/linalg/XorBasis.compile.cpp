#include "modules/linalg/XorBasis.hpp"

int main() {
  XorBasis<U64> b;
  b.insert(5);
  return b.size() == 1 && b.contains(5) ? 0 : 1;
}
