#ifndef CP_MODULES_DATA_STRUCTURES_PARALLEL_BINARY_SEARCH_HPP
#define CP_MODULES_DATA_STRUCTURES_PARALLEL_BINARY_SEARCH_HPP

#include "../_Common.hpp"

/**
 * @brief Binary searches every query at once over a shared timeline of updates.
 *
 * @details Many offline problems ask, for each of @c Q queries, the first
 * moment along a fixed sequence of @c T updates at which some monotone
 * predicate starts to hold. Answering them one at a time costs @c Q separate
 * replays of the timeline. Instead, all queries advance one binary-search step
 * per round: within a round the probe times are sorted, the structure is rebuilt
 * once and rolled forward monotonically, and every query is tested exactly once.
 * That is O(log T) rounds of one full replay each.
 *
 * The caller supplies three callbacks:
 *
 * - @c init() resets the structure to the state before any update;
 * - @c update(t) applies the update with index @c t, for @c t in [0, T);
 * - @c check(q) reports whether query @c q is satisfied by the current state.
 *
 * @c check has to be a pure observation, and replaying @c init() followed by
 * @c update(0..t-1) must always reproduce the same state, since that is exactly
 * what each round does.
 *
 * @param query_count Number of queries.
 * @param ok The end of the search range where the predicate holds. With the
 * usual "the predicate turns on and stays on" shape this is the number of
 * updates @c T, and @p ng is 0.
 * @param ng The end of the search range where it does not hold.
 *
 * Neither end of the range is ever probed: @p ok is taken on trust as
 * satisfying the predicate and @p ng as not satisfying it, so every answer
 * lands strictly between them, or on @p ok itself when no probe succeeded.
 *
 * @return For each query, the number of updates on the @p ok side of the
 * boundary. A query whose predicate never flips keeps its initial @p ok value.
 *
 * @code
 * // First edge insertion that connects s and t, for each query pair.
 * const VecI32 answer = parallel_binary_search(
 *     isz(queries), isz(edges), 0,
 *     [&] { dsu.init(n); },
 *     [&](I32 t) { dsu.unite(edges[t].first, edges[t].second); },
 *     [&](I32 q) { return dsu.connected(queries[q].first, queries[q].second); });
 * @endcode
 */
template <typename FInit, typename FUpdate, typename FCheck>
[[nodiscard]] auto parallel_binary_search(
    I32 query_count,
    I32 ok,
    I32 ng,
    FInit init,
    FUpdate update,
    FCheck check) -> VecI32 {
  my_assert(query_count >= 0);
  my_assert(ok >= 0 && ng >= 0);

  const I32 horizon = std::max(ok, ng);
  VecI32 side_ok(query_count, ok);
  VecI32 side_ng(query_count, ng);
  if (query_count == 0) return side_ok;

  VecI32 probe(query_count);
  VecI32 start(horizon + 1);
  VecI32 cursor(horizon + 1);
  VecI32 order;

  while (true) {
    // A query is still live while its two ends are more than one step apart.
    FOR(q, query_count) {
      const I32 lo = std::min(side_ok[q], side_ng[q]);
      const I32 hi = std::max(side_ok[q], side_ng[q]);
      probe[q] = (hi - lo > 1 ? (lo + hi) / 2 : -1);
    }

    // Counting sort of the live queries by probe time, so the replay below can
    // walk the timeline forward without ever restarting it.
    std::fill(all(start), 0);
    FOR(q, query_count) {
      if (probe[q] >= 0) ++start[probe[q] + 1];
    }
    FOR(t, horizon) start[t + 1] += start[t];
    const I32 total = start[horizon];
    if (total == 0) break;

    cursor = start;
    order.assign(total, 0);
    FOR(q, query_count) {
      if (probe[q] >= 0) order[cursor[probe[q]]++] = q;
    }

    init();
    I32 now = 0;
    for (const I32 q : order) {
      while (now < probe[q]) update(now++);
      if (check(q)) side_ok[q] = now;
      else side_ng[q] = now;
    }
  }

  return side_ok;
}

#endif
