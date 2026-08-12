#ifndef CP_MODULES_DATA_STRUCTURES_LI_CHAO_TREE_HPP
#define CP_MODULES_DATA_STRUCTURES_LI_CHAO_TREE_HPP

#include "../_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/**
 * @brief Dynamic Li Chao tree for minimum line queries.
 *
 * @details Domain is integer x in [x_left, x_right].
 * Supports arbitrary line insertion order and point minimum queries.
 * Complexity: O(log domain) per add/query.
 */
struct LiChaoTree {
  struct Line {
    I64 m = 0;
    I64 b = infinity<I64>;
    I64 eval(I64 x) const { return as<I64>(as<I128>(m) * x + b); }
  };

  struct Node {
    Line ln;
    I32 left = -1;
    I32 right = -1;
  };

  I64 x_left = 0;
  I64 x_right = 0;
  Vec<Node> nodes;
  I32 root = -1;

  LiChaoTree() = default;

  LiChaoTree(I64 l, I64 r) { init(l, r); }

  void init(I64 l, I64 r) {
    x_left = l;
    x_right = r;
    nodes.clear();
    root = new_node();
  }

  void clear() { nodes.clear(); root = -1; }

  /// @brief Adds line y = m*x + b.
  void add_line(I64 m, I64 b) {
    if (root == -1) root = new_node();
    add_line_internal(root, x_left, x_right, Line{m, b});
  }

  /// @brief Minimum value at x.
  I64 query(I64 x) const {
    if (root == -1) return infinity<I64>;
    return query_internal(root, x_left, x_right, x);
  }

private:
  I32 new_node() {
    nodes.push_back(Node{});
    return isz(nodes) - 1;
  }

  void add_line_internal(I32 v, I64 l, I64 r, Line nw) {
    I64 mid = l + (r - l) / 2;
    bool left_better = nw.eval(l) < nodes[v].ln.eval(l);
    bool mid_better = nw.eval(mid) < nodes[v].ln.eval(mid);

    if (mid_better) std::swap(nodes[v].ln, nw);
    if (l == r) return;

    if (left_better != mid_better) {
      if (nodes[v].left == -1) {
        nodes[v].left = new_node();
      }
      add_line_internal(nodes[v].left, l, mid, nw);
    } else {
      if (nodes[v].right == -1) {
        nodes[v].right = new_node();
      }
      add_line_internal(nodes[v].right, mid + 1, r, nw);
    }
  }

  I64 query_internal(I32 v, I64 l, I64 r, I64 x) const {
    I64 res = nodes[v].ln.eval(x);
    if (l == r) return res;
    I64 mid = l + (r - l) / 2;
    if (x <= mid && nodes[v].left != -1) {
      return std::min(res, query_internal(nodes[v].left, l, mid, x));
    }
    if (x > mid && nodes[v].right != -1) {
      return std::min(res, query_internal(nodes[v].right, mid + 1, r, x));
    }
    return res;
  }
};

#endif
