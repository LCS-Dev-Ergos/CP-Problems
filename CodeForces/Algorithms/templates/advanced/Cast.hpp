#pragma once
#include "templates/advanced/RangeStreamConcepts.hpp"
#include "templates/core/ContainerAliases.hpp"
#include "templates/core/Debug.hpp"
#include "templates/core/TypeTraits.hpp"

//===----------------------------------------------------------------------===//
/* Explicit Cast Helpers and Narrowing Policy */

namespace cp::cast {

template <class To, class From>
[[gnu::always_inline]] constexpr To narrow(From value) {
  using Src = RemoveCvrefT<From>;
  using Dst = RemoveCvrefT<To>;
  if constexpr (Int<Src> && Int<Dst>)
    CP_EXPECT(integer_in_range<Dst>(value), "narrow(): integral value out of destination range.");
  else if constexpr (Float<Src> && Int<Dst>)
    CP_EXPECT(floating_in_integer_range<Dst>(value),
              "narrow(): floating value out of destination range.");
  return ::as<To>(value);
}

template <class To, class From>
[[nodiscard]] constexpr auto try_narrow(From value) -> Optional<RemoveCvrefT<To>> {
  using Src = RemoveCvrefT<From>;
  using Dst = RemoveCvrefT<To>;
  if constexpr (Int<Src> && Int<Dst>) {
    if (!integer_in_range<Dst>(value))
      return std::nullopt;
  } else if constexpr (Float<Src> && Int<Dst>) {
    if (!floating_in_integer_range<Dst>(value))
      return std::nullopt;
  }
  return ::as<Dst>(value);
}

//clang-format off
template <class To, class From>
[[gnu::always_inline]] constexpr To saturate(From value) {
  using Src = RemoveCvrefT<From>;
  using Dst = RemoveCvrefT<To>;
  if constexpr (Int<Src> && Int<Dst>) {
    if (integer_in_range<Dst>(value))
      return ::as<To>(value);
    if constexpr (std::is_signed_v<Src>
    #if HAS_INT128
      || Same<Src, I128>
    #endif
    ) {
      if (value < 0) {
        if constexpr (std::is_signed_v<Dst>
      #if HAS_INT128
        || Same<Dst, I128>
      #endif
        )
          return ::as<To>(Limits<Dst>::min());
        return Dst{};
      }
    }
    return ::as<To>(Limits<Dst>::max());
  } else if constexpr (Float<Src> && Int<Dst>) {
    if (std::isnan(value))
      return Dst{};
    if (value < 0)
      return Limits<Dst>::min();
    if (!floating_in_integer_range<Dst>(value))
      return Limits<Dst>::max();
  }
  return ::as<To>(value);
}
//clang-format on

template <Enum E>
[[gnu::always_inline]] constexpr auto to_underlying(E value) noexcept
    -> std::underlying_type_t<RemoveCvrefT<E>> {
  return ::as<std::underlying_type_t<RemoveCvrefT<E>>>(value);
}

template <Enum E, Int I>
[[gnu::always_inline]] constexpr E enum_cast(I value) noexcept {
  return ::as<E>(value);
}

} // namespace cp::cast

template <typename To>
[[gnu::always_inline]] constexpr To narrow_as(auto x) {
  return cp::cast::narrow<To>(x);
}
