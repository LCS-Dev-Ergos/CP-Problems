#ifndef CP_MODULES_DATA_STRUCTURES_SPARSE_DELTA_SWEEP_HPP
#define CP_MODULES_DATA_STRUCTURES_SPARSE_DELTA_SWEEP_HPP

#include "../_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/// @brief Sparse difference-array sweep with O(K log K) per round, where K is touched positions.
struct SparseDeltaSweep {
  I32 max_position = -1;
  VecI32 delta;
  VecI32 used_positions;
  VecU32 stamp;
  U32 round_id = 1;

  SparseDeltaSweep() = default;
  explicit SparseDeltaSweep(const I32 max_pos) { init(max_pos); }

  void init(const I32 max_pos) {
    my_assert(max_pos >= 0);
    max_position = max_pos;
    delta.assign((max_position + 1), 0);
    stamp.assign((max_position + 1), 0);
    used_positions.clear();
    used_positions.reserve((max_position + 1));
    round_id = 1;
  }

  /// @brief Starts a fresh logical round without clearing full buffers.
  inline void begin_round() {
    ++round_id;
    used_positions.clear();
    if (round_id == 0) {
      std::fill(stamp.begin(), stamp.end(), 0);
      round_id = 1;
    }
  }

  /// @brief Adds @p delta_value to the difference-array event at @p position.
  inline void add(const I32 position, const I32 delta_value) {
    my_assert(0 <= position && position <= max_position);
    if (stamp[position] != round_id) {
      stamp[position] = round_id;
      delta[position] = 0;
      used_positions.push_back(position);
    }
    delta[position] += delta_value;
  }

  /// @brief Computes sum_{j=0..right_inclusive} cover(j)^2 from the current round events.
  [[nodiscard]] auto squared_coverage_sum(const I32 right_inclusive) -> I128 {
    my_assert(0 <= right_inclusive && right_inclusive <= max_position);
    if (used_positions.empty()) return 0;

    std::sort(used_positions.begin(), used_positions.end());

    I64 active = 0;
    I32 last = 0;
    I128 result = 0;

    for (const I32 pos : used_positions) {
      if (pos > last) {
        const I64 len = as<I64>(pos - last);
        result += as<I128>(len) * active * active;
      }
      active += delta[pos];
      last = pos;
    }

    const I64 right_exclusive = as<I64>(right_inclusive) + 1;
    if (as<I64>(last) <= right_exclusive) {
      const I64 len = right_exclusive - as<I64>(last);
      result += as<I128>(len) * active * active;
    }
    return result;
  }
};

#endif
