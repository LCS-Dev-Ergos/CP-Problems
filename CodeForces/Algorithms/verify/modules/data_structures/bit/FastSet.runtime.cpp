#include "modules/data_structures/bit/FastSet.hpp"

#include <cassert>

int main() {
  FastSet st(20);

  assert(st.empty());
  assert(st.size() == 20);
  assert(st.count() == 0);
  assert(st.next(0) == 20);
  assert(st.prev(19) == -1);

  assert(st.insert(5));
  assert(st.insert(7));
  assert(st.insert(12));
  assert(!st.insert(7));
  assert(st.count() == 3);
  assert(st.contains(5));
  assert(st[7]);
  assert(!st.contains(6));

  assert(st.next(0) == 5);
  assert(st.next(6) == 7);
  assert(st.next(7) == 7);
  assert(st.next(8) == 12);
  assert(st.next(13) == 20);

  assert(st.prev(19) == 12);
  assert(st.prev(12) == 12);
  assert(st.prev(11) == 7);
  assert(st.prev(6) == 5);
  assert(st.prev(4) == -1);

  assert(st.any(6, 8));
  assert(!st.any(8, 12));
  assert(st.any(8, 13));

  VecI32 vals;
  st.enumerate(0, 20, [&](I32 x) { vals.eb(x); });
  assert((vals == VecI32{5, 7, 12}));

  assert(st.erase(7));
  assert(!st.erase(7));
  assert(st.count() == 2);
  assert(st.next(6) == 12);
  assert(st.prev(11) == 5);

  FastSet even(16, [&](I32 i) { return i % 2 == 0; });
  assert(even.count() == 8);
  assert(even.next(5) == 6);
  assert(even.prev(5) == 4);

  even.clear();
  assert(even.empty());
  assert(even.next(0) == 16);
  return 0;
}
