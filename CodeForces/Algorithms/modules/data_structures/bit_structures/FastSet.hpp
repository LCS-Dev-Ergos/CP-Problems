#ifndef CP_MODULES_DATA_STRUCTURES_FAST_SET_HPP
#define CP_MODULES_DATA_STRUCTURES_FAST_SET_HPP

#include "../_Common.hpp"

#include <cassert>

/// @brief Hierarchical 64-ary ordered set on the range [0, n).
struct FastSet {
  static constexpr I32 B = 64;

  I32 n = 0;
  I32 cnt = 0;
  Vec<VecU64> seg;

  FastSet() = default;
  explicit FastSet(I32 n_) { init(n_); }

  template <typename F>
  FastSet(I32 n_, F&& pred) {
    build(n_, std::forward<F>(pred));
  }

  /// @brief Reinitializes the structure with an empty universe [0, n).
  void init(I32 n_) {
    assert(n_ >= 0);
    n = n_;
    cnt = 0;
    seg.clear();
    I32 m = n;
    do {
      seg.eb((m + B - 1) / B, 0);
      m = (m + B - 1) / B;
    } while (m > 1);
  }

  /// @brief Rebuilds from a predicate over indices.
  template <typename F>
  void build(I32 n_, F&& pred) {
    init(n_);
    FOR(i, n) {
      if (pred(i)) insert(i);
    }
  }

  /// @brief Removes all present indices.
  void clear() {
    for (auto& level : seg) {
      std::fill(level.begin(), level.end(), 0);
    }
    cnt = 0;
  }

  /// @brief Universe size.
  [[nodiscard]] I32 size() const { return n; }

  /// @brief Number of stored indices.
  [[nodiscard]] I32 count() const { return cnt; }

  /// @brief Returns whether the set is empty.
  [[nodiscard]] bool empty() const { return cnt == 0; }

  /// @brief Returns whether index @p i is present.
  [[nodiscard]] bool contains(I32 i) const {
    assert(0 <= i && i < n);
    return (seg[0][i >> 6] >> (i & 63)) & U64(1);
  }

  /// @brief Alias for @ref contains.
  [[nodiscard]] bool operator[](I32 i) const { return contains(i); }

  /// @brief Inserts index @p i if absent.
  bool insert(I32 i) {
    assert(0 <= i && i < n);
    if (contains(i)) return false;
    ++cnt;
    for (I32 h = 0; h < isz(seg); ++h) {
      seg[h][i >> 6] |= U64(1) << (i & 63);
      i >>= 6;
    }
    return true;
  }

  /// @brief Erases index @p i if present.
  bool erase(I32 i) {
    assert(0 <= i && i < n);
    if (!contains(i)) return false;
    --cnt;
    for (I32 h = 0; h < isz(seg); ++h) {
      I32 block = i >> 6;
      seg[h][block] &= ~(U64(1) << (i & 63));
      if (seg[h][block] != 0) break;
      i = block;
    }
    return true;
  }

  /// @brief Smallest stored index in [i, n), or n if none exists.
  [[nodiscard]] I32 next(I32 i) const {
    if (i < 0) i = 0;
    if (i >= n) return n;
    FOR(h, isz(seg)) {
      if ((i >> 6) >= isz(seg[h])) break;
      U64 bits = seg[h][i >> 6] >> (i & 63);
      if (bits == 0) {
        i = (i >> 6) + 1;
        continue;
      }
      i += first_bit(bits);
      for (I32 g = h - 1; g >= 0; --g) {
        i <<= 6;
        i += first_bit(seg[g][i >> 6]);
      }
      return i < n ? i : n;
    }
    return n;
  }

  /// @brief Largest stored index in [0, i], or -1 if none exists.
  [[nodiscard]] I32 prev(I32 i) const {
    if (i >= n) i = n - 1;
    if (i < 0) return -1;
    FOR(h, isz(seg)) {
      if (i < 0) break;
      U64 bits = seg[h][i >> 6] << (63 - (i & 63));
      if (bits == 0) {
        i = (i >> 6) - 1;
        continue;
      }
      i -= last_bit(bits);
      for (I32 g = h - 1; g >= 0; --g) {
        i <<= 6;
        i += last_bit(seg[g][i >> 6]);
      }
      return i;
    }
    return -1;
  }

  /// @brief Returns whether any stored index lies in [l, r).
  [[nodiscard]] bool any(I32 l, I32 r) const {
    if (r <= l) return false;
    return next(l) < r;
  }

  /// @brief Enumerates stored indices in [l, r) in increasing order.
  template <typename F>
  void enumerate(I32 l, I32 r, F&& fn) const {
    for (I32 x = next(l); x < r; x = next(x + 1)) fn(x);
  }

 private:
  [[nodiscard]] static I32 first_bit(U64 x) {
    return __builtin_ctzll(x);
  }

  [[nodiscard]] static I32 last_bit(U64 x) {
    return __builtin_clzll(x);
  }
};

#endif
