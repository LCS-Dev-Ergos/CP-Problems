#include "modules/number_theory/Binomial.hpp"
#include "templates/modules/Mod_Int.hpp"

using Mint = ModInt<998244353>;

int main() {
  Binomial<Mint> bn;
  return bn.C(5, 2) == Mint(10) ? 0 : 1;
}
