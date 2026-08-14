#pragma once
#include "templates/core/Contracts.hpp"
#include "templates/core/CoreConcepts.hpp"
#include "templates/core/TypeTraits.hpp"

//=====----- [ Int Math ] -----------------------------------------------=====//

namespace cp::detail {

template <cp::Unsigned T>
[[gnu::always_inline]] constexpr T mul_mod_unsigned(T a, T b, T mod) {
  CP_EXPECT(mod != 0, "mul_mod_unsigned(): modulus must be non-zero.");
  a %= mod;
#if HAS_INT128
  if constexpr (sizeof(T) <= sizeof(U64)) {
    return as<T>((as<U128>(a) * as<U128>(b)) % as<U128>(mod));
  }
#endif

  T result = 0;
  while (b > 0) {
    if ((b & 1U) != 0U) {
      if (result >= mod - a)
        result -= (mod - a);
      else
        result += a;
    }
    b >>= 1U;
    if (b == 0)
      break;
    if (a >= mod - a)
      a -= (mod - a);
    else
      a += a;
  }
  return result;
}

} // namespace cp::detail

template <cp::Signed T>
[[gnu::always_inline]] constexpr T safe_mod(T x, T mod) {
  CP_EXPECT(mod > 0, "safe_mod(): modulus must be positive.");
  x %= mod;
  if (x < 0)
    x += mod;
  return x;
}

template <cp::Signed T>
[[gnu::always_inline]] constexpr std::pair<T, T> inv_gcd(T a, T b) {
  CP_EXPECT(b > 0, "inv_gcd(): modulus must be positive.");
  a = safe_mod(a, b);
  if (a == 0)
    return {b, 0};

  T s  = b, t = a;
  T m0 = 0, m1 = 1;

  while (t) {
    T u = s / t;
    s  -= t * u;
    m0 -= m1 * u;
    std::swap(s, t);
    std::swap(m0, m1);
  }

  if (m0 < 0)
    m0 += b / s;
  return {s, m0};
}

template <cp::Signed T>
[[gnu::always_inline]] constexpr T mod_inv(T a, T mod) {
  auto [g, x] = inv_gcd(a, mod);
  CP_EXPECT(g == 1, "mod_inv(): inverse does not exist.");
  return x;
}

template <cp::Signed T>
[[gnu::always_inline]] constexpr bool merge_congruences(T& r1, T& m1, T r2, T m2) {
  CP_EXPECT(m1 > 0 && m2 > 0, "merge_congruences(): moduli must be positive.");
  r1 = safe_mod(r1, m1);
  r2 = safe_mod(r2, m2);

  const T r1_mod = safe_mod(r1, m2);
  const T target = r2 >= r1_mod ? r2 - r1_mod : m2 - (r1_mod - r2);
  auto [g, x] = inv_gcd(m1, m2);
  if (target % g != 0)
    return false;

  const T m2_g = m2 / g;
  const T lhs  = safe_mod(target / g, m2_g);
  const T rhs  = safe_mod(x, m2_g);
  using U = cp::MakeUnsignedT<T>;
  const T step = as<T>(cp::detail::mul_mod_unsigned(as<U>(lhs), as<U>(rhs), as<U>(m2_g)));

  CP_EXPECT(m1 <= Limits<T>::max() / m2_g, "merge_congruences(): lcm overflows result type.");
  r1 += step * m1;
  m1 *= m2_g;
  r1 = safe_mod(r1, m1);
  return true;
}

template <cp::Int T>
[[gnu::always_inline]] constexpr T div_floor(T a, T b) {
  CP_EXPECT(b != 0, "div_floor(): divisor must be non-zero.");
  if constexpr (cp::Signed<T>) {
    CP_EXPECT(!(a == Limits<T>::min() && b == T(-1)), "div_floor(): quotient overflows.");
    T q = a / b;
    T r = a % b;
    if (r != 0 && ((r > 0) != (b > 0)))
      --q;
    return q;
  } else {
    return a / b;
  }
}

template <cp::Int T>
[[gnu::always_inline]] constexpr T div_ceil(T a, T b) {
  CP_EXPECT(b != 0, "div_ceil(): divisor must be non-zero.");
  if constexpr (cp::Signed<T>) {
    CP_EXPECT(!(a == Limits<T>::min() && b == T(-1)), "div_ceil(): quotient overflows.");
    T q = a / b;
    T r = a % b;
    if (r != 0 && ((r > 0) == (b > 0)))
      ++q;
    return q;
  } else {
    return a / b + (a % b != 0);
  }
}

