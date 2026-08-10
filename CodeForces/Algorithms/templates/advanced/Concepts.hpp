#pragma once
#include "templates/core/IdiomAliases.hpp"

//===----------------------------------------------------------------------===//
/* Range and Stream Concepts */

/*
 * Arithmetic concepts (Int/Signed/Float/Arithmetic/...) live in
 * core/IdiomAliases.hpp and are always available; this layer only adds the
 * range/stream vocabulary used by the type-safe utilities.
 */

namespace cp {

template <class T>
concept IndexLike = NonBoolInt<T>;

template <class R>
concept Range = std::ranges::range<remove_cvref_t<R>>;

template <class R>
concept SizedRange = Range<R> && requires(remove_cvref_t<R> r) { std::ranges::size(r); };

template <class T>
concept StreamReadable = requires(std::istream& is, T& value) {
  { is >> value } -> std::same_as<std::istream&>;
};

template <class T>
concept StreamWritable = requires(std::ostream& os, const T& value) {
  { os << value } -> std::same_as<std::ostream&>;
};

} // namespace cp
