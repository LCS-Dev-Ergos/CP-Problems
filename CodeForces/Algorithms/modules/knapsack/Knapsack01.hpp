#ifndef CP_MODULES_KNAPSACK_KNAPSACK_01_HPP
#define CP_MODULES_KNAPSACK_KNAPSACK_01_HPP

#include "_Common.hpp"

/**
 * @file Knapsack01.hpp
 * @brief Adaptive 0/1 knapsack by weight, value, or meet in the middle.
 *
 * @details Weights and values must be non-negative. The automatic dispatcher
 * estimates the three standard costs and chooses the cheapest applicable one.
 */
enum class Knapsack01Strategy { Auto, MeetInMiddle, Weight, Value };

namespace knapsack_01_detail {

inline void check(const VecI32& weight, const VecI64& value, I32 cap) {
#ifdef LOCAL
  my_assert(isz(weight) == isz(value));
  my_assert(cap >= 0);
  for (const I32 w : weight) my_assert(w >= 0);
  for (const I64 v : value) my_assert(v >= 0);
#else
  (void)weight;
  (void)value;
  (void)cap;
#endif
}

[[nodiscard]] inline auto states(
    const VecI32& weight, const VecI64& value, I32 l, I32 r, I32 cap) -> Vec<Pair<I32, I64>> {
  const I32 n = r - l;
  Vec<Pair<I32, I64>> dp(1 << n);
  FOR(mask, 1, 1 << n) {
    const I32 bit = std::countr_zero(U32(mask));
    const I32 prev = mask & (mask - 1);
    dp[mask] = {dp[prev].first + weight[l + bit], dp[prev].second + value[l + bit]};
  }
  std::sort(all(dp));
  Vec<Pair<I32, I64>> kept;
  for (const auto [w, v] : dp) {
    if (w > cap) break;
    if (!kept.empty() && kept.back().first == w) {
      chmax(kept.back().second, v);
    } else if (kept.empty() || kept.back().second < v) {
      kept.push_back({w, v});
    }
  }
  FOR(i, 1, isz(kept)) chmax(kept[i].second, kept[i - 1].second);
  return kept;
}

}  // namespace knapsack_01_detail

/// @brief O(N * cap) DP indexed by total weight.
[[nodiscard]] inline auto knapsack_01_by_weight(
    const VecI32& weight, const VecI64& value, I32 cap) -> I64 {
  knapsack_01_detail::check(weight, value, cap);
  I64 sum_weight = 0;
  for (const I32 w : weight) sum_weight += w;
  const I32 lim = I32(std::min<I64>(cap, sum_weight));
  VecI64 dp(lim + 1, neg_infinity<I64>);
  dp[0] = 0;
  FOR(i, isz(weight)) {
    if (weight[i] > lim) continue;
    FOR_R(w, lim - weight[i] + 1) {
      if (dp[w] == neg_infinity<I64>) continue;
      chmax(dp[w + weight[i]], dp[w] + value[i]);
    }
  }
  return *std::max_element(all(dp));
}

/// @brief O(N * sum(value)) DP storing the minimum weight for every value.
[[nodiscard]] inline auto knapsack_01_by_value(
    const VecI32& weight, const VecI64& value, I32 cap) -> I64 {
  knapsack_01_detail::check(weight, value, cap);
  I64 sum_value = 0;
  for (const I64 v : value) sum_value += v;
  my_assert(sum_value <= std::numeric_limits<I32>::max() - 1);
  const I32 total = I32(sum_value);
  VecI64 dp(total + 1, infinity<I64>);
  dp[0] = 0;
  I32 reached = 0;
  FOR(i, isz(weight)) {
    const I32 v = I32(value[i]);
    FOR_R(cur, reached + 1) {
      if (dp[cur] == infinity<I64>) continue;
      chmin(dp[cur + v], dp[cur] + weight[i]);
    }
    reached += v;
  }
  FOR_R(v, total + 1) {
    if (dp[v] <= cap) return v;
  }
  return 0;
}

/// @brief O(2^(N/2) log 2^(N/2)) meet-in-the-middle solver.
[[nodiscard]] inline auto knapsack_01_meet_in_middle(
    const VecI32& weight, const VecI64& value, I32 cap) -> I64 {
  knapsack_01_detail::check(weight, value, cap);
  const I32 n = isz(weight);
  my_assert(n <= 46);
  const I32 mid = n / 2;
  const auto left = knapsack_01_detail::states(weight, value, 0, mid, cap);
  const auto right = knapsack_01_detail::states(weight, value, mid, n, cap);
  I64 ans = 0;
  for (const auto [w, v] : left) {
    const auto it = std::upper_bound(
        all(right), Pair<I32, I64>{cap - w, infinity<I64>});
    if (it != right.begin()) chmax(ans, v + std::prev(it)->second);
  }
  return ans;
}

/// @brief Chooses or forces one exact 0/1 knapsack strategy.
[[nodiscard]] inline auto knapsack_01(
    const VecI32& weight,
    const VecI64& value,
    I32 cap,
    Knapsack01Strategy strategy = Knapsack01Strategy::Auto) -> I64 {
  knapsack_01_detail::check(weight, value, cap);
  if (strategy == Knapsack01Strategy::Weight) {
    return knapsack_01_by_weight(weight, value, cap);
  }
  if (strategy == Knapsack01Strategy::Value) {
    return knapsack_01_by_value(weight, value, cap);
  }
  if (strategy == Knapsack01Strategy::MeetInMiddle) {
    return knapsack_01_meet_in_middle(weight, value, cap);
  }

  const I32 n = isz(weight);
  I64 sum_weight = 0;
  I64 sum_value = 0;
  for (const I32 w : weight) sum_weight += w;
  for (const I64 v : value) sum_value += v;
  const long double cost_weight = (static_cast<long double>(n) + 1) *
                                  (std::min<I64>(cap, sum_weight) + 1);
  const long double cost_value = sum_value <= std::numeric_limits<I32>::max() - 1
                                     ? (static_cast<long double>(n) + 1) * (sum_value + 1)
                                     : infinity<long double>;
  const long double cost_mitm = n <= 46 ? std::exp2((n + 1) / 2.0L) : infinity<long double>;
  if (cost_mitm <= cost_weight && cost_mitm <= cost_value) {
    return knapsack_01_meet_in_middle(weight, value, cap);
  }
  if (cost_weight <= cost_value) return knapsack_01_by_weight(weight, value, cap);
  return knapsack_01_by_value(weight, value, cap);
}

#endif
