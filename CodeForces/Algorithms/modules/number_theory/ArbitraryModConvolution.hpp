#ifndef CP_MODULES_NUMBER_THEORY_ARBITRARY_MOD_CONVOLUTION_HPP
#define CP_MODULES_NUMBER_THEORY_ARBITRARY_MOD_CONVOLUTION_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"
#include "NTT.hpp"

/**
 * @brief Three-modulus Garner reconstruction specialised for convolution.
 *
 * @details Rebuilds the unique x in [0, P0*P1*P2) with x = r_i (mod P_i) and
 * returns it modulo @p mod. The mixed-radix digits are
 *
 *     x = d0 + d1*P0 + d2*P0*P1,
 *
 * which keeps every intermediate below P0*P1 < 2^63 and needs no wide integer
 * arithmetic. The two modular inverses depend only on the template parameters,
 * so they are folded at compile time; that is what makes this worth having
 * next to the general garner(), which re-derives them and re-checks pairwise
 * coprimality on every call.
 *
 * The caller is responsible for the reconstruction being meaningful: the true
 * value has to be smaller than P0*P1*P2.
 *
 * @tparam P0 P1 P2 Pairwise coprime moduli in strictly increasing order.
 */
template <I64 P0, I64 P1, I64 P2>
[[nodiscard]] constexpr auto garner3(I64 r0, I64 r1, I64 r2, I64 mod) -> I64 {
  static_assert(P0 < P1 && P1 < P2, "moduli must be given in increasing order");

  constexpr I64 INV_P0_MOD_P1 = mod_inv<I64>(P0 % P1, P1);
  constexpr I64 P0P1_MOD_P2 = as<I64>(as<I128>(P0) * P1 % P2);
  constexpr I64 INV_P0P1_MOD_P2 = mod_inv<I64>(P0P1_MOD_P2, P2);

  const I64 d0 = r0;
  const I64 d1 = (r1 - d0) % P1 * INV_P0_MOD_P1 % P1;
  const I64 digit1 = d1 < 0 ? d1 + P1 : d1;

  // d0 + digit1*P0 stays below P0*P1, so this still fits in 64 bits.
  const I64 partial = d0 + digit1 * P0;
  const I64 d2 = (r2 - partial % P2) % P2 * INV_P0P1_MOD_P2 % P2;
  const I64 digit2 = d2 < 0 ? d2 + P2 : d2;

  const I64 p0p1 = as<I64>(as<I128>(P0) * P1 % mod);
  return (partial % mod + as<I64>(as<I128>(digit2) * p0p1 % mod)) % mod;
}

/// @brief The three NTT-friendly primes backing arbitrary-modulus convolution.
///
/// Their 2-adic valuations are 25, 26 and 24, so 2^24 bounds the transform
/// length; the product exceeds 5.9e25, which bounds the representable
/// convolution coefficients.
inline constexpr I64 CONVOLUTION_PRIME_0 = 167'772'161LL;   // 5 * 2^25 + 1
inline constexpr I64 CONVOLUTION_PRIME_1 = 469'762'049LL;   // 7 * 2^26 + 1
inline constexpr I64 CONVOLUTION_PRIME_2 = 754'974'721LL;   // 45 * 2^24 + 1

using ConvolutionNTT0 = NTTOver<CONVOLUTION_PRIME_0, 3>;
using ConvolutionNTT1 = NTTOver<CONVOLUTION_PRIME_1, 3>;
using ConvolutionNTT2 = NTTOver<CONVOLUTION_PRIME_2, 11>;

/// @brief Schoolbook convolution modulo @p mod, for inputs too small to transform.
[[nodiscard]] inline auto convolution_naive(const VecI64& a, const VecI64& b, I64 mod) -> VecI64 {
  if (a.empty() || b.empty()) return {};
  VecI64 result(isz(a) + isz(b) - 1, 0);
  FOR(i, isz(a)) {
    if (a[i] == 0) continue;
    FOR(j, isz(b)) result[i + j] = (result[i + j] + a[i] * b[j]) % mod;
  }
  return result;
}

/**
 * @brief Convolution of two sequences modulo an arbitrary modulus.
 *
 * @details The modulus need not be NTT-friendly. Each input is transformed
 * three times, once under each of the primes above, and the residues are
 * recombined by garner3(). Because the primes multiply to more than 5.9e25 and
 * a coefficient of the true integer convolution is at most
 * min(|a|,|b|) * (mod-1)^2, the reconstruction is exact for every modulus below
 * 2^31 at any length this transform can reach.
 *
 * When @p mod is 998244353 the transform is done once directly, and short
 * inputs fall back to the schoolbook product, where the transform would only
 * cost more.
 *
 * @param a First sequence; entries are reduced internally, so they may be
 * unreduced or negative.
 * @param b Second sequence, same convention.
 * @param mod Modulus of the result; has to be positive and below 2^31.
 *
 * @return The sequence c with c[k] = sum over i+j=k of a[i]*b[j] modulo
 * @p mod, of length |a| + |b| - 1, or empty when either input is empty.
 *
 * @code
 * const VecI64 product = convolution_mod({1, 2, 3}, {4, 5}, 1'000'000'007LL);
 * // product == {4, 13, 22, 15}
 * @endcode
 */
[[nodiscard]] inline auto convolution_mod(const VecI64& a, const VecI64& b, I64 mod) -> VecI64 {
  my_assert(mod > 0);
  my_assert(mod < (1LL << 31));
  if (a.empty() || b.empty()) return {};

  const I32 n = isz(a);
  const I32 m = isz(b);

  VecI64 lhs(n);
  VecI64 rhs(m);
  FOR(i, n) lhs[i] = mod_floor(a[i], mod);
  FOR(i, m) rhs[i] = mod_floor(b[i], mod);

  // Below this size the three transforms cost more than the direct product.
  if (std::min(n, m) <= 60) return convolution_naive(lhs, rhs, mod);

  if (mod == MOD2) return NTT::multiply(std::move(lhs), std::move(rhs));

  const VecI64 c0 = ConvolutionNTT0::multiply(lhs, rhs);
  const VecI64 c1 = ConvolutionNTT1::multiply(lhs, rhs);
  const VecI64 c2 = ConvolutionNTT2::multiply(std::move(lhs), std::move(rhs));

  VecI64 result(n + m - 1);
  FOR(i, n + m - 1) {
    result[i] = garner3<CONVOLUTION_PRIME_0, CONVOLUTION_PRIME_1, CONVOLUTION_PRIME_2>(
        c0[i], c1[i], c2[i], mod);
  }
  return result;
}

#endif
