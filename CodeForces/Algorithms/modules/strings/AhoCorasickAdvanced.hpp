#ifndef CP_MODULES_STRINGS_AHO_CORASICK_ADVANCED_HPP
#define CP_MODULES_STRINGS_AHO_CORASICK_ADVANCED_HPP

#include "_Common.hpp"

#include <cassert>

/**
 * @brief Advanced Aho-Corasick for lowercase alphabet.
 *
 * @details Supports:
 *  - adding patterns incrementally
 *  - collecting all matches (position, pattern_id)
 *  - counting occurrences of each pattern in O(|text| + states + matches)
 */
struct AhoCorasickAdvanced {
  static constexpr I32 ALPHABET = 26;
  static constexpr char BASE = 'a';

  struct Node {
    Array<I32, ALPHABET> next{};
    Array<I32, ALPHABET> go{};
    I32 link = 0;
    I32 term_link = -1;
    I32 parent = -1;
    char parent_char = 0;
    VecI32 out;

    Node() { next.fill(-1); go.fill(0); }
  };

  Vec<Node> nodes;
  VecI32 pattern_len;
  bool built = false;
  VecI32 bfs_order;

  AhoCorasickAdvanced() { reset(); }

  void reset() {
    nodes.assign(1, Node{});
    pattern_len.clear();
    built = false;
    bfs_order.clear();
  }

  /// @brief Adds one pattern and returns its id.
  I32 add_pattern(const String& pattern) {
    I32 id = isz(pattern_len);
    pattern_len.push_back(isz(pattern));

    I32 v = 0;
    for (char ch : pattern) {
      I32 c = ch - BASE;
      if (c < 0 || c >= ALPHABET) {
        #ifndef NDEBUG
          assert(false && "AhoCorasickAdvanced supports only ['a'..'z']");
        #endif
        return id;
      }
      if (nodes[v].next[c] == -1) {
        nodes[v].next[c] = isz(nodes);
        nodes.push_back(Node{});
        nodes.back().parent = v;
        nodes.back().parent_char = ch;
      }
      v = nodes[v].next[c];
    }
    nodes[v].out.push_back(id);
    built = false;
    return id;
  }

  void build() {
    Queue<I32> q;
    bfs_order.clear();
    bfs_order.reserve(nodes.size());

    for (auto& node : nodes) {
      node.link = 0;
      node.term_link = -1;
      node.go.fill(0);
    }

    FOR(c, ALPHABET) {
      I32 to = nodes[0].next[c];
      if (to != -1) {
        nodes[to].link = 0;
        nodes[0].go[c] = to;
        q.push(to);
      } else {
        nodes[0].go[c] = 0;
      }
    }

    while (!q.empty()) {
      I32 v = q.front();
      q.pop();
      bfs_order.push_back(v);
      const I32 link_v = nodes[v].link;
      if (!nodes[link_v].out.empty()) {
        nodes[v].term_link = link_v;
      } else {
        nodes[v].term_link = nodes[link_v].term_link;
      }

      FOR(c, ALPHABET) {
        I32 to = nodes[v].next[c];
        if (to == -1) {
          nodes[v].go[c] = nodes[link_v].go[c];
          continue;
        }

        I32 link_to = nodes[link_v].go[c];
        nodes[to].link = link_to;
        nodes[v].go[c] = to;
        q.push(to);
      }
    }

    built = true;
  }

  /// @brief Returns all matches as {end_position, pattern_id}.
  VecPairI32 all_matches(const String& text) {
    if (!built) build();
    VecPairI32 matches;
    I32 v = 0;
    FOR(i, isz(text)) {
      char ch = text[i];
      I32 c = ch - BASE;
      if (c < 0 || c >= ALPHABET) {
        v = 0;
        continue;
      }
      v = nodes[v].go[c];
      for (I32 id : nodes[v].out) {
        matches.push_back({i, id});
      }
      for (I32 u = nodes[v].term_link; u != -1; u = nodes[u].term_link) {
        for (I32 id : nodes[u].out) {
          matches.push_back({i, id});
        }
      }
    }
    return matches;
  }

  /// @brief Counts pattern occurrences in text.
  VecI64 count_occurrences(const String& text) {
    if (!built) build();
    VecI64 ans(pattern_len.size(), 0);
    I32 v = 0;
    for (char ch : text) {
      I32 c = ch - BASE;
      if (c < 0 || c >= ALPHABET) {
        v = 0;
        continue;
      }
      v = nodes[v].go[c];
      for (I32 id : nodes[v].out) {
        ++ans[id];
      }
      for (I32 u = nodes[v].term_link; u != -1; u = nodes[u].term_link) {
        for (I32 id : nodes[u].out) {
          ++ans[id];
        }
      }
    }
    return ans;
  }
};

#endif
