#pragma once
#include <functional>

//=====----- [ MinMax ] -------------------------------------------------=====//

#ifndef CP_UTILITY_FUNCTIONS_INCLUDED
#define CP_UTILITY_FUNCTIONS_INCLUDED
  template <class T, class S, class Compare = std::less<>>
  [[gnu::always_inline]] inline bool chmax(T& a, const S& b, const Compare& cmp = {}) {
    return cmp(a, b) ? (a = b, true) : false;
  }

  template <class T, class S, class Compare = std::less<>>
  [[gnu::always_inline]] inline bool chmin(T& a, const S& b, const Compare& cmp = {}) {
    return cmp(b, a) ? (a = b, true) : false;
  }
#endif

template <typename T>
constexpr T _min(const T& a, const T& b) { return (b < a) ? b : a; }

template <typename T>
constexpr T _max(const T& a, const T& b) { return (a < b) ? b : a; }

template <typename T, typename... Args>
constexpr T _min(const T& a, const T& b, const Args&... args) { return _min(a, _min(b, args...)); }

template <typename T, typename... Args>
constexpr T _max(const T& a, const T& b, const Args&... args) { return _max(a, _max(b, args...)); }
