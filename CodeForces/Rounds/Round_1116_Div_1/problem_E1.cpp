#ifndef CP_TEMPLATE_PROFILE_STRICT
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#define NEED_MACROS
#define NEED_TIMER
#define CP_IO_PROFILE_FAST_MINIMAL

#include "templates/Base.hpp"

#include "modules/data_structures/segment_tree/DualSegmentTree.hpp"

//===----------------------------------------------------------------------===//
/* Temporal Tags */

struct H32 {
  I32 sum{};
  I32 pref{};
  I32 suf{};
  I32 best{};
};

struct H64 {
  I64 sum{};
  I64 pref{};
  I64 suf{};
  I64 best{};
  I32 len{};
};

auto cat(const H32& a, I32 na, const H32& b, I32 nb) -> H32 {
  if (na == 0) return b;
  if (nb == 0) return a;
  return {
    a.sum + b.sum,
    std::max(a.pref, a.sum + b.pref),
    std::max(b.suf, a.suf + b.sum),
    std::max({a.best, b.best, a.suf + b.pref}),
  };
}

auto cat(const H64& a, const H64& b) -> H64 {
  if (a.len == 0) return b;
  if (b.len == 0) return a;
  return {
    a.sum + b.sum,
    std::max(a.pref, a.sum + b.pref),
    std::max(b.suf, a.suf + b.sum),
    std::max({a.best, b.best, a.suf + b.pref}),
    a.len + b.len,
  };
}

auto one(I64 x) -> H64 { return {x, x, x, x, 1}; }

auto scale(const H32& h, I32 len, I64 k) -> H64 {
  return {
    I64(h.sum) * k,
    I64(h.pref) * k,
    I64(h.suf) * k,
    I64(h.best) * k,
    len,
  };
}

struct Hom {
  H32 h[2]{};
  I32 len{};
  I8 to[2]{1, -1};
};

struct Eval {
  I64 cur{};
  H64 h{};
};

auto eval(const Hom& f, I64 x) -> Eval {
  if (x == 0) {
    H64 h;
    h.len = f.len;
    return {0, h};
  }

  I32 s = x > 0 ? 0 : 1;
  I64 k = x > 0 ? x : -x;
  return {I64(f.to[s]) * k, scale(f.h[s], f.len, k)};
}

auto comp(const Hom& a, const Hom& b) -> Hom {
  Hom c;
  c.len = a.len + b.len;

  FOR(s, 2) {
    I32 x = a.to[s];
    H32 h;
    I8 to = 0;

    if (x > 0) {
      h  = b.h[0];
      to = b.to[0];
    } else if (x < 0) {
      h  = b.h[1];
      to = b.to[1];
    }

    c.h[s] = cat(a.h[s], a.len, h, b.len);
    c.to[s] = to;
  }

  return c;
}

constexpr I64 NO_SET = Limits<I64>::max();

struct Tag {
  Hom pre{};
  H64 post{};
  I64 cur = NO_SET;

  auto fixed() const -> bool { return cur != NO_SET; }
};

auto eval(const Tag& f, I64 x) -> Eval {
  Eval e = eval(f.pre, x);
  if (!f.fixed()) return e;
  e.h = cat(e.h, f.post);
  e.cur = f.cur;
  return e;
}

auto comp(const Tag& a, const Tag& b) -> Tag {
  Tag c;

  if (a.fixed()) {
    Eval e = eval(b, a.cur);
    c.pre  = a.pre;
    c.post = cat(a.post, e.h);
    c.cur  = e.cur;
  } else if (b.fixed()) {
    c.pre  = comp(a.pre, b.pre);
    c.post = b.post;
    c.cur  = b.cur;
  } else {
    c.pre  = comp(a.pre, b.pre);
  }

  return c;
}

auto init_tag(I64 x) -> Tag {
  Tag f;
  f.post = one(x);
  f.cur  = x;
  return f;
}

auto set_tag(I64 x) -> Tag {
  Tag f;
  f.cur = x;
  return f;
}

auto neg_tag() -> Tag {
  Tag f;
  f.pre.to[0] = -1;
  f.pre.to[1] = 1;
  return f;
}

auto clamp_tag() -> Tag {
  Tag f;
  f.pre.to[1] = 0;
  return f;
}

auto append_tag() -> Tag {
  Tag f;
  f.pre.h[0] = {1, 1, 1, 1};
  f.pre.h[1] = {-1, -1, -1, -1};
  f.pre.len = 1;
  return f;
}

struct TagMonoid {
  using value_type = Tag;

  static auto identity() -> Tag { return {}; }
  static auto combine(const Tag& a, const Tag& b) -> Tag { return comp(a, b); }
};

struct Seg : DualSegmentTree<TagMonoid> {
  using DualSegmentTree<TagMonoid>::DualSegmentTree;

  void apply_all(const Tag& f) {
    if (size == 1) {
      lazy[1] = TagMonoid::combine(lazy[1], f);
    } else if (has_lazy[1]) {
      lazy[1] = TagMonoid::combine(lazy[1], f);
    } else {
      lazy[1] = f;
      has_lazy[1] = true;
    }
  }
};

//===----------------------------------------------------------------------===//
/* Main Solver Function */

void solve() {
  INT(n, q);
  Vec<I64> a(n);
  FOR(i, n) IN(a[i]);

  I32 m = 1;
  while (m < n) m <<= 1;

  Seg seg(m, [&](I32 i) -> Tag {
    return i < n ? init_tag(a[i]) : Tag{};
  });

  const Tag neg = neg_tag();
  const Tag clamp  = clamp_tag();
  const Tag append = append_tag();
  U64 last = 0;

  auto dec = [&](U64 x) -> I32 {
    return I32((x ^ last) % U64(n));
  };

  FOR(q) {
    INT(op);

    if (op == 1) {
      ULL(u, v);
      LL(x);
      I32 l = dec(u);
      I32 r = dec(v);
      if (l > r) std::swap(l, r);
      seg.apply(l, r + 1, set_tag(x));
    } else if (op == 2 || op == 3) {
      ULL(u, v);
      I32 l = dec(u);
      I32 r = dec(v);
      if (l > r) std::swap(l, r);
      seg.apply(l, r + 1, op == 2 ? neg : clamp);
    } else {
      ULL(u);
      Tag f = seg.get(dec(u));
      I64 ans = f.post.best;
      OUT(ans);
      last = U64(ans);
    }

    seg.apply_all(append);
  }
}

//===----------------------------------------------------------------------===//
/* Main Function */

auto main() -> int {
#ifdef LOCAL
  Stopwatch timer;
#endif

  INT(T);
  FOR(T) solve();

  return 0;
}

//===----------------------------------------------------------------------===//
