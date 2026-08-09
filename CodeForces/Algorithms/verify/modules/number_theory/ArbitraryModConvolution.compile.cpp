#include "modules/number_theory/ArbitraryModConvolution.hpp"

int main() {
  // The three primes have to stay NTT-friendly enough for a 2^24 transform.
  static_assert(ConvolutionNTT0::MAX_LOG_N >= 24);
  static_assert(ConvolutionNTT1::MAX_LOG_N >= 24);
  static_assert(ConvolutionNTT2::MAX_LOG_N >= 24);

  // The reconstruction is fully resolvable at compile time.
  static_assert((garner3<CONVOLUTION_PRIME_0, CONVOLUTION_PRIME_1, CONVOLUTION_PRIME_2>(
                     1, 1, 1, 1'000'000'007LL)) == 1);

  const VecI64 product = convolution_mod({1, 2, 3}, {4, 5}, 1'000'000'007LL);
  return (product == VecI64{4, 13, 22, 15}) ? 0 : 1;
}
