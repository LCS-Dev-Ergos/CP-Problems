#ifndef CP_MODULES_GRAPH_DETAIL_WEIGHT_HPP
#define CP_MODULES_GRAPH_DETAIL_WEIGHT_HPP

#include "templates/core/CoreConcepts.hpp"

namespace cp::graph_detail {

template <class T>
[[gnu::always_inline]] inline bool checked_add(T lhs, T rhs, T& out) {
  if constexpr (cp::Int<T>)
    return !__builtin_add_overflow(lhs, rhs, &out);
  out = lhs + rhs;
  return true;
}

} // namespace cp::graph_detail

#endif
