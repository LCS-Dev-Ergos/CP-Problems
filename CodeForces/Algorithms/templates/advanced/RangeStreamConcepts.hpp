#pragma once
#include "templates/core/CoreConcepts.hpp"

//=====----- [ Range Concepts ] -----------------------------------------=====//

namespace cp {

template <class T>
concept IndexLike = NonBoolInt<T>;

template <class R>
concept Range = std::ranges::range<RemoveCvrefT<R>>;

template <class R>
concept SizedRange = Range<R> && requires(RemoveCvrefT<R> r) { std::ranges::size(r); };

template <class T>
concept StreamReadable = requires(std::istream& is, T& value) {
  { is >> value } -> std::same_as<std::istream&>;
};

template <class T>
concept StreamWritable = requires(std::ostream& os, const T& value) {
  { os << value } -> std::same_as<std::ostream&>;
};

} // namespace cp
