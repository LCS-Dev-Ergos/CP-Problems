#ifndef CP_MODULES_OPTIMIZATION_LARSCH_HPP
#define CP_MODULES_OPTIMIZATION_LARSCH_HPP

#include "_Common.hpp"

/**
 * @file LARSCH.hpp
 * @brief Online row minima of an implicit Monge matrix.
 *
 * @details SMAWK needs the whole matrix to be answerable up front. LARSCH
 * lifts that restriction: row @c i is queried only after rows @c 0..i-1 have
 * been answered, and its entries are read only in columns @c 0..i. That is
 * exactly the dependency shape of @c dp[i]=min_j(dp[j]+cost(j,i)), which is
 * why this is the ingredient that turns Monge shortest path into a linear
 * time algorithm instead of a quadratic one.
 *
 * The construction is SMAWK run incrementally: each level alternately halves
 * the rows and reduces the columns, and every level keeps just enough state
 * to answer one more row on demand.
 */
template <typename T, typename F>
struct LARSCH {
private:
  struct ReduceRow;
  struct ReduceCol;

  /// @brief Chain of column renamings, one link per column-reduction level.
  struct ColumnMap {
    const ColumnMap* parent = nullptr;
    const VecI32* table = nullptr;

    [[nodiscard]] auto map(I32 j) const -> I32 {
      const I32 x = (table != nullptr ? (*table)[j] : j);
      return (parent != nullptr ? parent->map(x) : x);
    }
  };

  /**
   * @brief Reads one matrix entry through the level's index remapping.
   *
   * @details Rows are remapped arithmetically, since halving the rows only
   * ever selects an arithmetic progression, while columns need the explicit
   * table built by the reduction.
   */
  struct Entry {
    const F* f = nullptr;
    I64 stride = 1;
    I64 shift = 0;
    const ColumnMap* cols = nullptr;

    auto operator()(I32 i, I32 j) const -> T {
      const I32 row = I32(stride * I64(i) + shift);
      const I32 col = (cols != nullptr ? cols->map(j) : j);
      return (*f)(row, col);
    }
  };

  /// @brief Level that answers the odd rows recursively and fills in the even ones.
  struct ReduceRow {
    I32 n;
    Entry eval;
    I32 cur_row = 0;
    I32 state = 0;
    std::unique_ptr<ReduceCol> rec;

    ReduceRow(I32 rows, const Entry& e) : n(rows), eval(e) {
      const I32 half = n / 2;
      if (half > 0) {
        Entry odd = e;
        odd.shift = e.stride + e.shift;
        odd.stride = 2 * e.stride;
        rec = std::make_unique<ReduceCol>(half, odd);
      }
    }

    void reset() {
      cur_row = 0;
      state = 0;
      if (rec) rec->reset();
    }

    auto get_argmin() -> I32 {
      const I32 i = cur_row++;
      if ((i & 1) == 0) {
        // An even row is bracketed by the argmins of the odd rows around it.
        const I32 lo = state;
        const I32 hi = (i + 1 == n ? n - 1 : rec->get_argmin());
        state = hi;
        I32 best = lo;
        FOR(j, lo + 1, hi + 1) {
          if (eval(i, best) > eval(i, j)) best = j;
        }
        return best;
      }
      // An odd row was already narrowed to two candidates by the level below.
      return (eval(i, state) <= eval(i, i)) ? state : i;
    }
  };

  /// @brief Level that keeps only the columns still able to win some row.
  struct ReduceCol {
    I32 n;
    Entry eval;
    I32 cur_row = 0;
    VecI32 cols;
    ColumnMap map_here;
    ReduceRow rec;

    ReduceCol(I32 rows, const Entry& e)
        : n(rows),
          eval(e),
          map_here{e.cols, &cols},
          rec(rows, Entry{e.f, e.stride, e.shift, &map_here}) {
      cols.reserve(n);
    }

    void reset() {
      cur_row = 0;
      cols.clear();
      rec.reset();
    }

    /// @brief Offers column @p j to the stack, which is only valid up to row @p rows.
    void push_col(I32 j, I32 rows) {
      while (!cols.empty()) {
        const I32 height = isz(cols);
        if (height == rows) break;
        if (eval(height - 1, cols.back()) > eval(height - 1, j)) {
          cols.pop_back();
        } else {
          break;
        }
      }
      if (isz(cols) != n) cols.push_back(j);
    }

    auto get_argmin() -> I32 {
      const I32 i = cur_row++;
      if (i == 0) {
        cols.clear();
        cols.push_back(0);
      } else {
        push_col(2 * i - 1, i);
        push_col(2 * i, i);
      }
      return cols[rec.get_argmin()];
    }
  };

  F f_;
  ColumnMap root_map_{};
  Entry root_eval_{};
  std::unique_ptr<ReduceRow> base_;

public:
  /**
   * @brief Prepares @p n queries against the matrix given by @p f.
   *
   * @param n Number of rows that will be requested, one per @ref get_argmin.
   * @param f Entry accessor @c f(i,j); it is copied and must stay callable
   * for the lifetime of the object. Row @c i is only ever read in columns
   * @c 0..i, so entries above the diagonal may be reported as unreachable.
   */
  LARSCH(I32 n, F f) : f_(std::move(f)), root_eval_{&f_, 1, 0, &root_map_} {
    my_assert(n > 0);
    base_ = std::make_unique<ReduceRow>(n, root_eval_);
  }

  // Every level holds a pointer back into the level above it, so the object
  // graph is pinned to its address once built.
  LARSCH(const LARSCH&) = delete;
  LARSCH(LARSCH&&) = delete;
  auto operator=(const LARSCH&) -> LARSCH& = delete;
  auto operator=(LARSCH&&) -> LARSCH& = delete;
  ~LARSCH() = default;

  /// @brief Replays the sequence of rows from the first one.
  void reset() { base_->reset(); }

  /// @brief Argmin column of the next row, in amortised constant time.
  auto get_argmin() -> I32 { return base_->get_argmin(); }
};

#endif
