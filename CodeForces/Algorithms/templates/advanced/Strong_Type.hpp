#pragma once
#include "templates/advanced/RangeStreamConcepts.hpp"

//===----------------------------------------------------------------------===//
/* Strong Types */

namespace cp {

template <class T, class Tag>
class StrongType {
public:
  using value_type = T;
  using tag_type   = Tag;
  using CPStrongTypeMarker = void;

  constexpr StrongType()
    requires std::default_initializable<T> = default;

  explicit constexpr StrongType(const T& value) : value_(value) {}
  explicit constexpr StrongType(T&& value) : value_(std::move(value)) {}

  [[nodiscard]] static constexpr StrongType from_raw(T value) {
    return StrongType(std::move(value));
  }

  [[nodiscard]] constexpr const T& get() const& noexcept { return value_; }
  [[nodiscard]] constexpr T&  get() &  noexcept { return value_; }
  [[nodiscard]] constexpr T&& get() && noexcept { return std::move(value_); }

  [[nodiscard]] explicit constexpr operator const T&() const& noexcept { return value_; }
  [[nodiscard]] explicit constexpr operator T&() & noexcept { return value_; }
  explicit operator const T&() const&& = delete;
  explicit operator T&() && = delete;

  friend constexpr bool operator==(const StrongType&, const StrongType&) = default;
  friend constexpr auto operator<=>(const StrongType&, const StrongType&)
    requires std::three_way_comparable<T> = default;

  constexpr StrongType& operator+=(const StrongType& other)
    requires requires(T a, const T& b) { a += b; }
  {
    value_ += other.value_;
    return *this;
  }

  constexpr StrongType& operator-=(const StrongType& other)
    requires requires(T a, const T& b) { a -= b; }
  {
    value_ -= other.value_;
    return *this;
  }

  constexpr StrongType& operator*=(const StrongType& other)
    requires requires(T a, const T& b) { a *= b; }
  {
    value_ *= other.value_;
    return *this;
  }

  constexpr StrongType& operator/=(const StrongType& other)
    requires requires(T a, const T& b) { a /= b; }
  {
    value_ /= other.value_;
    return *this;
  }

  constexpr StrongType& operator%=(const StrongType& other)
    requires requires(T a, const T& b) { a %= b; }
  {
    value_ %= other.value_;
    return *this;
  }

  constexpr StrongType& operator++()
    requires requires(T a) { ++a; }
  {
    ++value_;
    return *this;
  }

  constexpr StrongType operator++(int)
    requires requires(T a) { a++; }
  {
    StrongType copy(*this);
    value_++;
    return copy;
  }

  constexpr StrongType& operator--()
    requires requires(T a) { --a; }
  {
    --value_;
    return *this;
  }

  constexpr StrongType operator--(int)
    requires requires(T a) { a--; }
  {
    StrongType copy(*this);
    value_--;
    return copy;
  }

  friend constexpr StrongType operator+(StrongType lhs, const StrongType& rhs)
    requires requires(StrongType a, const StrongType& b) { a += b; }
  {
    lhs += rhs;
    return lhs;
  }

  friend constexpr StrongType operator-(StrongType lhs, const StrongType& rhs)
    requires requires(StrongType a, const StrongType& b) { a -= b; }
  {
    lhs -= rhs;
    return lhs;
  }

  friend constexpr StrongType operator*(StrongType lhs, const StrongType& rhs)
    requires requires(StrongType a, const StrongType& b) { a *= b; }
  {
    lhs *= rhs;
    return lhs;
  }

  friend constexpr StrongType operator/(StrongType lhs, const StrongType& rhs)
    requires requires(StrongType a, const StrongType& b) { a /= b; }
  {
    lhs /= rhs;
    return lhs;
  }

  friend constexpr StrongType operator%(StrongType lhs, const StrongType& rhs)
    requires requires(StrongType a, const StrongType& b) { a %= b; }
  {
    lhs %= rhs;
    return lhs;
  }

  friend std::ostream& operator<<(std::ostream& os, const StrongType& value)
    requires StreamWritable<T>
  {
    return os << value.value_;
  }

  friend std::istream& operator>>(std::istream& is, StrongType& value)
    requires StreamReadable<T>
  {
    return is >> value.value_;
  }

  friend constexpr void swap(StrongType& a, StrongType& b)
      noexcept(noexcept(std::ranges::swap(a.get(), b.get()))) {
    std::ranges::swap(a.value_, b.value_);
  }

private:
  T value_{};
};

template <class T, class Tag>
[[nodiscard]] constexpr const T& unwrap(const StrongType<T, Tag>& value) noexcept {
  return value.get();
}

template <class T, class Tag>
[[nodiscard]] constexpr T& unwrap(StrongType<T, Tag>& value) noexcept {
  return value.get();
}

template <class T, class Tag>
[[nodiscard]] constexpr T&& unwrap(StrongType<T, Tag>&& value) noexcept {
  return std::move(value).get();
}

template <class T, class Tag>
[[nodiscard]] constexpr StrongType<T, Tag> make_strong(T value) {
  return StrongType<T, Tag>(std::move(value));
}

template <class Tag, class T>
[[nodiscard]] constexpr auto strong(T&& value) {
  using U = RemoveCvrefT<T>;
  return StrongType<U, Tag>(std::forward<T>(value));
}

template <NonBoolInt T, class Tag>
using StrongInt = StrongType<T, Tag>;

} // namespace cp

#ifndef CP_DECLARE_STRONG_TYPE
  #define CP_DECLARE_STRONG_TYPE(Name, UnderlyingType) \
    struct Name##Tag {}; \
    using Name##_tag = Name##Tag; \
    using Name = cp::StrongType<UnderlyingType, Name##Tag>
#endif

namespace std {

template <class T, class Tag>
struct hash<cp::StrongType<T, Tag>> {
  size_t operator()(const cp::StrongType<T, Tag>& value) const
      noexcept(noexcept(std::hash<T>{}(value.get()))) {
    return std::hash<T>{}(value.get());
  }
};

} // namespace std
