#include "modules/number_theory/NTT.hpp"

int main() {
  // The default alias keeps the historical 998244353 interface.
  static_assert(NTT::MOD == MOD2);
  static_assert(NTT::ROOT == 3);
  static_assert(NTT::MAX_SIZE == (1 << 23));

  // Any NTT-friendly prime instantiates, with its bound read off the modulus.
  using Alternate = NTTOver<469'762'049LL, 3>;
  static_assert(Alternate::MAX_SIZE == (1 << 26));

  const VecI64 a = {1, 2, 3};
  const VecI64 b = {4, 5};
  return (NTT::multiply(a, b) == VecI64{4, 13, 22, 15} &&
          Alternate::multiply(a, b) == VecI64{4, 13, 22, 15})
             ? 0
             : 1;
}
