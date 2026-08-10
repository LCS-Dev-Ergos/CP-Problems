#pragma once
#include "templates/core/Preamble.hpp"

//===----------------------------------------------------------------------===//
/* Policy-Based Data Structures (optional) */

// Without this, a missing pb_ds surfaces as "ordered_set was not declared".
#if !defined(PBDS_AVAILABLE) || !PBDS_AVAILABLE
  #ifndef CP_ALLOW_MISSING_PBDS
    #error "NEED_PBDS was requested but <ext/pb_ds/...> is unavailable on this toolchain (libstdc++ only). \
            Build with GCC, drop NEED_PBDS, or define CP_ALLOW_MISSING_PBDS."
  #endif
#endif

#if defined(PBDS_AVAILABLE) && PBDS_AVAILABLE
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

template <typename T>
using ordered_set = __gnu_pbds::tree<
    T,
    __gnu_pbds::null_type,
    std::less<T>,
    __gnu_pbds::rb_tree_tag,
    __gnu_pbds::tree_order_statistics_node_update>;

/* WARNING: std::less_equal violates strict weak ordering. find() and erase()
 * by value will not work correctly. Use find_by_order / order_of_key only.
 * To erase, use: tree.erase(tree.find_by_order(tree.order_of_key(val)));
 */
template <typename T>
using ordered_multiset = __gnu_pbds::tree<
    T,
    __gnu_pbds::null_type,
    std::less_equal<T>,
    __gnu_pbds::rb_tree_tag,
    __gnu_pbds::tree_order_statistics_node_update>;

template <typename K, typename V>
using ordered_map = __gnu_pbds::tree<
    K, V,
    std::less<K>,
    __gnu_pbds::rb_tree_tag,
    __gnu_pbds::tree_order_statistics_node_update>;

template <typename K, typename V>
using gp_hash_table = __gnu_pbds::gp_hash_table<
    K, V,
    std::hash<K>, std::equal_to<K>,
    __gnu_pbds::direct_mask_range_hashing<>,
    __gnu_pbds::linear_probe_fn<>,
    __gnu_pbds::hash_standard_resize_policy<
    __gnu_pbds::hash_exponential_size_policy<>,
    __gnu_pbds::hash_load_check_resize_trigger<>,
    true>>;
#endif
