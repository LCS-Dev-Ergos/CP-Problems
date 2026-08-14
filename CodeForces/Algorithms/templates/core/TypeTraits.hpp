#pragma once
#include "templates/core/ScalarTypes.hpp"

//=====----- [ Traits ] -------------------------------------------------=====//

template <typename To>
[[gnu::always_inline]] constexpr To as(auto&& x)
    noexcept(noexcept(static_cast<To>(std::forward<decltype(x)>(x)))) {
  return static_cast<To>(std::forward<decltype(x)>(x));
}

namespace cp {

template <class T>
using RemoveCvrefT = std::remove_cv_t<std::remove_reference_t<T>>;

namespace detail {

template <class T>
inline constexpr bool is_extended_integral_v = false;

template <class T>
inline constexpr bool is_extended_signed_v = std::is_signed_v<T>;

template <class T>
struct MakeUnsigned {
  using type = std::make_unsigned_t<T>;
};

template <>
struct MakeUnsigned<bool> {
  using type = U8;
};

// clang-format off
#if HAS_INT128
template <>
inline constexpr bool is_extended_integral_v<I128> = true;
template <>
inline constexpr bool is_extended_integral_v<U128> = true;
template <>
inline constexpr bool is_extended_signed_v<I128> = true;

template <>
struct MakeUnsigned<I128> {
  using type = U128;
};
template <>
struct MakeUnsigned<U128> {
  using type = U128;
};
#endif
// clang-format on

} // namespace detail

template <class T>
using MakeUnsignedT = typename detail::MakeUnsigned<RemoveCvrefT<T>>::type;

template <class T>
using remove_cvref_t = RemoveCvrefT<T>;

template <class T>
using make_unsigned_t = MakeUnsignedT<T>;

template <class To, class From>
  requires(detail::is_extended_integral_v<RemoveCvrefT<To>> ||
           std::integral<RemoveCvrefT<To>>) &&
          (detail::is_extended_integral_v<RemoveCvrefT<From>> ||
           std::integral<RemoveCvrefT<From>>)
[[gnu::always_inline]] constexpr bool integer_in_range(From value) noexcept {
  using Dst = RemoveCvrefT<To>;
  using Src = RemoveCvrefT<From>;
#if HAS_INT128
  using WideU = U128;
  using WideS = I128;
#else
  using WideU = U64;
  using WideS = I64;
#endif
  if constexpr (detail::is_extended_signed_v<Src>) {
    if (value < 0) {
      if constexpr (!detail::is_extended_signed_v<Dst>)
        return false;
      return as<WideS>(value) >= as<WideS>(Limits<Dst>::min());
    }
  }
  return as<WideU>(value) <= as<WideU>(Limits<Dst>::max());
}

template <class To, std::floating_point From>
  requires(detail::is_extended_integral_v<RemoveCvrefT<To>> ||
           std::integral<RemoveCvrefT<To>>)
[[gnu::always_inline]] constexpr bool floating_in_integer_range(From value) noexcept {
  using Dst = RemoveCvrefT<To>;
  if (!std::isfinite(value))
    return false;

  F80 upper_exclusive = 1;
  for (I32 digit = 0; digit < Limits<Dst>::digits; ++digit)
    upper_exclusive *= 2;
  const F80 wide_value = as<F80>(value);
  if constexpr (detail::is_extended_signed_v<Dst>)
    return wide_value >= -upper_exclusive && wide_value < upper_exclusive;
  return wide_value >= 0 && wide_value < upper_exclusive;
}

} // namespace cp
