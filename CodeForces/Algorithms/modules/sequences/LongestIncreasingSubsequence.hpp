#ifndef CP_MODULES_SEQUENCES_LONGEST_INCREASING_SUBSEQUENCE_HPP
#define CP_MODULES_SEQUENCES_LONGEST_INCREASING_SUBSEQUENCE_HPP

#include "_Common.hpp"

/**
 * @file LongestIncreasingSubsequence.hpp
 * @brief Reconstructs a longest increasing subsequence in O(N log N).
 */

/// @brief Indices of one longest increasing subsequence.
template <bool Strict = true, typename T, typename Compare = std::less<T>>
[[nodiscard]] auto longest_increasing_subsequence_indices(const Vec<T>& a, Compare comp = {})
    -> VecI32 {
  const I32 n = isz(a);
  Vec<T> tail;
  VecI32 tail_idx;
  VecI32 prev(n, -1);
  tail.reserve(n);
  tail_idx.reserve(n);

  FOR(i, n) {
    const auto it = [&] {
      if constexpr (Strict) return std::lower_bound(all(tail), a[i], comp);
      return std::upper_bound(all(tail), a[i], comp);
    }();
    const I32 p = I32(it - tail.begin());
    if (p > 0) prev[i] = tail_idx[p - 1];
    if (p == isz(tail)) {
      tail.push_back(a[i]);
      tail_idx.push_back(i);
    } else {
      tail[p] = a[i];
      tail_idx[p] = i;
    }
  }

  VecI32 ans(isz(tail));
  I32 cur = tail_idx.empty() ? -1 : tail_idx.back();
  FOR_R(i, isz(ans)) {
    ans[i] = cur;
    cur = prev[cur];
  }
  return ans;
}

/// @brief Length of a longest increasing subsequence.
template <bool Strict = true, typename T, typename Compare = std::less<T>>
[[nodiscard]] auto longest_increasing_subsequence_length(const Vec<T>& a, Compare comp = {})
    -> I32 {
  Vec<T> tail;
  tail.reserve(a.size());
  for (const T& x : a) {
    const auto it = [&] {
      if constexpr (Strict) return std::lower_bound(all(tail), x, comp);
      return std::upper_bound(all(tail), x, comp);
    }();
    if (it == tail.end()) tail.push_back(x);
    else *it = x;
  }
  return isz(tail);
}

#endif
