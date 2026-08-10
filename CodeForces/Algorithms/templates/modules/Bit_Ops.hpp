#pragma once
#include "templates/core/Debug.hpp"
#include "templates/core/IdiomAliases.hpp"
#include "templates/core/TypeTraits.hpp"

//===----------------------------------------------------------------------===//
/* Advanced Bitwise Operations */

namespace cp::detail {

template <class T>
inline constexpr bool std_bit_native_v =
    std::is_unsigned_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char> &&
    !std::is_same_v<T, char8_t> && !std::is_same_v<T, char16_t> &&
    !std::is_same_v<T, char32_t> && !std::is_same_v<T, wchar_t>;

template <class T>
concept OwnBitOps = cp::Int<T> && !std_bit_native_v<std::remove_cv_t<T>>;

} // namespace cp::detail

using std::bit_ceil;
using std::bit_floor;
using std::bit_width;
using std::popcount;

template <cp::detail::OwnBitOps T>
[[gnu::always_inline]] constexpr I32 popcount(T x) {
  using Raw = std::remove_cv_t<T>;
  using U   = cp::make_unsigned_t<Raw>;
  if constexpr (sizeof(Raw) <= 4)
    return __builtin_popcount(U32(U(x)));
  else if constexpr (sizeof(Raw) <= 8)
    return __builtin_popcountll(U64(U(x)));
  else {
    const U ux = U(x);
    return __builtin_popcountll(U64(ux)) + __builtin_popcountll(U64(ux >> 64));
  }
}

template <cp::Int T>
[[gnu::always_inline]] constexpr I32 leading_zeros(T x) {
  using Raw = std::remove_cv_t<T>;
  using U   = cp::make_unsigned_t<Raw>;
  U ux = U(x);
  if (ux == 0)
    return sizeof(Raw) * 8;
  if constexpr (sizeof(Raw) <= 4) {
    return __builtin_clz(U32(ux)) - (32 - I32(sizeof(Raw) * 8));
  } else if constexpr (sizeof(Raw) <= 8) {
    return __builtin_clzll(U64(ux)) - (64 - I32(sizeof(Raw) * 8));
  } else {
    const U64 hi = U64(ux >> 64);
    if (hi != 0)
      return __builtin_clzll(hi);
    return 64 + __builtin_clzll(U64(ux));
  }
}

template <cp::Int T>
[[gnu::always_inline]] constexpr I32 trailing_zeros(T x) {
  using Raw = std::remove_cv_t<T>;
  using U   = cp::make_unsigned_t<Raw>;
  U ux = U(x);
  if (ux == 0)
    return sizeof(Raw) * 8;
  if constexpr (sizeof(Raw) <= 4)
    return __builtin_ctz(U32(ux));
  else if constexpr (sizeof(Raw) <= 8)
    return __builtin_ctzll(U64(ux));
  else {
    const U64 lo = U64(ux);
    if (lo != 0)
      return __builtin_ctzll(lo);
    return 64 + __builtin_ctzll(U64(ux >> 64));
  }
}

template <cp::detail::OwnBitOps T>
[[gnu::always_inline]] constexpr I32 bit_width(T x) {
  return I32(sizeof(std::remove_cv_t<T>) * 8) - leading_zeros(x);
}

// Shifts run on the unsigned representation: `T(1) << 63` on a signed T is UB.
template <cp::detail::OwnBitOps T>
[[gnu::always_inline]] constexpr T bit_floor(T x) {
  using U = cp::make_unsigned_t<std::remove_cv_t<T>>;
  my_assert(x >= T(0) && "bit_floor(): negative input.");
  const U ux = U(x);
  if (ux == 0)
    return 0;
  return as<T>(U(U(1) << (bit_width(ux) - 1)));
}

template <cp::detail::OwnBitOps T>
[[gnu::always_inline]] constexpr T bit_ceil(T x) {
  using U = cp::make_unsigned_t<std::remove_cv_t<T>>;
  my_assert(x >= T(0) && "bit_ceil(): negative input.");
  const U ux = U(x);
  if (ux <= 1)
    return 1;
  return as<T>(U(U(1) << bit_width(U(ux - 1))));
}

template <cp::Int T>
[[gnu::always_inline]] constexpr I32 parity_sign(T x) { return (popcount(x) & 1) ? -1 : 1; }

template <cp::Int T>
constexpr T kth_bit(I32 k) {
  using U = cp::make_unsigned_t<std::remove_cv_t<T>>;
  my_assert(k >= 0 && k < I32(sizeof(T) * 8) && "kth_bit(): shift out of range.");
  return as<T>(U(U(1) << k));
}

template <cp::Int T>
constexpr bool has_kth_bit(T x, I32 k) {
  using U = cp::make_unsigned_t<std::remove_cv_t<T>>;
  my_assert(k >= 0 && k < I32(sizeof(T) * 8) && "has_kth_bit(): shift out of range.");
  return ((U(x) >> k) & U(1)) != U(0);
}

/// @brief Iterate over set bits in a mask, yielding their 0-based indices.
template <cp::Int T>
struct bit_range {
  T mask;

  struct iterator {
    T current;

    iterator(T mask) : current(mask) {}

    I32 operator*() const { return trailing_zeros(current); }

    iterator& operator++() {
      current &= current - 1;
      return *this;
    }

    bool operator!=(const iterator&) const { return current != 0; }
  };

  bit_range(T mask) : mask(mask) {}

  iterator begin() const { return iterator(mask); }
  iterator end() const { return iterator(0); }
};

template <cp::Int T>
struct subset_range {
  T mask;

  struct iterator {
    T    subset, original;
    bool finished;

    iterator(T mask) : subset(mask), original(mask), finished(false) {}

    T operator*() const { return original ^ subset; }

    iterator& operator++() {
      if (subset == 0)
        finished = true;
      else
        subset = (subset - 1) & original;
      return *this;
    }

    bool operator!=(const iterator&) const { return !finished; }
  };

  subset_range(T mask) : mask(mask) {}

  iterator begin() const { return iterator(mask); }
  iterator end() const { return iterator(0); }
};
