#include "modules/data_structures/offline/OfflineDynamicQueries.hpp"
#include "modules/data_structures/union_find/DSURollback.hpp"

#include <cassert>

int main() {
  using Edge = PairI32;
  OfflineDynamicQueries<Edge> queries(7);
  queries.add(0, {0, 1});
  queries.add(1, {1, 2});
  queries.add(2, {3, 4});
  queries.remove(3, {1, 2});
  queries.add_interval(4, 7, {2, 3});
  queries.remove(6, {0, 1});

  DSURollback dsu(5);
  VecI32 components(7);
  VecBool connected(7);
  queries.run(
      [&](Edge edge) { dsu.unite(edge.first, edge.second); },
      [&] { return dsu.snapshot(); },
      [&](Size snap) { dsu.rollback(snap); },
      [&](I32 t) {
        components[t] = dsu.num_components();
        connected[t] = dsu.connected(0, 4);
      });

  assert(components == VecI32({4, 3, 2, 3, 2, 2, 3}));
  assert(connected == VecBool({false, false, false, false, false, false, false}));

  OfflineDynamicQueries<I32> empty(0);
  I32 calls = 0;
  empty.run([](I32) {}, [] { return 0; }, [](I32) {}, [&](I32) { ++calls; });
  assert(calls == 0);
  return 0;
}
