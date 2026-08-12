#ifndef CP_MODULES_STRINGS_PREFIX_AUTOMATON_HPP
#define CP_MODULES_STRINGS_PREFIX_AUTOMATON_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"
#include "PrefixFunction.hpp"

/**
 * @brief Prefix-function automaton (KMP automaton).
 *
 * @details Builds transitions for pattern prefixes and allows linear scan over
 * text-like streams. Useful when transition table is reused many times.
 * Complexity:
 *  - build: O(|pattern| * ALPHABET)
 *  - transition/query: O(1)
 */
template <I32 ALPHABET = 26, char BASE = 'a'>
struct PrefixAutomaton {
  static_assert(ALPHABET > 0, "ALPHABET must be positive");

  using Row = Array<I32, ALPHABET>;

  String pattern;
  VecI32 pi;
  Vec<Row> next_state_table;

  PrefixAutomaton() = default;
  explicit PrefixAutomaton(const String& p) { build(p); }

  void build(const String& p) {
    pattern = p;
    pi = prefix_function(pattern);

    const I32 n = isz(pattern);
    next_state_table.assign((n + 1), Row{});
    for (auto& row : next_state_table) row.fill(0);

    FOR(i, n + 1) {
      FOR(c, ALPHABET) {
        const char ch = as<char>(BASE + c);
        if (i < n && pattern[i] == ch) {
          next_state_table[i][c] = i + 1;
        } else if (i == 0) {
          next_state_table[i][c] = 0;
        } else {
          next_state_table[i][c] = next_state_table[pi[i - 1]][c];
        }
      }
    }
  }

  /// @brief Returns next automaton state after reading ch from state.
  I32 next_state(I32 state, char ch) const {
    if (state < 0 || state >= isz(next_state_table)) return 0;
    const I32 c = as<I32>(ch - BASE);
    if (c < 0 || c >= ALPHABET) return 0;
    return next_state_table[state][c];
  }

  /// @brief Returns starting positions of all pattern occurrences in text.
  VecI32 find_occurrences(const String& text) const {
    VecI32 occ;
    const I32 m = isz(pattern);
    const I32 n = isz(text);
    if (m == 0) {
      occ.resize((n + 1));
      std::iota(all(occ), 0);
      return occ;
    }

    I32 state = 0;
    FOR(i, n) {
      state = next_state(state, text[i]);
      if (state == m) {
        occ.push_back(i - m + 1);
      }
    }
    return occ;
  }
};

#endif
