#pragma once
#include "templates/core/ContainerAliases.hpp"
#include "templates/core/IdiomAliases.hpp"
#include "templates/core/Macros.hpp"

//===----------------------------------------------------------------------===//
/* Container Utilities and Algorithms */

template <class T>
auto make_vec2(Size n1, Size n2) {
  return Vec(n1, Vec<T>(n2));
}

template <class T>
auto make_vec2(Size n1, Size n2, const T& value) {
  return Vec(n1, Vec<T>(n2, value));
}

template <class T>
auto make_vec3(Size n1, Size n2, Size n3) {
  return Vec(n1, Vec(n2, Vec<T>(n3)));
}

template <class T>
auto make_vec3(Size n1, Size n2, Size n3, const T& value) {
  return Vec(n1, Vec(n2, Vec<T>(n3, value)));
}

template <class T>
auto make_vec4(Size n1, Size n2, Size n3, Size n4) {
  return Vec(n1, Vec(n2, Vec(n3, Vec<T>(n4))));
}

template <class T>
auto make_vec4(Size n1, Size n2, Size n3, Size n4, const T& value) {
  return Vec(n1, Vec(n2, Vec(n3, Vec<T>(n4, value))));
}

template <typename F>
  requires cp::Predicate<F&, I64>
I64 binary_search(F&& predicate, I64 left, I64 right) {
  my_assert(left < right);
  while (left + 1 < right) {
    I64 mid = left + (right - left) / 2;
    (predicate(mid) ? left : right) = mid;
  }
  return left;
}

template <typename F>
  requires cp::Predicate<F&, F64>
F64 binary_search_real(F&& predicate, F64 left, F64 right, I32 iterations = 100) {
  for (I32 i = 0; i < iterations; ++i) {
    F64 mid = left + (right - left) / 2;
    (predicate(mid) ? left : right) = mid;
  }
  return left + (right - left) / 2;
}

template <typename T>
VecI32 argsort(const Vec<T>& v, bool reverse = false) {
  my_assert(v.size() <= as<Size>(Limits<I32>::max()) && "argsort(): input too large for I32 indices.");
  VecI32 indices(v.size());
  std::iota(indices.begin(), indices.end(), I32(0));
  if (reverse) {
    std::ranges::sort(indices, [&](I32 i, I32 j) { return v[i] == v[j] ? i < j : v[i] > v[j]; });
  } else {
    std::ranges::sort(indices, [&](I32 i, I32 j) { return v[i] == v[j] ? i < j : v[i] < v[j]; });
  }
  return indices;
}

template <typename T>
Vec<T> rearrange(const Vec<T>& v, const VecI32& indices) {
  Vec<T> result(indices.size());
  for (Size i = 0; i < indices.size(); ++i)
    result[i] = v[indices[i]];
  return result;
}

template <typename T>
Vec<T> cumsum(const Vec<T>& v, bool include_zero = true) {
  Vec<T> result(v.size() + (include_zero ? 1 : 0));
  if (v.empty())
    return result;
  if (include_zero) {
    for (Size i = 0; i < v.size(); ++i)
      result[i + 1] = result[i] + v[i];
  } else {
    result[0] = v[0];
    for (Size i = 1; i < v.size(); ++i)
      result[i] = result[i - 1] + v[i];
  }
  return result;
}

// Concatenate multiple containers into a single vector.
template <typename T, typename... Containers>
void concat(Vec<T>& dest, const Containers&... sources) {
  (dest.insert(dest.end(), sources.begin(), sources.end()), ...);
}

// Typed accumulation (for ModInt, double, or custom types):
template <typename ReturnT, typename Container>
ReturnT sum_as(const Container& c) {
  return std::accumulate(c.begin(), c.end(), ReturnT{});
}

// Convert a string to a vector of integers based on a base character.
inline VecI32 string_to_ints(const String& s, char base_char = 'a') {
  my_assert(s.size() <= as<Size>(Limits<I32>::max()) && "string_to_ints(): input too large for I32 output.");
  VecI32 result(s.size());
  for (Size i = 0; i < s.size(); ++i) {
    result[i] = s[i] == '?' ? -1 : s[i] - base_char;
  }
  return result;
}

// pop_val utilities for different containers (move-aware):
template <typename T>
T pop_val(std::deque<T>& container) {
  my_assert(!container.empty());
  T element = std::move(container.front());
  container.pop_front();
  return element;
}

// priority_queue::top() returns const T&, so we copy (move would no-op).
template <typename T>
T pop_val(MinPriorityQueue<T>& container) {
  my_assert(!container.empty());
  T element = container.top();
  container.pop();
  return element;
}

template <typename T>
T pop_val(PriorityQueue<T>& container) {
  my_assert(!container.empty());
  T element = container.top();
  container.pop();
  return element;
}

template <typename T>
T pop_val(Vec<T>& container) {
  my_assert(!container.empty());
  T element = std::move(container.back());
  container.pop_back();
  return element;
}

template <typename T>
T pop_val(Queue<T>& container) {
  my_assert(!container.empty());
  T element = std::move(container.front());
  container.pop();
  return element;
}

template <typename T>
T pop_val(Stack<T>& container) {
  my_assert(!container.empty());
  T element = std::move(container.top());
  container.pop();
  return element;
}
