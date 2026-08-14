#pragma once
#include "templates/core/TypeTraits.hpp"

//=====----- [ Concepts ] -----------------------------------------------=====//

namespace cp {

template <bool B, class T, class F>
using Conditional = std::conditional_t<B, T, F>;

template <class T, class U>
concept Same = std::same_as<RemoveCvrefT<T>, RemoveCvrefT<U>>;

template <class T>
concept Int = std::integral<RemoveCvrefT<T>> || detail::is_extended_integral_v<RemoveCvrefT<T>>;

template <class T>
concept Signed = Int<T> && detail::is_extended_signed_v<RemoveCvrefT<T>>;

template <class T>
concept Unsigned = Int<T> && !Signed<T>;

template <class T>
concept NonBoolInt = Int<T> && !std::same_as<RemoveCvrefT<T>, bool>;

template <class T>
concept Float = std::floating_point<RemoveCvrefT<T>>;

template <class T>
concept Arithmetic = Int<T> || Float<T>;

template <class T>
concept Enum = std::is_enum_v<RemoveCvrefT<T>>;

template <class F, class... Args>
concept Predicate = std::predicate<F, Args...>;

template <class T>
concept Hashable = requires(const RemoveCvrefT<T>& value) {
  { std::hash<RemoveCvrefT<T>>{}(value) } -> std::convertible_to<std::size_t>;
};

} // namespace cp
