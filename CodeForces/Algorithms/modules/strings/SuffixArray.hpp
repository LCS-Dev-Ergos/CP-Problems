#ifndef CP_MODULES_STRINGS_SUFFIX_ARRAY_HPP
#define CP_MODULES_STRINGS_SUFFIX_ARRAY_HPP

#include "_Common.hpp"
#include "templates/core/TypeTraits.hpp"

/**
 * @brief Suffix array + Kasai LCP with O(1) LCP queries.
 *
 * Construction:
 *  - suffix array in O(n log n) via counting-sort doubling on cyclic shifts
 *  - LCP in O(n) via Kasai
 *  - RMQ over LCP in O(n log n)
 */
struct SuffixArray {
  String s;
  VecI32 sa;    // Sorted suffix start positions.
  VecI32 rank;  // rank[i] = position of suffix i in sa.
  VecI32 lcp;   // lcp[k] = LCP(sa[k], sa[k+1]), size n-1.

  VecI32 lg;
  Vec2D<I32> st;

  SuffixArray() = default;
  explicit SuffixArray(const String& str) { build(str); }

  /// @brief Rebuilds all structures for input string.
  void build(const String& str) {
    s = str;
    build_sa();
    build_lcp();
    build_rmq();
  }

  /// @brief Returns LCP length of suffixes starting at i and j.
  I32 lcp_query(I32 i, I32 j) const {
    if (i == j) return sz(s) - i;
    I32 ri = rank[i];
    I32 rj = rank[j];
    if (ri > rj) std::swap(ri, rj);
    return rmq(ri, rj - 1);
  }

  /// @brief Finds all occurrences of pattern using binary search on SA.
  VecI32 find_pattern(const String& pattern) const {
    VecI32 result;
    const I32 n = sz(s);

    auto cmp_suffix_pattern = [&](I32 suffix_pos) -> I32 {
      I32 i = 0;
      while (suffix_pos + i < n && i < sz(pattern)) {
        char a = s[suffix_pos + i];
        char b = pattern[i];
        if (a < b) return -1;
        if (a > b) return 1;
        ++i;
      }
      return (i == sz(pattern)) ? 0 : -1;
    };

    I32 left = 0;
    I32 right = n;
    while (left < right) {
      I32 mid = (left + right) / 2;
      if (cmp_suffix_pattern(sa[mid]) < 0) {
        left = mid + 1;
      } else {
        right = mid;
      }
    }

    while (left < n && cmp_suffix_pattern(sa[left]) == 0) {
      result.push_back(sa[left]);
      ++left;
    }
    return result;
  }

private:
  /// @brief Builds suffix array and rank array for s.
  void build_sa() {
    const I32 n = sz(s);
    sa.clear();
    rank.clear();
    if (n == 0) return;

    // Build SA on s + sentinel(0) as cyclic shifts.
    const I32 m = n + 1;
    VecI32 a(m);
    FOR(i, n) a[i] = as<U8>(s[i]) + 1;
    a[n] = 0;

    VecI32 p(m), c(m);
    {
      const I32 alphabet = std::max<I32>(256 + 1, m);
      VecI32 cnt(alphabet, 0);
      FOR(i, m) cnt[a[i]]++;
      FOR(i, 1, alphabet) cnt[i] += cnt[i - 1];
      FOR_R(i, m) p[--cnt[a[i]]] = i;

      c[p[0]] = 0;
      I32 classes = 1;
      FOR(i, 1, m) {
        if (a[p[i]] != a[p[i - 1]]) ++classes;
        c[p[i]] = classes - 1;
      }
    }

    I32 k = 0;
    while ((1 << k) < m) {
      FOR(i, m) p[i] = (p[i] - (1 << k) + m) % m;

      I32 classes = 0;
      FOR(i, m) classes = std::max(classes, c[i] + 1);
      VecI32 cnt(classes, 0);
      FOR(i, m) cnt[c[p[i]]]++;
      FOR(i, 1, classes) cnt[i] += cnt[i - 1];

      VecI32 pn(m);
      FOR_R(i, m) pn[--cnt[c[p[i]]]] = p[i];
      p.swap(pn);

      VecI32 cn(m);
      cn[p[0]] = 0;
      classes = 1;
      FOR(i, 1, m) {
        auto cur = Pair<I32, I32>{c[p[i]], c[(p[i] + (1 << k)) % m]};
        auto prv = Pair<I32, I32>{c[p[i - 1]], c[(p[i - 1] + (1 << k)) % m]};
        if (cur != prv) ++classes;
        cn[p[i]] = classes - 1;
      }
      c.swap(cn);
      ++k;
    }

    // Drop sentinel suffix.
    sa.resize(n);
    FOR(i, n) sa[i] = p[i + 1];
    rank.assign(n, 0);
    FOR(i, n) rank[sa[i]] = i;
  }

  /// @brief Builds LCP array using Kasai algorithm.
  void build_lcp() {
    const I32 n = sz(s);
    lcp.assign(n > 0 ? n - 1 : 0, 0);
    if (n <= 1) return;

    I32 k = 0;
    FOR(i, n) {
      if (rank[i] == n - 1) {
        k = 0;
        continue;
      }
      I32 j = sa[rank[i] + 1];
      while (i + k < n && j + k < n && s[i + k] == s[j + k]) ++k;
      lcp[rank[i]] = k;
      if (k) --k;
    }
  }

  /// @brief Builds RMQ sparse table over LCP array.
  void build_rmq() {
    const I32 n = sz(lcp);
    lg.assign(n + 1, 0);
    FOR(i, 2, n + 1) lg[i] = lg[i / 2] + 1;
    if (n == 0) {
      st.clear();
      return;
    }

    const I32 max_log = lg[n] + 1;
    st.assign(max_log, VecI32(n));
    FOR(i, n) st[0][i] = lcp[i];
    FOR(k, 1, max_log) {
      FOR(i, n - (1 << k) + 1) {
        st[k][i] = std::min(st[k - 1][i], st[k - 1][i + (1 << (k - 1))]);
      }
    }
  }

  /// @brief RMQ query on LCP array for range [l, r].
  I32 rmq(I32 l, I32 r) const {
    if (l > r) return 0;
    I32 k = lg[r - l + 1];
    return std::min(st[k][l], st[k][r - (1 << k) + 1]);
  }
};

#endif
