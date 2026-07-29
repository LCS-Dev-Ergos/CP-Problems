#ifndef CP_MODULES_STRINGS_PALINDROMIC_TREE_HPP
#define CP_MODULES_STRINGS_PALINDROMIC_TREE_HPP

#include "_Common.hpp"

#include <cassert>

/**
 * @brief Eertree / palindromic tree for one string.
 *
 * @details Supports online extension, longest palindromic suffix tracking,
 * suffix-link traversal, and post-build occurrence propagation.
 */
struct PalindromicTree {
  struct Node {
    UnorderedMap<char, I32> next;
    I32 link = 0;
    I32 len  = 0;
    I32 diff = 0;
    I32 series_link = 0;
    I32 occ = 0;
    I32 first_pos = -1;
  };

  String text;
  Vec<Node> nodes;
  VecI32 path;
  I32 last = 1;
  bool occ_ready = false;

  /// @brief Creates an empty palindromic tree with two standard roots.
  PalindromicTree() { reset(); }

  /// @brief Builds the tree for the given string.
  explicit PalindromicTree(const String& s) {
    reset();
    append(s);
  }

  /// @brief Clears the structure and restores the two roots.
  void reset() {
    text.clear();
    nodes.clear();
    path.clear();

    Node odd_root;
    odd_root.len  = -1;
    odd_root.link = 0;
    odd_root.series_link = 0;

    Node even_root;
    even_root.len  = 0;
    even_root.link = 0;
    even_root.series_link = 0;

    nodes.eb(odd_root);
    nodes.eb(even_root);
    last = 1;
    occ_ready = false;
    path.eb(last);
  }

  /// @brief Rebuilds the tree for the given string.
  void build(const String& s) {
    reset();
    append(s);
  }

  /// @brief Appends a full string one character at a time.
  void append(const String& s) {
    for (char c : s) extend(c);
  }

  /// @brief Number of palindromic substrings as distinct nodes.
  I32 distinct_palindromes() const {
    return sz(nodes) - 2;
  }

  /// @brief Total number of stored nodes including the two roots.
  I32 size() const {
    return sz(nodes);
  }

  /// @brief Returns whether no real palindrome node has been created yet.
  bool empty() const {
    return distinct_palindromes() == 0;
  }

  /// @brief Longest palindromic suffix of the whole built string.
  I32 longest_suffix_length() const {
    return nodes[last].len;
  }

  /// @brief Longest palindromic suffix of prefix [0, pref_len).
  I32 longest_suffix_length(I32 pref_len) const {
    assert(0 <= pref_len && pref_len < sz(path));
    return nodes[path[pref_len]].len;
  }

  /// @brief Node id of longest palindromic suffix of prefix [0, pref_len).
  I32 suffix_node(I32 pref_len) const {
    assert(0 <= pref_len && pref_len < sz(path));
    return path[pref_len];
  }

  /// @brief Returns one occurrence interval [l, r) for node v.
  PairI32 interval(I32 v) const {
    assert(0 <= v && v < sz(nodes));
    I32 r = nodes[v].first_pos + 1;
    return {r - nodes[v].len, r};
  }

  /// @brief Propagates occurrence counts through suffix links.
  void build_occurrences() {
    VecI32 ord(sz(nodes));
    std::iota(ord.begin(), ord.end(), 0);
    std::ranges::sort(ord, [&](I32 a, I32 b) { return nodes[a].len > nodes[b].len; });
    for (I32 v : ord) {
      if (v <= 1) continue;
      nodes[nodes[v].link].occ += nodes[v].occ;
    }
    occ_ready = true;
  }

  /// @brief Occurrence count of node v in the built string.
  I32 occurrences(I32 v) {
    assert(0 <= v && v < sz(nodes));
    if (!occ_ready) build_occurrences();
    return nodes[v].occ;
  }

  /// @brief Number of palindromic suffixes of prefix [0, pref_len).
  I32 suffix_count(I32 pref_len) const {
    assert(0 <= pref_len && pref_len < sz(path));
    I32 cnt = 0;
    for (I32 v = path[pref_len]; v > 1; v = nodes[v].link) ++cnt;
    return cnt;
  }

  /// @brief Longest palindromic suffix of substring [l, r).
  I32 max_suffix_length(I32 l, I32 r) const {
    assert(0 <= l && l <= r && r < sz(path));
    I32 need = r - l;
    for (I32 v = path[r]; v > 1; v = nodes[v].link) {
      if (nodes[v].len <= need) return nodes[v].len;
    }
    return 0;
  }

  /// @brief Appends one character and returns the node id of the new suffix palindrome.
  I32 extend(char c) {
    occ_ready = false;
    I32 pos = sz(text);
    text += c;

    I32 p = get_link(last, pos);
    auto it = nodes[p].next.find(c);
    if (it != nodes[p].next.end()) {
      last = it->second;
      nodes[last].occ++;
      path.eb(last);
      return last;
    }

    I32 v = sz(nodes);
    nodes.eb();
    nodes[v].len = nodes[p].len + 2;
    nodes[v].first_pos = pos;
    nodes[v].occ = 1;
    nodes[p].next[c] = v;

    if (nodes[v].len == 1) {
      nodes[v].link = 1;
    } else {
      I32 q = get_link(nodes[p].link, pos);
      nodes[v].link = nodes[q].next[c];
    }

    nodes[v].diff = nodes[v].len - nodes[nodes[v].link].len;
    if (nodes[v].diff == nodes[nodes[v].link].diff) {
      nodes[v].series_link = nodes[nodes[v].link].series_link;
    } else {
      nodes[v].series_link = nodes[v].link;
    }

    last = v;
    path.eb(last);
    return last;
  }

 private:
  /// @brief Follows suffix links until a palindrome can be extended at pos.
  I32 get_link(I32 v, I32 pos) const {
    while (true) {
      I32 j = pos - 1 - nodes[v].len;
      if (j >= 0 && text[j] == text[pos]) return v;
      v = nodes[v].link;
    }
  }
};

#endif
