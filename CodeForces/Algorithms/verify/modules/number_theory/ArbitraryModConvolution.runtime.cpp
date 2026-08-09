#include "modules/number_theory/ArbitraryModConvolution.hpp"

#include <cassert>
#include <random>

namespace {

constexpr I64 P0 = CONVOLUTION_PRIME_0;
constexpr I64 P1 = CONVOLUTION_PRIME_1;
constexpr I64 P2 = CONVOLUTION_PRIME_2;

/// Horner evaluation, used to check large products without a quadratic pass.
auto evaluate(const VecI64& poly, I64 point, I64 mod) -> I64 {
  I64 acc = 0;
  FOR_R(i, isz(poly)) acc = as<I64>((as<I128>(acc) * point + poly[i]) % mod);
  return acc;
}

auto random_below(std::mt19937_64& rng, I128 bound) -> I128 {
  const I128 raw = (I128(rng() >> 1) << 63) | I128(rng() >> 1);
  return raw % bound;
}

}  // namespace

int main() {
  std::mt19937_64 rng(20260814);

  // garner3() is checked end to end: pick the value first, reduce it against
  // each prime, and require the reconstruction to agree modulo the target.
  {
    const I128 product = I128(P0) * P1 * P2;
    for (const I64 mod : {2LL, 7LL, 1'000'000'000LL, 1'000'000'007LL, 2'147'483'647LL}) {
      FOR(iter, 2000) {
        const I128 value = random_below(rng, product);
        const I64 r0 = as<I64>(value % P0);
        const I64 r1 = as<I64>(value % P1);
        const I64 r2 = as<I64>(value % P2);
        const I64 want = as<I64>(value % mod);
        assert((garner3<P0, P1, P2>(r0, r1, r2, mod)) == want);
      }
    }
    // The extreme residues, where the intermediate digits are largest.
    assert((garner3<P0, P1, P2>(0, 0, 0, 1'000'000'007LL)) == 0);
    const I128 last = product - 1;
    assert((garner3<P0, P1, P2>(as<I64>(last % P0), as<I64>(last % P1), as<I64>(last % P2),
                                1'000'000'007LL)) == as<I64>(last % 1'000'000'007LL));
  }

  // Randomized products against the schoolbook reference, over moduli that are
  // prime, composite, NTT-friendly and degenerate.
  {
    const VecI64 moduli = {1,          2,          3,           97,
                           998'244'353, 1'000'000'007, 1'000'000'000, 1LL << 30,
                           2'147'483'647};
    for (const I64 mod : moduli) {
      FOR(iter, 12) {
        // Sizes straddle the threshold where the transform takes over.
        const I32 n = 1 + I32(rng() % 130);
        const I32 m = 1 + I32(rng() % 130);
        VecI64 a(n);
        VecI64 b(m);
        FOR(i, n) a[i] = I64(rng() % U64(mod));
        FOR(i, m) b[i] = I64(rng() % U64(mod));
        assert(convolution_mod(a, b, mod) == convolution_naive(a, b, mod));
      }
    }
  }

  // The two code paths have to agree across the size threshold itself.
  {
    const I64 mod = 1'000'000'007LL;
    FOR(n, 55, 70) {
      VecI64 a(n);
      VecI64 b(n);
      FOR(i, n) a[i] = I64(rng() % U64(mod));
      FOR(i, n) b[i] = I64(rng() % U64(mod));
      assert(convolution_mod(a, b, mod) == convolution_naive(a, b, mod));
    }
  }

  // Unreduced and negative entries are normalized internally, on both paths.
  {
    const I64 mod = 1'000'000'007LL;
    FOR(n, 1, 130) {
      VecI64 raw(n);
      VecI64 reduced(n);
      FOR(i, n) {
        raw[i] = I64(rng() % 4'000'000'000ULL) - 2'000'000'000LL;
        reduced[i] = ((raw[i] % mod) + mod) % mod;
      }
      assert(convolution_mod(raw, raw, mod) == convolution_mod(reduced, reduced, mod));
    }
  }

  // The NTT-friendly modulus takes a shortcut that must not change answers.
  {
    VecI64 a(200);
    VecI64 b(200);
    FOR(i, 200) a[i] = I64(rng() % U64(MOD2));
    FOR(i, 200) b[i] = I64(rng() % U64(MOD2));
    assert(convolution_mod(a, b, MOD2) == NTT::multiply(a, b));
    assert(convolution_mod(a, b, MOD2) == convolution_naive(a, b, MOD2));
  }

  // A modulus of one collapses everything to zero.
  {
    const VecI64 a(100, 5);
    const VecI64 got = convolution_mod(a, a, 1);
    assert(isz(got) == 199);
    for (const I64 x : got) assert(x == 0);
  }

  // Empty inputs.
  {
    assert(convolution_mod({}, {1, 2}, 97).empty());
    assert(convolution_mod({1, 2}, {}, 97).empty());
    assert(convolution_naive({}, {}, 97).empty());
  }

  // Scale case: cross-checked by evaluating the factors at random points, which
  // is independent of how the coefficients were reconstructed.
  {
    constexpr I32 N = 70000;
    const I64 mod = 1'000'000'007LL;
    VecI64 a(N);
    VecI64 b(N);
    FOR(i, N) a[i] = I64(rng() % U64(mod));
    FOR(i, N) b[i] = I64(rng() % U64(mod));
    const VecI64 got = convolution_mod(a, b, mod);
    assert(isz(got) == 2 * N - 1);
    FOR(trial, 8) {
      const I64 point = I64(rng() % U64(mod));
      const I64 want =
          as<I64>(as<I128>(evaluate(a, point, mod)) * evaluate(b, point, mod) % mod);
      assert(evaluate(got, point, mod) == want);
    }
  }

  // The widest supported modulus, where the three primes are needed in full.
  {
    constexpr I32 N = 20000;
    const I64 mod = 2'147'483'647LL;
    VecI64 a(N);
    VecI64 b(N);
    FOR(i, N) a[i] = mod - 1 - I64(rng() % 3);
    FOR(i, N) b[i] = mod - 1 - I64(rng() % 3);
    const VecI64 got = convolution_mod(a, b, mod);
    FOR(trial, 4) {
      const I64 point = I64(rng() % U64(mod));
      const I64 want =
          as<I64>(as<I128>(evaluate(a, point, mod)) * evaluate(b, point, mod) % mod);
      assert(evaluate(got, point, mod) == want);
    }
  }

  return 0;
}
