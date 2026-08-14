#pragma once

//=====----- [ Portable STL ] -------------------------------------------=====//

// I/O
#include <fstream>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <streambuf>

// Containers
#include <array>
#include <bitset>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Algorithms and Iterators
#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>

// Strings
#include <string>
#include <string_view>

// Memory
#include <memory>
#include <new>
#include <scoped_allocator>

// Type Utilities
#include <cfloat>
#include <climits>
#include <initializer_list>
#include <limits>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

// Error Handling
#include <cassert>
#include <exception>
#include <stdexcept>
#include <system_error>

// Math and Numerics
#include <cinttypes>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <random>
#include <ratio>

// Time
#include <chrono>
#include <ctime>

// C Standard Library
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <clocale>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>

// C++17 additions
#include <any>
#include <optional>
#include <variant>

// C++20 — Required Baseline
#include <bit>
#include <compare>
#include <concepts>
#include <numbers>
#include <ranges>
#include <span>
#include <version>
#if __has_include(<source_location>)
  #include <source_location>
#endif
#if __has_include(<coroutine>)
  #include <coroutine>
#endif

// C++23 — included if available
#if __has_include(<format>)
  #include <format>
#endif
#if __has_include(<print>)
  #include <print>
#endif
#if __has_include(<expected>)
  #include <expected>
#endif
#if __has_include(<flat_map>)
  #include <flat_map>
#endif
#if __has_include(<flat_set>)
  #include <flat_set>
#endif
#if __has_include(<mdspan>)
  #include <mdspan>
#endif
#if __has_include(<generator>)
  #include <generator>
#endif
#if __has_include(<stacktrace>)
  #include <stacktrace>
#endif
