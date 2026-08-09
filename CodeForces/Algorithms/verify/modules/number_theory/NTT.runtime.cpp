#include "modules/number_theory/NTT.hpp"

#include <cassert>
#include <random>

namespace {

/// Reference product, quadratic in the input sizes.
auto naive_multiply(const VecI64& a, const VecI64& b, I64 mod) -> VecI64 {
  if (a.empty() || b.empty()) return {};
  VecI64 result(isz(a) + isz(b) - 1, 0);
  FOR(i, isz(a)) FOR(j, isz(b)) result[i + j] = (result[i + j] + a[i] * b[j]) % mod;
  return result;
}

/// Horner evaluation, used to check large products without a quadratic pass.
auto evaluate(const VecI64& poly, I64 point, I64 mod) -> I64 {
  I64 acc = 0;
  FOR_R(i, isz(poly)) acc = (acc * point + poly[i]) % mod;
  return acc;
}

/// Confirms that ROOT really generates the whole multiplicative group.
template <typename Transform>
void check_root() {
  constexpr I64 MOD = Transform::MOD;
  constexpr I64 ROOT = Transform::ROOT;
  // The largest transform the modulus supports needs a root of that exact order.
  const I64 unity = mod_pow<I64>(ROOT, (MOD - 1) / Transform::MAX_SIZE, MOD);
  assert(mod_pow<I64>(unity, Transform::MAX_SIZE, MOD) == 1);
  assert(mod_pow<I64>(unity, Transform::MAX_SIZE / 2, MOD) != 1);
  // 2^MAX_LOG_N has to be the exact power of two dividing MOD - 1.
  assert((MOD - 1) % Transform::MAX_SIZE == 0);
  assert(((MOD - 1) / Transform::MAX_SIZE) % 2 == 1);
}

template <typename Transform>
void check_transform(std::mt19937_64& rng) {
  constexpr I64 MOD = Transform::MOD;
  check_root<Transform>();

  // Randomized products against the schoolbook reference.
  FOR(iter, 40) {
    const I32 n = 1 + I32(rng() % 60);
    const I32 m = 1 + I32(rng() % 60);
    VecI64 a(n);
    VecI64 b(m);
    FOR(i, n) a[i] = I64(rng() % U64(MOD));
    FOR(i, m) b[i] = I64(rng() % U64(MOD));
    assert(Transform::multiply(a, b) == naive_multiply(a, b, MOD));
  }

  // Unreduced and negative inputs have to be normalized internally.
  {
    const VecI64 raw = {MOD + 3, -1, 2 * MOD};
    const VecI64 reduced = {3, MOD - 1, 0};
    assert(Transform::multiply(raw, raw) == Transform::multiply(reduced, reduced));
  }

  // Forward then inverse transform is the identity.
  {
    VecI64 a(64);
    FOR(i, 64) a[i] = I64(rng() % U64(MOD));
    const VecI64 original = a;
    Transform::ntt(a, false);
    Transform::ntt(a, true);
    assert(a == original);
  }

  // Empty and singleton inputs.
  assert(Transform::multiply({}, {1, 2}).empty());
  assert(Transform::multiply({1, 2}, {}).empty());
  assert(Transform::multiply({7}, {6}) == (VecI64{42 % MOD}));
}

}  // namespace

int main() {
  // The transform bound is read off the modulus rather than hard-coded.
  static_assert(NTT::MOD == MOD2);
  static_assert(NTT::MAX_LOG_N == 23);
  static_assert(NTTOver<167'772'161LL, 3>::MAX_LOG_N == 25);
  static_assert(NTTOver<469'762'049LL, 3>::MAX_LOG_N == 26);
  static_assert(NTTOver<754'974'721LL, 11>::MAX_LOG_N == 24);
  static_assert(two_adic_valuation(1) == 0);
  static_assert(two_adic_valuation(0) == 0);
  static_assert(two_adic_valuation(96) == 5);

  std::mt19937_64 rng(20260813);
  check_transform<NTT>(rng);
  check_transform<NTTOver<167'772'161LL, 3>>(rng);
  check_transform<NTTOver<469'762'049LL, 3>>(rng);
  check_transform<NTTOver<754'974'721LL, 11>>(rng);

  // A product whose length is exactly a power of two, where the padding loop
  // stops without a doubling step.
  {
    VecI64 a(512);
    VecI64 b(513);
    FOR(i, 512) a[i] = I64(rng() % U64(MOD2));
    FOR(i, 513) b[i] = I64(rng() % U64(MOD2));
    const VecI64 got = NTT::multiply(a, b);
    assert(isz(got) == 1024);
    assert(got == naive_multiply(a, b, MOD2));
  }

  // Scale case: checked by evaluating both factors at random points instead of
  // running a quadratic reference.
  {
    constexpr I32 N = 1 << 16;
    VecI64 a(N);
    VecI64 b(N);
    FOR(i, N) a[i] = I64(rng() % U64(MOD2));
    FOR(i, N) b[i] = I64(rng() % U64(MOD2));
    const VecI64 got = NTT::multiply(a, b);
    assert(isz(got) == 2 * N - 1);
    FOR(trial, 8) {
      const I64 point = I64(rng() % U64(MOD2));
      const I64 want = as<I64>(as<I128>(evaluate(a, point, MOD2)) *
                               evaluate(b, point, MOD2) % MOD2);
      assert(evaluate(got, point, MOD2) == want);
    }
  }

  return 0;
}
