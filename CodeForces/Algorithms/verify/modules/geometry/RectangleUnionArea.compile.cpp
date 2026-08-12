#include "modules/geometry/RectangleUnionArea.hpp"

int main() {
  RectangleUnionArea<> area;
  area.add_rect(0, 0, 3, 2);
  area.add_rect(1, 1, 4, 3);
  return area.area() == 10 ? 0 : 1;
}
