#ifndef CP_MODULES_DATA_STRUCTURES_BIT_DYNAMIC_BITSET_HPP
#define CP_MODULES_DATA_STRUCTURES_BIT_DYNAMIC_BITSET_HPP

#include "../_Common.hpp"

/// @brief Dynamic bitset backed by 64-bit words.
struct DynamicBitset {
  I32 n_bits = 0;
  VecU64 words;

  explicit DynamicBitset() = default;
  explicit DynamicBitset(const I32 n) { init(n); }

  void init(const I32 n) {
    n_bits = n;
    const I32 w = (n + 63) >> 6;
    words.assign(w, 0);
  }

  /// @brief Sets bit at position @p pos.
  inline void set_bit(const I32 pos) { words[pos >> 6] |= (U64(1) << (pos & 63)); }

  /// @brief Checks whether bit at @p pos is set.
  [[nodiscard]] inline auto test_bit(const I32 pos) const -> bool {
    return ((words[pos >> 6] >> (pos & 63)) & U64(1)) != 0;
  }

  /// @brief Bitwise OR with @p other.
  inline void or_with(const DynamicBitset& other) {
    const I32 w = isz(words);
    for (I32 i = 0; i < w; ++i) words[i] |= other.words[i];
  }

  /// @brief Logical right shift by one bit.
  inline void shift_right_one() {
    U64 carry = 0;
    for (I32 i = isz(words) - 1; i >= 0; --i) {
      const U64 next_carry = (words[i] & U64(1)) << 63;
      words[i] = (words[i] >> 1) | carry;
      carry = next_carry;
    }
    trim_unused_bits();
  }

private:
  inline void trim_unused_bits() {
    if (words.empty()) return;
    const I32 rem = n_bits & 63;
    if (rem == 0) return;
    const U64 mask = (U64(1) << rem) - 1;
    words.back() &= mask;
  }
};

#endif
