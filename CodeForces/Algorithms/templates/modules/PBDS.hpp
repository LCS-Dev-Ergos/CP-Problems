#pragma once
#include "templates/core/Contracts.hpp"
#include "templates/core/ScalarTypes.hpp"

#if !defined(PBDS_AVAILABLE) || !PBDS_AVAILABLE
  #ifndef CP_ALLOW_MISSING_PBDS
    #error "NEED_PBDS was requested but <ext/pb_ds/...> is unavailable on this toolchain (libstdc++ only). \
            Build with GCC, drop NEED_PBDS, or define CP_ALLOW_MISSING_PBDS."
  #endif
#endif

#if defined(PBDS_AVAILABLE) && PBDS_AVAILABLE
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

//===----------------------------------------------------------------------===//
/* Policy-Based Data Structures */

template <typename T>
using OrderedSet = __gnu_pbds::tree<
    T,
    __gnu_pbds::null_type,
    std::less<T>,
    __gnu_pbds::rb_tree_tag,
    __gnu_pbds::tree_order_statistics_node_update>;

template <typename T>
class OrderedMultiSet {
  using Key = std::pair<T, U64>;
  using Tree = __gnu_pbds::tree<
      Key,
      __gnu_pbds::null_type,
      std::less<Key>,
      __gnu_pbds::rb_tree_tag,
      __gnu_pbds::tree_order_statistics_node_update>;

  Tree data_;
  U64 next_id_ = 0;

public:
  class ConstIterator {
    typename Tree::const_iterator it_;
    friend class OrderedMultiSet;
    explicit ConstIterator(typename Tree::const_iterator it) : it_(it) {}

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    ConstIterator() = default;
    reference operator*() const { return it_->first; }
    pointer operator->() const { return std::addressof(it_->first); }
    ConstIterator& operator++() {
      ++it_;
      return *this;
    }
    ConstIterator operator++(int) {
      auto copy = *this;
      ++*this;
      return copy;
    }
    ConstIterator& operator--() {
      --it_;
      return *this;
    }
    ConstIterator operator--(int) {
      auto copy = *this;
      --*this;
      return copy;
    }
    friend bool operator==(const ConstIterator&, const ConstIterator&) = default;
  };

  using iterator = ConstIterator;
  using const_iterator = ConstIterator;

  [[nodiscard]] bool empty() const { return data_.empty(); }
  [[nodiscard]] Size size() const { return data_.size(); }
  void clear() { data_.clear(); next_id_ = 0; }

  iterator begin() const { return iterator(data_.begin()); }
  iterator end() const { return iterator(data_.end()); }

  iterator insert(const T& value) {
    CP_EXPECT(next_id_ != Limits<U64>::max(), "OrderedMultiSet: insertion id exhausted.");
    return iterator(data_.insert({value, next_id_++}).first);
  }

  iterator find(const T& value) const {
    auto it = data_.lower_bound({value, 0});
    return it != data_.end() && it->first == value ? iterator(it) : end();
  }

  bool erase_one(const T& value) {
    auto it = data_.lower_bound({value, 0});
    if (it == data_.end() || it->first != value)
      return false;
    data_.erase(*it);
    return true;
  }

  Size erase(const T& value) {
    Size removed = 0;
    while (erase_one(value))
      ++removed;
    return removed;
  }

  [[nodiscard]] Size order_of_key(const T& value) const {
    return data_.order_of_key({value, 0});
  }

  [[nodiscard]] Size count(const T& value) const {
    return data_.order_of_key({value, Limits<U64>::max()}) - order_of_key(value);
  }

  iterator find_by_order(Size order) const { return iterator(data_.find_by_order(order)); }
};

template <typename K, typename V>
using OrderedMap = __gnu_pbds::tree<
    K, V,
    std::less<K>,
    __gnu_pbds::rb_tree_tag,
    __gnu_pbds::tree_order_statistics_node_update>;

template <typename K, typename V>
using GPHashTable = __gnu_pbds::gp_hash_table<
    K, V,
    std::hash<K>, std::equal_to<K>,
    __gnu_pbds::direct_mask_range_hashing<>,
    __gnu_pbds::linear_probe_fn<>,
    __gnu_pbds::hash_standard_resize_policy<
    __gnu_pbds::hash_exponential_size_policy<>,
    __gnu_pbds::hash_load_check_resize_trigger<>,
    true>>;

template <typename T>
using ordered_set = OrderedSet<T>;

template <typename T>
using ordered_multiset = OrderedMultiSet<T>;

template <typename K, typename V>
using ordered_map = OrderedMap<K, V>;

template <typename K, typename V>
using gp_hash_table = GPHashTable<K, V>;
#endif
