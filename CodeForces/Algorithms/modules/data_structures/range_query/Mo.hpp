#ifndef CP_MODULES_DATA_STRUCTURES_MO_HPP
#define CP_MODULES_DATA_STRUCTURES_MO_HPP

#include "../_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/**
 * @brief Mo's algorithm for offline range queries.
 * @details Processes queries in an order that minimizes pointer movement.
 * Uses alternating direction on even/odd blocks for better cache behavior.
 * Queries use half-open intervals [l, r).
 */
struct Mo {
  I32 block_sz;

  Mo(I32 n) : block_sz(std::max(1, as<I32>(std::sqrt(n)))) {}

  /// @brief Processes range queries offline.
  template <typename Add, typename Remove, typename Answer>
  void process(const Vec<PairI32>& queries, Add add, Remove remove, Answer answer) {
    I32 q = sz(queries);
    VecI32 order(q);
    std::iota(all(order), 0);
    std::sort(all(order), [&](I32 a, I32 b) {
      I32 ba = queries[a].first / block_sz, bb = queries[b].first / block_sz;
      if (ba != bb) return ba < bb;
      return (ba & 1) ? queries[a].second > queries[b].second : queries[a].second < queries[b].second;
    });

    I32 cur_l = 0, cur_r = 0;
    for (I32 idx : order) {
      auto [ql, qr] = queries[idx];
      while (cur_r < qr) add(cur_r++);
      while (cur_l > ql) add(--cur_l);
      while (cur_r > qr) remove(--cur_r);
      while (cur_l < ql) remove(cur_l++);
      answer(idx);
    }
  }
};

#endif
