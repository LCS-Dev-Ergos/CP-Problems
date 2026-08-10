#pragma once
#include "templates/advanced/Concepts.hpp"
#include "templates/core/ContainerAliases.hpp"
#include "templates/core/Debug.hpp"
#include "templates/core/TypeTraits.hpp"

//===----------------------------------------------------------------------===//
/* Explicit Cast Helpers and Narrowing Policy */

namespace cp::cast {

template <class To, class From>
[[gnu::always_inline]] constexpr To narrow(From value) {
  using Src = remove_cvref_t<From>;
  using Dst = remove_cvref_t<To>;
  if constexpr (std::integral<Src> && std::integral<Dst>) {
    my_assert(std::in_range<Dst>(value) && "narrow(): integral value out of destination range.");
  }
  return ::as<To>(value);
}

template <class To, class From>
[[nodiscard]] constexpr auto try_narrow(From value) -> Optional<remove_cvref_t<To>> {
  using Src = remove_cvref_t<From>;
  using Dst = remove_cvref_t<To>;
  if constexpr (std::integral<Src> && std::integral<Dst>) {
    if (!std::in_range<Dst>(value))
      return std::nullopt;
  }
  return ::as<Dst>(value);
}

template <class To, class From>
[[gnu::always_inline]] constexpr To saturate(From value) {
  using Src = remove_cvref_t<From>;
  using Dst = remove_cvref_t<To>;
  if constexpr (std::integral<Src> && std::integral<Dst>) {
    if (std::in_range<Dst>(value))
      return ::as<To>(value);
    if (std::cmp_less(value, Limits<Dst>::min()))
      return ::as<To>(Limits<Dst>::min());
    return ::as<To>(Limits<Dst>::max());
  }
  return ::as<To>(value);
}

template <Enum E>
[[gnu::always_inline]] constexpr auto to_underlying(E value) noexcept
    -> std::underlying_type_t<remove_cvref_t<E>> {
  return ::as<std::underlying_type_t<remove_cvref_t<E>>>(value);
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
