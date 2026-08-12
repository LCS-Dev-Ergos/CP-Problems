#include "modules/data_structures/offline/OfflineDynamicQueries.hpp"

int main() {
  OfflineDynamicQueries<I32> queries(3);
  queries.add(0, 7);
  queries.remove(2, 7);
  I32 active = 0;
  VecI32 answer(3);
  queries.run(
      [&](I32) { ++active; }, [&] { return active; },
      [&](I32 snap) { active = snap; }, [&](I32 t) { answer[t] = active; });
  return answer == VecI32({1, 1, 0}) ? 0 : 1;
}
