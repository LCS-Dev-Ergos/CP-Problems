#include "modules/bitwise/SubsetTransform.hpp"

int main() {
  Vec<I64> f{1, 1, 1, 1};
  subset_zeta(f);
  return f[3] == 4 ? 0 : 1;
}
