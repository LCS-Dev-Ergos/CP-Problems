#include "modules/data_structures/bit/FastSet.hpp"

int main() {
  FastSet st(10);
  st.insert(3);
  return st.next(0) == 3 ? 0 : 1;
}
