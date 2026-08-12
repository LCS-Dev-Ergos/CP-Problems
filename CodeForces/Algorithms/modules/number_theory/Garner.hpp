#ifndef CP_MODULES_NUMBER_THEORY_GARNER_HPP
#define CP_MODULES_NUMBER_THEORY_GARNER_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"
#include "Euclid.hpp"

/**
 * @brief Garner algorithm (mixed radix CRT) for pairwise-coprime moduli.
 *
 * @details Reconstructs x from x ≡ residues[i] (mod moduli[i]) and returns
 * x modulo @p result_mod. Returns -1 when input is invalid or reconstruction
 * prerequisites are not satisfied.
 */
inline I64 garner(const Vec<I64>& residues, const Vec<I64>& moduli, I64 result_mod) {
  if (residues.size() != moduli.size()) return -1;
  if (result_mod <= 0) return -1;
  if (residues.empty()) return 0;

  I32 k = isz(residues);
  FOR(i, k) {
    if (moduli[i] <= 1) return -1;
  }
  FOR(i, k) {
    FOR(j, i + 1, k) {
      if (std::gcd(moduli[i], moduli[j]) != 1) return -1;
    }
  }

  auto norm_mod = [](I64 x, I64 mod) {
    x %= mod;
    if (x < 0) x += mod;
    return x;
  };

  Vec<I64> mods = moduli;
  mods.push_back(result_mod);

  Vec<I64> coeffs((k + 1), 1);
  Vec<I64> constants((k + 1), 0);

  FOR(i, k) {
    I64 mi = mods[i];
    I64 ai = norm_mod(residues[i], mi);
    I64 coeff_i = norm_mod(coeffs[i], mi);
    I64 inv = mod_inverse<I64>(coeff_i, mi);
    if (inv == -1) return -1;

    I64 delta = norm_mod(ai - constants[i], mi);
    I64 t = as<I64>((as<I128>(delta) * inv) % mi);

    FOR(j, i + 1, k + 1) {
      I64 mj = mods[j];
      constants[j] = as<I64>((as<I128>(constants[j]) + as<I128>(coeffs[j]) * t) % mj);
      coeffs[j] = as<I64>((as<I128>(coeffs[j]) * mi) % mj);
    }
  }

  return constants[k];
}

#endif
