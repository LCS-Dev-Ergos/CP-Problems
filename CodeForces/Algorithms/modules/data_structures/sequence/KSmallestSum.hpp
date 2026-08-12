#ifndef CP_MODULES_DATA_STRUCTURES_SEQUENCE_K_SMALLEST_SUM_HPP
#define CP_MODULES_DATA_STRUCTURES_SEQUENCE_K_SMALLEST_SUM_HPP

#include "../_Common.hpp"

/**
 * @file KSmallestSum.hpp
 * @brief Dynamic sum of the first K values under an ordering.
 *
 * @details Insertions and erasures cost O(log N). Changing K costs
 * O((1+|delta K|) log N), so the intended use keeps K fixed or moves it
 * gradually.
 */
template <typename T, typename Compare = std::less<T>, typename Sum = T>
struct KExtremalSum {
private:
  OrderedMultiSetBy<T, Compare> chosen;
  OrderedMultiSetBy<T, Compare> rest;
  Compare comp;
  Sum total{};
  I32 k = 0;

  void move_to_chosen(typename OrderedMultiSetBy<T, Compare>::iterator it) {
    total += Sum(*it);
    chosen.insert(*it);
    rest.erase(it);
  }

  void move_to_rest(typename OrderedMultiSetBy<T, Compare>::iterator it) {
    total -= Sum(*it);
    rest.insert(*it);
    chosen.erase(it);
  }

  void normalize() {
    const I32 need = std::min(k, size());
    while (isz(chosen) < need) move_to_chosen(rest.begin());
    while (isz(chosen) > need) move_to_rest(std::prev(chosen.end()));
    while (!chosen.empty() && !rest.empty()) {
      auto a = std::prev(chosen.end());
      auto b = rest.begin();
      if (!comp(*b, *a)) break;
      const T x = *a;
      const T y = *b;
      total += Sum(y) - Sum(x);
      chosen.erase(a);
      rest.erase(b);
      chosen.insert(y);
      rest.insert(x);
    }
  }

public:
  explicit KExtremalSum(I32 k_ = 0, Compare comp_ = {})
      : chosen(comp_), rest(comp_), comp(comp_), k(k_) {
    my_assert(k >= 0);
  }

  void insert(const T& x) {
    rest.insert(x);
    normalize();
  }

  void erase(const T& x) {
    auto it = chosen.find(x);
    if (it != chosen.end()) {
      total -= Sum(*it);
      chosen.erase(it);
    } else {
      it = rest.find(x);
      my_assert(it != rest.end());
      rest.erase(it);
    }
    normalize();
  }

  void set_k(I32 k_) {
    my_assert(k_ >= 0);
    k = k_;
    normalize();
  }

  [[nodiscard]] auto sum() const -> Sum { return total; }
  [[nodiscard]] auto limit() const -> I32 { return k; }
  [[nodiscard]] auto size() const -> I32 { return isz(chosen) + isz(rest); }
  [[nodiscard]] auto empty() const -> bool { return size() == 0; }

  [[nodiscard]] auto boundary() const -> const T& {
    my_assert(!chosen.empty());
    return *std::prev(chosen.end());
  }
};

template <typename T, typename Sum = T>
using KSmallestSum = KExtremalSum<T, std::less<T>, Sum>;

template <typename T, typename Sum = T>
using KLargestSum = KExtremalSum<T, std::greater<T>, Sum>;

#endif
