#include "modules/graph/RangeToRangeGraph.hpp"
#include "modules/graph/DijkstraSparse.hpp"

int main() {
  // Vertex 0 reaches [1, 5) at cost 3, and [1, 3) reaches [3, 5) at cost 4.
  RangeToRangeGraph<I64> builder(5);
  builder.add_to_range(0, 1, 5, 3);
  builder.add_range_to_range(1, 3, 3, 5, 4);
  const Graph<I64> g = builder.build();
  const VecI64 dist = dijkstra_sparse(g, 0).dist;
  const bool ok = (dist[0] == 0) && (dist[1] == 3) && (dist[4] == 3);
  return ok ? 0 : 1;
}
