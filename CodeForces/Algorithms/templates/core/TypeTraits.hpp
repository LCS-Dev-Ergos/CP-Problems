#pragma once
#include "templates/core/ScalarTypes.hpp"

//===----------------------------------------------------------------------===//
/* Extended Type Traits */

template <typename To>
[[gnu::always_inline]] constexpr To as(auto&& x) noexcept {
  return static_cast<To>(std::forward<decltype(x)>(x));
}

namespace cp {

template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

namespace detail {

template <class T>
inline constexpr bool is_extended_integral_v = false;

template <class T>
struct make_unsigned {
  using type = std::make_unsigned_t<T>;
};

template <>
struct make_unsigned<bool> {
  using type = U8;
};

#if HAS_INT128
  template <>
  inline constexpr bool is_extended_integral_v<I128> = true;
  template <>
  inline constexpr bool is_extended_integral_v<U128> = true;

  template <>
  struct make_unsigned<I128> {
    using type = U128;
  };
  template <>
  struct make_unsigned<U128> {
    using type = U128;
  };
#endif

} // namespace detail

template <class T>
using make_unsigned_t = typename detail::make_unsigned<remove_cvref_t<T>>::type;

namespace detail {

template <class T, bool = std::is_integral_v<remove_cvref_t<T>> && !std::is_same_v<remove_cvref_t<T>, bool>>
struct loop_arg {
  using type = remove_cvref_t<T>;
};

template <class T>
struct loop_arg<T, true> {
  using type = std::make_signed_t<remove_cvref_t<T>>;
};

} // namespace detail

template <class... Ts>
using loop_t = std::common_type_t<I32, typename detail::loop_arg<Ts>::type...>;

template <class... Ts>
struct loop {
  using T = loop_t<Ts...>;

  template <class X>
  [[gnu::always_inline]] static constexpr T val(X x) noexcept {
    return x;
  }
};

template <class R>
[[gnu::always_inline]] constexpr I64 sz64(const R& x)
    noexcept(noexcept(std::ssize(x))) {
  return std::ssize(x);
}

template <class R>
[[gnu::always_inline]] constexpr I32 sz32(const R& x)
    noexcept(noexcept(std::ssize(x))) {
  return std::ssize(x);
}

} // namespace cp