template <cp::Int T>
[[gnu::always_inline]] constexpr T mod_floor(T a, T b) {
  CP_EXPECT(b != 0, "mod_floor(): divisor must be non-zero.");
  if constexpr (cp::Signed<T>)
    CP_EXPECT(!(a == Limits<T>::min() && b == T(-1)), "mod_floor(): quotient overflows.");
  T remainder = a % b;
  if constexpr (cp::Signed<T>) {
    if (remainder != 0 && ((remainder > 0) != (b > 0)))
      remainder += b;
  }
  return remainder;
}

template <cp::Int T>
[[gnu::always_inline]] constexpr std::pair<T, T> divmod(T a, T b) {
  T q = div_floor(a, b);
  return {q, mod_floor(a, b)};
}

template <cp::Int T>
[[gnu::always_inline]] constexpr T power(T base, T exp) {
  if constexpr (cp::Signed<T>)
    CP_EXPECT(exp >= 0, "power(): negative exponent has no integer result.");
  T result = 1;
  while (exp > 0) {
    if (exp & 1) {
      if constexpr (cp::Signed<T>) {
        T next;
        CP_EXPECT(!__builtin_mul_overflow(result, base, &next), "power(): result overflows.");
        result = next;
      } else {
        result *= base;
      }
    }
    if (exp > 1) {
      if constexpr (cp::Signed<T>) {
        T next;
        CP_EXPECT(!__builtin_mul_overflow(base, base, &next), "power(): base square overflows.");
        base = next;
      } else {
        base *= base;
      }
    }
    exp >>= 1;
  }
  return result;
}

template <cp::Int T>
[[gnu::always_inline]] constexpr T mod_pow(T base, T exp, T mod) {
  CP_EXPECT(mod != 0, "mod_pow(): modulus must be non-zero.");
  if constexpr (cp::Signed<T>) {
    CP_EXPECT(mod > 0, "mod_pow(): modulus must be positive.");
    CP_EXPECT(exp >= 0, "mod_pow(): exponent must be non-negative.");
  }

  using U  = cp::MakeUnsignedT<T>;
  U umod   = as<U>(mod);
  U uexp   = as<U>(exp);
  U ubase  = as<U>(mod_floor(base, mod));
  U result = as<U>(1) % umod;

  while (uexp > 0) {
    if ((uexp & 1U) != 0U) {
      result = cp::detail::mul_mod_unsigned(result, ubase, umod);
    }
    ubase = cp::detail::mul_mod_unsigned(ubase, ubase, umod);
    uexp >>= 1U;
  }
  return as<T>(result);
}

template <cp::Int T>
[[gnu::always_inline]] inline T floor_sqrt(T x) {
  if constexpr (cp::Signed<T>) {
    CP_EXPECT(x >= 0, "floor_sqrt(): input must be non-negative.");
    if (x < 0)
      return 0;
  }

  using U = cp::MakeUnsignedT<T>;
  const U ux = as<U>(x);
  if (ux <= 1)
    return as<T>(ux);

  if constexpr (sizeof(U) > sizeof(U64)) {
    constexpr int bits = int(sizeof(U) * 8);
    U lo = 1, hi = U{1} << ((bits + 1) / 2);
    while (lo + 1 < hi) {
      const U mid = lo + (hi - lo) / 2;
      if (mid <= ux / mid)
        lo = mid;
      else
        hi = mid;
    }
    return as<T>(lo);
  }

  U r = as<U>(std::sqrt(as<F80>(ux)));
  while ((r + 1) <= ux / (r + 1))
    ++r;
  while (r > ux / r)
    --r;
  return as<T>(r);
}

template <cp::Int T>
[[gnu::always_inline]] inline T ceil_sqrt(T x) {
  using U      = cp::MakeUnsignedT<T>;
  const T root = floor_sqrt(x);
  const U uf   = as<U>(root);
  if (uf == 0)
    return 0;

  const U ux = as<U>(x);
  if (ux / uf == uf && ux % uf == 0)
    return root;
  return as<T>(uf + 1);
}
