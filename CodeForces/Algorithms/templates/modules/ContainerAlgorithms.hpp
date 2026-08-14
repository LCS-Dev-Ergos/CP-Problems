#pragma once
#include "templates/core/ContainerAliases.hpp"
#include "templates/core/Contracts.hpp"
#include "templates/core/CoreConcepts.hpp"

//=====----- [ Container Alg ] ------------------------------------------=====//

template <typename T>
VecI32 argsort(const Vec<T>& values, bool reverse = false) {
  CP_EXPECT(values.size() <= as<Size>(Limits<I32>::max()),
            "argsort(): input too large for I32 indices.");
  VecI32 indices(values.size());
  std::iota(indices.begin(), indices.end(), I32(0));
  const auto before = [&](I32 i, I32 j) {
    if (values[i] == values[j])
      return i < j;
    return reverse ? values[i] > values[j] : values[i] < values[j];
  };
  std::ranges::sort(indices, before);
  return indices;
}

template <typename T>
Vec<T> rearrange(const Vec<T>& values, const VecI32& indices) {
  Vec<T> result;
  result.reserve(indices.size());
  for (I32 index : indices) {
    CP_EXPECT(index >= 0 && as<Size>(index) < values.size(),
              "rearrange(): index outside source container.");
    result.push_back(values[as<Size>(index)]);
  }
  return result;
}

template <typename T>
Vec<T> cumsum(const Vec<T>& values, bool include_zero = true) {
  Vec<T> result(values.size() + as<Size>(include_zero));
  if (values.empty())
    return result;

  auto checked_add = [](const T& lhs, const T& rhs) -> T {
    if constexpr (cp::Signed<T>) {
      T sum{};
      CP_EXPECT(!__builtin_add_overflow(lhs, rhs, &sum), "cumsum(): signed overflow.");
      return sum;
    } else {
      return lhs + rhs;
    }
  };

  if (include_zero) {
    for (Size i = 0; i < values.size(); ++i)
      result[i + 1] = checked_add(result[i], values[i]);
  } else {
    result[0] = values[0];
    for (Size i = 1; i < values.size(); ++i)
      result[i] = checked_add(result[i - 1], values[i]);
  }
  return result;
}

template <typename T, typename... Containers>
void concat(Vec<T>& destination, const Containers&... sources) {
  (destination.insert(destination.end(), sources.begin(), sources.end()), ...);
}

template <typename ReturnT, typename Container>
ReturnT sum_as(const Container& container) {
  return std::accumulate(container.begin(), container.end(), ReturnT{});
}

inline VecI32 string_to_ints(const String& value, char base_char = 'a') {
  CP_EXPECT(value.size() <= as<Size>(Limits<I32>::max()),
            "string_to_ints(): input too large for I32 output.");
  VecI32 result(value.size());
  for (Size i = 0; i < value.size(); ++i)
    result[i] = value[i] == '?' ? -1 : value[i] - base_char;
  return result;
}

template <typename Container>
auto pop_val(Container& container) {
  CP_EXPECT(!container.empty(), "pop_val(): empty container.");
  if constexpr (requires { container.front(); container.pop_front(); }) {
    auto value = std::move(container.front());
    container.pop_front();
    return value;
  } else if constexpr (requires { container.back(); container.pop_back(); }) {
    auto value = std::move(container.back());
    container.pop_back();
    return value;
  } else if constexpr (requires { container.front(); container.pop(); }) {
    auto value = std::move(container.front());
    container.pop();
    return value;
  } else {
    auto value = container.top();
    container.pop();
    return value;
  }
}
