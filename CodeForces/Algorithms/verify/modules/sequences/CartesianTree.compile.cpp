#include "modules/sequences/CartesianTree.hpp"

int main() {
  const CartesianTree<I32> tree(VecI32{3, 1, 4, 1, 5});
  return tree.root == 1 && tree.range(tree.root) == Pair<I32, I32>{0, 5} ? 0 : 1;
}
