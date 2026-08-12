#ifndef CP_MODULES_DATA_STRUCTURES_TREE_LINK_CUT_TREE_HPP
#define CP_MODULES_DATA_STRUCTURES_TREE_LINK_CUT_TREE_HPP

#include "../_Common.hpp"

/**
 * @brief Link-Cut Tree (splay-based) for dynamic forest path queries.
 * @details This implementation maintains path sums over node values.
 */
struct LinkCutTree {
  /// @brief Node in auxiliary splay trees.
  struct Node {
    I32 left, right, parent;
    I64 value, sum;
    bool reversed;

    Node(I64 v = 0) : left(-1), right(-1), parent(-1), value(v), sum(v), reversed(false) {}
  };

  Vec<Node> nodes;

  /// @brief Initializes forest with @p n isolated nodes.
  LinkCutTree(I32 n) : nodes(n) { FOR(i, n) nodes[i] = Node(0); }

  /// @brief Checks whether node is root of its auxiliary splay.
  bool is_root(I32 v) {
    return nodes[v].parent == -1 || (nodes[nodes[v].parent].left != v && nodes[nodes[v].parent].right != v);
  }

  /// @brief Pushes lazy reversal flag down the tree.
  void push(I32 v) {
    if (!nodes[v].reversed) return;
    std::swap(nodes[v].left, nodes[v].right);
    if (nodes[v].left != -1) nodes[nodes[v].left].reversed ^= 1;
    if (nodes[v].right != -1) nodes[nodes[v].right].reversed ^= 1;
    nodes[v].reversed = false;
  }

  /// @brief Recomputes aggregate for node @p v.
  void update(I32 v) {
    nodes[v].sum = nodes[v].value;
    if (nodes[v].left != -1) nodes[v].sum += nodes[nodes[v].left].sum;
    if (nodes[v].right != -1) nodes[v].sum += nodes[nodes[v].right].sum;
  }

  /// @brief Single rotation in splay tree.
  void rotate(I32 v) {
    I32 p = nodes[v].parent;
    I32 g = nodes[p].parent;

    if (!is_root(p)) {
      if (nodes[g].left == p) nodes[g].left = v;
      else nodes[g].right = v;
    }

    if (nodes[p].left == v) {
      nodes[p].left = nodes[v].right;
      if (nodes[v].right != -1) nodes[nodes[v].right].parent = p;
      nodes[v].right = p;
    } else {
      nodes[p].right = nodes[v].left;
      if (nodes[v].left != -1) nodes[nodes[v].left].parent = p;
      nodes[v].left = p;
    }

    nodes[v].parent = g;
    nodes[p].parent = v;
    update(p);
    update(v);
  }

  /// @brief Splays node to root of auxiliary tree.
  void splay(I32 v) {
    while (!is_root(v)) {
      I32 p = nodes[v].parent;
      if (!is_root(p)) push(nodes[p].parent);
      push(p);
      push(v);

      if (!is_root(p)) {
        if ((nodes[nodes[p].parent].left == p) == (nodes[p].left == v)) {
          rotate(p);
        } else {
          rotate(v);
        }
      }
      rotate(v);
    }
    push(v);
    update(v);
  }

  /// @brief Exposes preferred path root->v and returns last accessed node.
  I32 access(I32 v) {
    I32 last = -1;
    for (I32 w = v; w != -1; w = nodes[w].parent) {
      splay(w);
      I32 old_right = nodes[w].right;
      if (old_right != -1) nodes[old_right].parent = -1;
      nodes[w].right = last;
      if (last != -1) nodes[last].parent = w;
      update(w);
      last = w;
    }
    splay(v);
    return last;
  }

  /// @brief Makes node @p v the represented-tree root.
  void make_root(I32 v) { access(v); nodes[v].reversed ^= 1; }

  /// @brief Links two trees by edge (u, v).
  void link(I32 u, I32 v) { make_root(u); nodes[u].parent = v; }

  /// @brief Cuts edge (u, v) assuming it exists.
  void cut(I32 u, I32 v) {
    make_root(u);
    access(v);
    nodes[v].left = -1;
    nodes[u].parent = -1;
    update(v);
  }

  /// @brief Returns sum of node values on path u-v.
  I64 path_sum(I32 u, I32 v) {
    make_root(u);
    access(v);
    return nodes[v].sum;
  }
};

#endif
