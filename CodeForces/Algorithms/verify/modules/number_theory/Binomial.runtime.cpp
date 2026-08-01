#include "modules/number_theory/Binomial.hpp"
#include "templates/modules/Mod_Int.hpp"

#include <cassert>

using Mint = ModInt<998244353>;

int main() {
  Binomial<Mint> bn;
  assert(bn.C(5, 2) == Mint(10));
  assert(bn.C(0, 0) == Mint(1));
  assert(bn.C(5, 6) == Mint(0));
  assert(bn.C(5, -1) == Mint(0));
  assert(bn.C(-1, 0) == Mint(0));
  assert(bn.P(5, 2) == Mint(20));
  assert(bn.P(5, 0) == Mint(1));
  assert(bn.H(3, 2) == Mint(6));
  assert(bn.H(3, 0) == Mint(1));
  assert(bn.catalan(0) == Mint(1));
  assert(bn.catalan(5) == Mint(42));
  assert(bn.inv(3) * Mint(3) == Mint(1));

  // Pascal's triangle, which also exercises the geometric table growth.
  constexpr I32 N = 60;
  Vec2D<Mint> pas(N, Vec<Mint>(N, Mint(0)));
  FOR(i, N) {
    pas[i][0] = Mint(1);
    FOR(j, 1, i + 1) pas[i][j] = pas[i - 1][j - 1] + pas[i - 1][j];
  }
  FOR(i, N) FOR(j, i + 1) assert(bn.C(i, j) == pas[i][j]);

  // C_huge must agree with the tabulated form wherever both apply.
  FOR(n, 40) FOR(r, n + 1) assert(bn.C_huge(n, r) == bn.C(n, r));

  // Hockey-stick identity as an independent structural check.
  FOR(n, 1, 20) {
    Mint acc(0);
    FOR(i, n + 1) acc += bn.C(i, 3);
    assert(acc == bn.C(n + 1, 4));
  }
  return 0;
}
