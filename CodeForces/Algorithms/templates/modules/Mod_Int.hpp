#pragma once
#include "templates/modules/IntegerMath.hpp"
#include "templates/core/Constants.hpp"
#include "templates/core/IdiomAliases.hpp"
#include "templates/core/Macros.hpp"
#include "templates/core/Debug.hpp"

//===----------------------------------------------------------------------===//
/* Advanced Modular Arithmetic */

struct Barrett {
  U32 m;
  U64 im;

  explicit constexpr Barrett(U32 mod) : m(mod), im(U64(-1) / mod + 1) {}

  constexpr U32 mod() const { return m; }

  constexpr U32 mul(U32 a, U32 b) const {
#if HAS_INT128
    U64 z = U64(a) * b;
    U64 x = U64((U128(z) * im) >> 64);
    U32 v = U32(z - x * m);
    if (m <= v)
      v += m;
    return v;
#else
    return U32((U64(a) * b) % m);
#endif
  }
};

namespace cp::modint_detail {
// Implementation layer: keep policy/base machinery out of the contest API.

template <cp::Int T>
using SignedWide =
#if HAS_INT128
    Conditional<cp::Signed<T>, I128, U128>;
#else
    Conditional<cp::Signed<T>, I64, U64>;
#endif

template <I64 MOD>
struct StaticModPolicy {
  static_assert(MOD > 0, "ModInt requires MOD > 0.");
  using Value = U64;

  static constexpr I64 mod() { return MOD; }

  template <cp::Int T>
  static constexpr Value normalize(T x) {
    using Wide = SignedWide<T>;
    Wide r = as<Wide>(x) % as<Wide>(MOD);
    if constexpr (cp::Signed<T>) {
      if (r < 0)
        r += as<Wide>(MOD);
    }
    return as<Value>(r);
  }

  static constexpr Value add(Value a, Value b) {
    a += b;
    if (a >= as<Value>(MOD))
      a -= as<Value>(MOD);
    return a;
  }

  static constexpr Value sub(Value a, Value b) { return a < b ? a + as<Value>(MOD) - b : a - b; }

  static constexpr Value mul(Value a, Value b) {
#if HAS_INT128
    return as<Value>(U128(a) * b % U128(MOD));
#else
    static_assert(MOD <= (1LL << 32), "ModInt multiplication may overflow U64 for MOD > 2^32 without __int128 support.");
    return a * b % as<Value>(MOD);
#endif
  }

  static constexpr Value inv(Value x) {
    // mod_inv already returns a canonical residue in [0, MOD).
    return as<Value>(mod_inv(as<I64>(x), MOD));
  }
};

template <U32 Id>
struct DynamicModPolicy {
  using Value = U32;
  inline static Barrett bt = Barrett(998'244'353);

  static void set_mod(U32 mod) {
    my_assert(mod > 0);
    bt = Barrett(mod);
  }

  static U32 mod() { return bt.mod(); }

  template <cp::Int T>
  static Value normalize(T x) {
    const U32 m = mod();
    using Wide  = SignedWide<T>;
    Wide r = as<Wide>(x) % as<Wide>(m);
    if constexpr (cp::Signed<T>) {
      if (r < 0)
        r += m;
    }
    return as<Value>(r);
  }

  static Value add(Value a, Value b) {
    const Value m = mod();
    return a >= m - b ? a - (m - b) : a + b;
  }

  static Value sub(Value a, Value b) {
    const Value m = mod();
    return a < b ? a + (m - b) : a - b;
  }

  static Value mul(Value a, Value b) { return bt.mul(a, b); }

  // mod_inv already returns a canonical residue in [0, mod()).
  static Value inv(Value x) { return as<Value>(mod_inv(as<I64>(x), as<I64>(mod()))); }
};

template <class Derived, class Policy>
struct ModIntBase {
  using Value = typename Policy::Value;
  Value value = 0;

  constexpr ModIntBase() = default;

  template <cp::Int T>
  constexpr ModIntBase(T x) : value(Policy::normalize(x)) {}

  constexpr Value val() const { return value; }

  static constexpr auto mod() { return Policy::mod(); }

  constexpr Derived& self() { return static_cast<Derived&>(*this); }

  constexpr const Derived& self() const { return static_cast<const Derived&>(*this); }

  constexpr Derived operator-() const { return Derived(value == 0 ? 0 : Policy::sub(0, value)); }

  constexpr Derived& operator+=(const Derived& other) & {
    value = Policy::add(value, other.value);
    return self();
  }

  constexpr Derived& operator-=(const Derived& other) & {
    value = Policy::sub(value, other.value);
    return self();
  }

  constexpr Derived& operator*=(const Derived& other) & {
    value = Policy::mul(value, other.value);
    return self();
  }

  constexpr Derived& operator/=(const Derived& other) & { return *this *= other.inverse(); }

  friend constexpr Derived operator+(Derived lhs, const Derived& rhs) {
    lhs += rhs;
    return lhs;
  }

  friend constexpr Derived operator-(Derived lhs, const Derived& rhs) {
    lhs -= rhs;
    return lhs;
  }

  friend constexpr Derived operator*(Derived lhs, const Derived& rhs) {
    lhs *= rhs;
    return lhs;
  }

  friend constexpr Derived operator/(Derived lhs, const Derived& rhs) {
    lhs /= rhs;
    return lhs;
  }

  constexpr Derived pow(I64 exp) const {
    Derived result(1), base(self());
    while (exp > 0) {
      if (exp & 1)
        result *= base;
      base *= base;
      exp >>= 1;
    }
    return result;
  }

  explicit constexpr operator I64() const { return as<I64>(value); }

  constexpr Derived inverse() const { return Derived(Policy::inv(value)); }

  friend constexpr bool operator==(const Derived& lhs, const Derived& rhs) { return lhs.value == rhs.value; }

  friend constexpr std::strong_ordering operator<=>(const Derived& lhs, const Derived& rhs) {
    return lhs.value <=> rhs.value;
  }

  friend std::ostream& operator<<(std::ostream& os, const Derived& x) { return os << x.value; }

  friend std::istream& operator>>(std::istream& is, Derived& x) {
    I64 val;
    is >> val;
    x = Derived(val);
    return is;
  }
};

} // namespace cp::modint_detail

template <I64 MOD>
struct ModInt : cp::modint_detail::ModIntBase<ModInt<MOD>, cp::modint_detail::StaticModPolicy<MOD>> {
  using Base = cp::modint_detail::ModIntBase<ModInt<MOD>, cp::modint_detail::StaticModPolicy<MOD>>;
  using Base::Base;
};

template <U32 Id>
struct DynModInt : cp::modint_detail::ModIntBase<DynModInt<Id>, cp::modint_detail::DynamicModPolicy<Id>> {
  using Base = cp::modint_detail::ModIntBase<DynModInt<Id>, cp::modint_detail::DynamicModPolicy<Id>>;
  using Base::Base;

  static void set_mod(U32 mod) { cp::modint_detail::DynamicModPolicy<Id>::set_mod(mod); }
};

using MInt  = ModInt<MOD>;
using MInt2 = ModInt<MOD2>;
using DMInt = DynModInt<0>;
