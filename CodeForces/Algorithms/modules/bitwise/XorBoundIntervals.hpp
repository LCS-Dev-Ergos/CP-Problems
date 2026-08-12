#ifndef CP_MODULES_BITWISE_XOR_BOUND_INTERVALS_HPP
#define CP_MODULES_BITWISE_XOR_BOUND_INTERVALS_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/// @brief Precomputes interval decomposition for sets {j in [0, m] : (j xor x) <= m}.
struct XorBoundIntervals {
  I32 m = 0;
  I32 limit = 0;
  I32 top_bit = 0;
  VecI32 overlap;
  VecI32 offset;
  VecPairI32 intervals;

  XorBoundIntervals() = default;

  XorBoundIntervals(const I32 bound, const I32 xor_limit) { build(bound, xor_limit); }

  void build(const I32 bound, const I32 xor_limit) {
    my_assert(bound >= 0);
    my_assert(xor_limit > 0 && (xor_limit & (xor_limit - 1)) == 0);

    m = bound;
    limit = xor_limit;
    top_bit = ::bit_width(as<U32>(std::max(1, m))) - 1;

    overlap.assign(limit, 0);
    offset.assign((limit + 1), 0);
    intervals.clear();
    intervals.reserve(limit * 10);

    VecPairI32 tmp;
    tmp.reserve(32);

    FOR(x, limit) {
      offset[x] = isz(intervals);
      collect_for_x(x, tmp);

      I32 covered = 0;
      for (const auto [l, r] : tmp) {
        covered += (r - l + 1);
        intervals.push_back({l, r});
      }
      overlap[x] = covered;
    }
    offset[limit] = isz(intervals);
  }

  [[nodiscard]] inline auto overlap_count(const I32 x) const -> I32 { return overlap[x]; }
  [[nodiscard]] inline auto interval_begin(const I32 x) const -> I32 { return offset[x]; }
  [[nodiscard]] inline auto interval_end(const I32 x) const -> I32 { return offset[x + 1]; }
  [[nodiscard]] inline auto interval_at(const I32 idx) const -> PairI32 { return intervals[idx]; }

private:
  void collect_for_x(const I32 x, VecPairI32& out) const {
    out.clear();
    if ((x >> (top_bit + 1)) != 0) return;

    const auto dfs = [&](
        const auto& self,
        const I32 bit, const bool tight_j,
        const bool tight_y, const I32 prefix
      ) -> void {
      if (bit < 0) {
        out.push_back({prefix, prefix});
        return;
      }

      if (!tight_j && !tight_y) {
        const I64 len = I64(1) << (bit + 1);
        const I64 l = as<I64>(prefix) << (bit + 1);
        out.push_back({as<I32>(l), as<I32>(l + len - 1)});
        return;
      }

      const I32 mb = (m >> bit) & 1;
      const I32 xb = (x >> bit) & 1;
      const I32 max_jb = tight_j ? mb : 1;

      for (I32 jb = 0; jb <= max_jb; ++jb) {
        const I32 yb = jb ^ xb;
        if (tight_y && yb > mb) continue;

        const bool next_tight_j = tight_j && (jb == mb);
        const bool next_tight_y = tight_y && (yb == mb);
        self(self, bit - 1, next_tight_j, next_tight_y, (prefix << 1) | jb);
      }
    };

    dfs(dfs, top_bit, true, true, 0);
    if (out.empty()) return;

    VecPairI32 merged;
    merged.reserve(out.size());
    for (const auto [l, r] : out) {
      if (!merged.empty() && l <= merged.back().second + 1) {
        merged.back().second = std::max(merged.back().second, r);
      } else {
        merged.push_back({l, r});
      }
    }
    out.swap(merged);
  }
};

#endif
