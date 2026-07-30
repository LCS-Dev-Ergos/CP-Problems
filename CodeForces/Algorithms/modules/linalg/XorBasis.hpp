#ifndef CP_MODULES_LINALG_XOR_BASIS_HPP
#define CP_MODULES_LINALG_XOR_BASIS_HPP

#include "_Common.hpp"

/**
 * @brief Linear basis of a GF(2) vector space over fixed-width integers.
 *
 * @details The basis is kept in reduced row echelon form: @c dat[i] is either
 * zero or the unique basis vector whose highest set bit is @c i, and no other
 * basis vector has bit @c i set. That normal form makes the span sort in the
 * same order as the pivot coefficients, which is what turns @c kth() and
 * @c count_less() into a single walk over the pivots. The price is an
 * O(WIDTH) insert instead of the usual early-exit one.
 */
template <typename UInt = U64>
struct XorBasis {
  static constexpr I32 WIDTH = I32(sizeof(UInt)) * 8;

  I32 dim = 0;
  Array<UInt, sizeof(UInt) * 8> dat{};

  XorBasis() = default;

  /// @brief Number of independent vectors stored.
  [[nodiscard]] auto size() const -> I32 { return dim; }

  /// @brief Whether the span is trivial.
  [[nodiscard]] auto empty() const -> bool { return dim == 0; }

  /// @brief Adds @p v to the basis; returns false when it was already spanned.
  auto insert(UInt v) -> bool {
    FOR_R(i, WIDTH) chmin(v, UInt(v ^ dat[i]));
    if (v == 0) return false;
    FOR(i, WIDTH) {
      if (dat[i] != 0) chmin(dat[i], UInt(dat[i] ^ v));
    }
    dat[bit_width(v) - 1] = v;
    ++dim;
    return true;
  }

  /// @brief Whether @p v lies in the span.
  [[nodiscard]] auto contains(UInt v) const -> bool {
    FOR_R(i, WIDTH) chmin(v, UInt(v ^ dat[i]));
    return v == 0;
  }

  /// @brief Largest value of @p base xored with some element of the span.
  [[nodiscard]] auto max_xor(UInt base = 0) const -> UInt {
    FOR_R(i, WIDTH) chmax(base, UInt(base ^ dat[i]));
    return base;
  }

  /// @brief Smallest value of @p base xored with some element of the span.
  [[nodiscard]] auto min_xor(UInt base = 0) const -> UInt {
    FOR_R(i, WIDTH) chmin(base, UInt(base ^ dat[i]));
    return base;
  }

  /// @brief The @p k -th smallest element of the span, 0-indexed.
  [[nodiscard]] auto kth(U64 k) const -> UInt {
    my_assert(dim >= 64 || k < (U64(1) << dim));
    UInt res = 0;
    I32 d = 0;
    FOR(i, WIDTH) {
      if (dat[i] == 0) continue;
      if ((k >> d) & 1) res ^= dat[i];
      ++d;
    }
    return res;
  }

  /// @brief Number of elements of the span strictly below @p x.
  /// @note Requires dim <= 64, otherwise the count does not fit a U64.
  [[nodiscard]] auto count_less(UInt x) const -> U64 {
    my_assert(dim <= 64);
    U64 res = 0;
    UInt now = 0;
    I32 d = dim;
    FOR_R(i, WIDTH) {
      if (dat[i] == 0) continue;
      --d;
      // Every element with this pivot cleared is below every element with it
      // set, so one whole half falls under x as soon as its minimum does.
      if (UInt(now ^ dat[i]) < x) {
        res += U64(1) << d;
        now ^= dat[i];
      }
    }
    if (now < x) ++res;
    return res;
  }

  /// @brief Basis vectors in increasing order of their highest set bit.
  [[nodiscard]] auto to_vec() const -> Vec<UInt> {
    Vec<UInt> res;
    res.reserve(dim);
    FOR(i, WIDTH) {
      if (dat[i] != 0) res.push_back(dat[i]);
    }
    return res;
  }

  /// @brief Basis of the sum of two spans.
  static auto merge(XorBasis a, const XorBasis& b) -> XorBasis {
    FOR(i, WIDTH) {
      if (b.dat[i] != 0) a.insert(b.dat[i]);
    }
    return a;
  }
};

#endif
