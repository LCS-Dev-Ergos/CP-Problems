#pragma once
#include "templates/core/TypeTraits.hpp"

//===----------------------------------------------------------------------===//
/* Core Concept Vocabulary */

namespace cp {

template <bool B, class T, class F>
using Conditional = std::conditional_t<B, T, F>;

template <class T, class U>
concept Same = std::same_as<remove_cvref_t<T>, remove_cvref_t<U>>;

template <class T>
concept Int = std::integral<remove_cvref_t<T>> || detail::is_extended_integral_v<remove_cvref_t<T>>;

// clang-format off
template <class T>
concept Signed = Int<T> && (std::is_signed_v<remove_cvref_t<T>>
#if HAS_INT128
  || std::same_as<remove_cvref_t<T>, I128>
#endif
);
// clang-format on

template <class T>
concept Unsigned = Int<T> && !Signed<T>;

template <class T>
concept NonBoolInt = Int<T> && !std::same_as<remove_cvref_t<T>, bool>;

template <class T>
concept Float = std::floating_point<remove_cvref_t<T>>;

template <class T>
concept Arithmetic = Int<T> || Float<T>;

template <class T>
concept Enum = std::is_enum_v<remove_cvref_t<T>>;

template <class F, class... Args>
concept Predicate = std::predicate<F, Args...>;

template <class T>
concept Hashable = requires(const remove_cvref_t<T>& value) {
  { std::hash<remove_cvref_t<T>>{}(value) } -> std::convertible_to<std::size_t>;
};

} // namespace cp
