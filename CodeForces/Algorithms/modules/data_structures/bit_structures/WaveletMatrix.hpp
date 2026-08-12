#ifndef CP_MODULES_DATA_STRUCTURES_WAVELET_MATRIX_HPP
#define CP_MODULES_DATA_STRUCTURES_WAVELET_MATRIX_HPP

#include "../_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/// @brief Static wavelet matrix over a compressed ordered domain.
template <typename T>
struct WaveletMatrix {
  I32 n = 0;
  I32 levels = 0;
  Vec<T> values;
  VecI32 mids;
  Vec2D<I32> prefix_ones;

  WaveletMatrix() = default;

  /// @brief Builds from explicit values.
  explicit WaveletMatrix(const Vec<T>& data) { build(data); }

  /// @brief Rebuilds the structure from explicit values.
  void build(const Vec<T>& data) {
    n = isz(data);
    values = data;
    std::ranges::sort(values);
    values.erase(std::ranges::unique(values).begin(), values.end());

    VecI32 cur(n);
    FOR(i, n) cur[i] = rank_of(data[i]);

    levels = 0;
    const I32 limit = std::max<I32>(isz(values), 1);
    while ((I64(1) << levels) < limit) ++levels;
    mids.assign(levels, 0);
    prefix_ones.assign(levels, VecI32(n + 1, 0));

    VecI32 nxt(n);
    FOR_R(level, levels) {
      I32 zero_count = 0;
      FOR(i, n) {
        const I32 bit = (cur[i] >> level) & 1;
        prefix_ones[level][i + 1] = prefix_ones[level][i] + bit;
        zero_count += bit == 0;
      }
      mids[level] = zero_count;

      I32 left  = 0;
      I32 right = zero_count;
      FOR(i, n) {
        if (((cur[i] >> level) & 1) == 0) nxt[left++] = cur[i];
        else nxt[right++] = cur[i];
      }
      cur.swap(nxt);
    }
  }

  /// @brief Number of stored elements.
  [[nodiscard]] auto size() const -> I32 { return n; }

  /// @brief Returns whether the structure is empty.
  [[nodiscard]] auto empty() const -> bool { return n == 0; }

  /// @brief Returns the k-th smallest value on [l, r).
  [[nodiscard]] auto kth(I32 l, I32 r, I32 k) const -> T {
    my_assert(0 <= l && l <= r && r <= n);
    my_assert(0 <= k && k < r - l);
    I32 rank = 0;
    FOR_R(level, levels) {
      const I32 left_ones = prefix_ones[level][l];
      const I32 right_ones = prefix_ones[level][r];
      const I32 zeros = (r - l) - (right_ones - left_ones);
      if (k < zeros) {
        l -= left_ones;
        r -= right_ones;
      } else {
        k -= zeros;
        rank |= 1 << level;
        l = mids[level] + left_ones;
        r = mids[level] + right_ones;
      }
    }
    return values[rank];
  }

  /// @brief Returns the lower median on [l, r).
  [[nodiscard]] auto median_lower(I32 l, I32 r) const -> T {
    my_assert(l < r);
    return kth(l, r, (r - l - 1) / 2);
  }

  /// @brief Returns the upper median on [l, r).
  [[nodiscard]] auto median_upper(I32 l, I32 r) const -> T {
    my_assert(l < r);
    return kth(l, r, (r - l) / 2);
  }

  /// @brief Counts values in [l, r) that are < upper.
  [[nodiscard]] auto prefix_count(I32 l, I32 r, const T& upper) const -> I32 {
    return prefix_count_rank(l, r, lower_rank(upper));
  }

  /// @brief Counts values in [l, r) that lie in [lo, hi).
  [[nodiscard]] auto count_range(I32 l, I32 r, const T& lo, const T& hi) const -> I32 {
    if (!(lo < hi)) return 0;
    return prefix_count(l, r, hi) - prefix_count(l, r, lo);
  }

  /// @brief Counts occurrences of @p value on [l, r).
  [[nodiscard]] auto count_value(I32 l, I32 r, const T& value) const -> I32 {
    const I32 left  = lower_rank(value);
    const I32 right = upper_rank(value);
    return prefix_count_rank(l, r, right) - prefix_count_rank(l, r, left);
  }

  /// @brief Returns the minimum value in [l, r) that is >= lower, if any.
  [[nodiscard]] auto min_ge(I32 l, I32 r, const T& lower) const -> Optional<T> {
    my_assert(0 <= l && l <= r && r <= n);
    const I32 skipped = prefix_count(l, r, lower);
    if (skipped == r - l) return std::nullopt;
    return kth(l, r, skipped);
  }

  /// @brief Returns the maximum value in [l, r) that is < upper, if any.
  [[nodiscard]] auto max_lt(I32 l, I32 r, const T& upper) const -> Optional<T> {
    my_assert(0 <= l && l <= r && r <= n);
    const I32 cnt = prefix_count(l, r, upper);
    if (cnt == 0) return std::nullopt;
    return kth(l, r, cnt - 1);
  }

private:
  [[nodiscard]] auto rank_of(const T& value) const -> I32 {
    return as<I32>(std::ranges::lower_bound(values, value) - values.begin());
  }

  [[nodiscard]] auto lower_rank(const T& value) const -> I32 {
    return as<I32>(std::ranges::lower_bound(values, value) - values.begin());
  }

  [[nodiscard]] auto upper_rank(const T& value) const -> I32 {
    return as<I32>(std::ranges::upper_bound(values, value) - values.begin());
  }

  [[nodiscard]] auto prefix_count_rank(I32 l, I32 r, I32 upper_rank_exclusive) const -> I32 {
    my_assert(0 <= l && l <= r && r <= n);
    upper_rank_exclusive = std::clamp<I32>(upper_rank_exclusive, 0, isz(values));
    I32 cnt = 0;
    FOR_R(level, levels) {
      const I32 left_ones  = prefix_ones[level][l];
      const I32 right_ones = prefix_ones[level][r];
      if ((upper_rank_exclusive >> level) & 1) {
        cnt += (r - l) - (right_ones - left_ones);
        l = mids[level] + left_ones;
        r = mids[level] + right_ones;
      } else {
        l -= left_ones;
        r -= right_ones;
      }
    }
    return cnt;
  }
};

#endif
