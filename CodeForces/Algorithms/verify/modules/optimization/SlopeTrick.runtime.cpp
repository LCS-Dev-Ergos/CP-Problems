#include "modules/optimization/SlopeTrick.hpp"

#include <cassert>
#include <random>

namespace {

constexpr I64 SCAN_LO = -400;
constexpr I64 SCAN_HI = 400;

/// Independent model: the function as an explicit list of hinge terms.
struct Ref {
  VecI64 left;   ///< Terms max(0, l - x).
  VecI64 right;  ///< Terms max(0, x - r).
  I64 base = 0;

  [[nodiscard]] auto eval(I64 x) const -> I64 {
    I64 y = base;
    for (const I64 l : left) y += std::max<I64>(0, l - x);
    for (const I64 r : right) y += std::max<I64>(0, x - r);
    return y;
  }

  [[nodiscard]] auto min_value() const -> I64 {
    I64 best = infinity<I64>;
    FOR(x, SCAN_LO, SCAN_HI + 1) chmin(best, eval(x));
    return best;
  }

  /**
   * Rewrites the same function with the decreasing breakpoints on the left and
   * the increasing ones on the right. Terms accumulate in arrival order, which
   * can leave a left breakpoint above a right one; the shifting operations are
   * only defined on the sorted form, so they need this first.
   */
  void canonicalize() {
    const I32 nl = isz(left);
    VecI64 pts = left;
    for (const I64 r : right) pts.push_back(r);
    std::sort(pts.begin(), pts.end());
    const I64 folded = min_value();
    left.assign(pts.begin(), pts.begin() + nl);
    right.assign(pts.begin() + nl, pts.end());
    base = folded;
  }
};

void check_agrees(const SlopeTrick& st, const Ref& ref) {
  assert(st.min() == ref.min_value());
  FOR(x, SCAN_LO, SCAN_HI + 1) assert(st.eval(x) == ref.eval(x));
}

}  // namespace

int main() {
  std::mt19937 rng(20260809);

  FOR(iter, 60) {
    SlopeTrick st;
    Ref ref;
    check_agrees(st, ref);

    FOR(step, 25) {
      const I64 a = I64(rng() % 61) - 30;
      switch (rng() % 7) {
        case 0:
          st.add_const(a);
          ref.base += a;
          break;
        case 1:
          st.add_a_minus_x(a);
          ref.left.push_back(a);
          break;
        case 2:
          st.add_x_minus_a(a);
          ref.right.push_back(a);
          break;
        case 3:
          st.add_abs(a);
          ref.left.push_back(a);
          ref.right.push_back(a);
          break;
        case 4: {
          const I64 s = I64(rng() % 11) - 5;
          st.shift(s);
          for (I64& l : ref.left) l += s;
          for (I64& r : ref.right) r += s;
          break;
        }
        case 5: {
          // Verify the window minimum against the pre-operation model, then
          // fold it into the model.
          ref.canonicalize();
          const I64 lo = I64(rng() % 7) - 3;
          const I64 hi = lo + I64(rng() % 5);
          VecI64 want(SCAN_HI - SCAN_LO + 1);
          FOR(i, isz(want)) {
            const I64 x = SCAN_LO + i;
            I64 best = infinity<I64>;
            FOR(y, x - hi, x - lo + 1) chmin(best, ref.eval(y));
            want[i] = best;
          }
          st.sliding_window_minimum(lo, hi);
          FOR(i, isz(want)) assert(st.eval(SCAN_LO + i) == want[i]);
          for (I64& l : ref.left) l += lo;
          for (I64& r : ref.right) r += hi;
          break;
        }
        default: {
          // Dropping one arm turns the function into a running minimum.
          ref.canonicalize();
          const bool drop_right = (rng() % 2 == 0);
          VecI64 want(SCAN_HI - SCAN_LO + 1);
          FOR(i, isz(want)) {
            const I64 x = SCAN_LO + i;
            I64 best = infinity<I64>;
            if (drop_right) {
              FOR(y, SCAN_LO, x + 1) chmin(best, ref.eval(y));
            } else {
              FOR(y, x, SCAN_HI + 1) chmin(best, ref.eval(y));
            }
            want[i] = best;
          }
          if (drop_right) {
            st.clear_right();
            ref.right.clear();
          } else {
            st.clear_left();
            ref.left.clear();
          }
          FOR(i, isz(want)) assert(st.eval(SCAN_LO + i) == want[i]);
          break;
        }
      }
      check_agrees(st, ref);

      // The minimum has to be attained exactly on the reported interval.
      const auto [lo_arg, hi_arg, value] = st.argmin();
      assert(lo_arg <= hi_arg);
      assert(value == st.min());
      if (lo_arg > SlopeTrick::LEFT_EMPTY && hi_arg < SlopeTrick::RIGHT_EMPTY) {
        assert(st.eval(lo_arg) == value);
        assert(st.eval(hi_arg) == value);
        if (lo_arg > SCAN_LO) assert(st.eval(lo_arg - 1) > value);
        if (hi_arg < SCAN_HI) assert(st.eval(hi_arg + 1) > value);
      }
    }

    // add_linear is only representable once the arm being flattened owns
    // enough breakpoints, so it is checked last, directly against eval.
    {
      const I32 room = std::min(isz(st.left), isz(st.right));
      if (room > 0) {
        const I64 slope = I64(rng() % U32(2 * room + 1)) - room;
        const I64 offset = I64(rng() % 21) - 10;
        VecI64 want(SCAN_HI - SCAN_LO + 1);
        FOR(i, isz(want)) {
          const I64 x = SCAN_LO + i;
          want[i] = ref.eval(x) + slope * x + offset;
        }
        st.add_linear(slope, offset);
        FOR(i, isz(want)) assert(st.eval(SCAN_LO + i) == want[i]);
        I64 best = infinity<I64>;
        for (const I64 v : want) chmin(best, v);
        assert(st.min() == best);
      }
    }
  }

  return 0;
}
