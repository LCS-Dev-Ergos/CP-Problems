#ifndef CP_MODULES_STRINGS_MANACHER_HPP
#define CP_MODULES_STRINGS_MANACHER_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/// @brief Manacher algorithm for palindromic substrings in linear time.
struct Manacher {
  String s;
  VecI32 p;  // p[i] = radius of palindrome centered at 'i'.

  /// @brief Builds transformed string and palindrome radii.
  Manacher(const String& str) {
    // Transform string: "abc" -> "^#a#b#c#$".
    s = "^";
    for (char c : str) {
      s += "#";
      s += c;
    }
    s += "#$";

    I32 n = sz(s);
    p.assign(n, 0);

    I32 center = 0, right = 0;
    FOR(i, 1, n - 1) {
      I32 mirror = 2 * center - i;

      if (i < right) {
        p[i] = _min(as<I32>(right - i), p[mirror]);
      }

      // Expand around center 'i'.
      while (s[i + p[i] + 1] == s[i - p[i] - 1]) {
        p[i]++;
      }

      // Update center and right.
      if (i + p[i] > right) {
        center = i;
        right = i + p[i];
      }
    }
  }

  /// @brief Checks if substring [l, r) of original string is palindrome.
  bool is_palindrome(I32 l, I32 r) {
    I32 center = l + r + 1;  // Center in transformed string.
    I32 radius = r - l;
    return p[center] >= radius;
  }

  /// @brief Radius of longest odd palindrome centered at i.
  I32 odd_palindrome_at(I32 i) {
    return p[2 * i + 2] / 2;
  }

  /// @brief Radius of longest even palindrome centered between i and i+1.
  I32 even_palindrome_at(I32 i) {
    return (p[2 * i + 3] - 1) / 2;
  }
};

#endif
