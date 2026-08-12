#pragma once

#include "templates/core/CoreConcepts.hpp"
#include "templates/core/Contracts.hpp"
#include "templates/core/ScalarTypes.hpp"
#include "templates/core/TypeTraits.hpp"
#include <cerrno>

#if CP_FAST_IO_VARIANT != 0 && CP_FAST_IO_VARIANT != 1
  #error "CP_FAST_IO_VARIANT must be 0 or 1."
#endif

#ifndef CP_FAST_IO_NAMESPACE_DEFINED
  #define CP_FAST_IO_NAMESPACE_DEFINED 1
#endif
#ifdef CP_IO_COMPAT_FAST_IO_NAMESPACE_DEFINED
  #error "Fast_IO.hpp must be included before IO.hpp when both I/O backends are used."
#endif

//===----------------------------------------------------------------------===//
/* Fast I/O */

namespace fast_io {

template <class T>
concept FastIntegral = cp::Int<T> && !cp::Same<T, bool> && !cp::Same<T, char>;

template <class T>
concept FastFloating = cp::Float<T>;

inline constexpr U32 BUFFER_SIZE = 1U << 20;
alignas(64) inline char input_buffer[BUFFER_SIZE];
alignas(64) inline char output_buffer[BUFFER_SIZE];
alignas(64) inline char number_buffer[160];

struct FourDigitTable {
  char digits[10'000][4];

  constexpr FourDigitTable() : digits{} {
    for (U32 value = 0; value < 10'000; ++value) {
      U32 x = value;
      for (I32 pos = 3; pos >= 0; --pos) {
        digits[value][pos] = as<char>('0' + (x % 10));
        x /= 10;
      }
    }
  }
};

inline constexpr FourDigitTable four_digit_table{};

inline U32 input_pos  = 0;
inline U32 input_end  = 0;
inline U32 output_pos = 0;

#if CP_FAST_IO_VARIANT == 1
inline bool input_eof = false;
#endif

inline void load_input() {
#if CP_FAST_IO_VARIANT == 1
  if (input_eof && input_pos >= input_end) {
    input_pos = input_end = 0;
    return;
  }
  const U32 remaining = input_end - input_pos;
  std::memmove(input_buffer, input_buffer + input_pos, remaining);
  const U32 capacity   = BUFFER_SIZE - remaining;
  const U32 bytes_read = as<U32>(std::fread(input_buffer + remaining, 1, capacity, stdin));
  input_end = remaining + bytes_read;
  input_pos = 0;
  if (bytes_read < capacity) {
    input_eof = true;
    if (input_end < BUFFER_SIZE)
      input_buffer[input_end++] = '\n';
  }
#else
  input_end = as<U32>(std::fread(input_buffer, 1, BUFFER_SIZE, stdin));
  input_pos = 0;
#endif
}

[[gnu::always_inline]] inline char next_input_char() {
  if (input_pos >= input_end) {
    load_input();
    if (input_pos >= input_end)
      return 0;
  }
  return input_buffer[input_pos++];
}

[[gnu::always_inline]] inline char next_token_char() {
  char c;
  do {
    c = next_input_char();
  } while (c <= ' ' && c != 0);
  CP_EXPECT(c != 0, "Fast I/O: unexpected EOF.");
  return c;
}

namespace detail {

inline constexpr U64 DIGIT_ZEROES = 0x3030'3030'3030'3030ULL;
inline constexpr U64 DIGIT_NINES  = 0x0909'0909'0909'0909ULL;
inline constexpr U64 HIGH_BITS    = 0x8080'8080'8080'8080ULL;

template <class U>
[[gnu::always_inline]] inline bool append_digit_block(U& value, U limit) {
  if constexpr (std::endian::native != std::endian::little || Limits<U>::digits < 32) {
    return false;
  } else {
    if (input_end - input_pos < 8)
      return false;

    U64 block;
    std::memcpy(&block, input_buffer + input_pos, 8);
    block -= DIGIT_ZEROES;
    if ((block | (DIGIT_NINES - block)) & HIGH_BITS)
      return false;

    block = (block * 10 + (block >> 8)) & 0x00ff'00ff'00ff'00ffULL;
    block = (block * 100 + (block >> 16)) & 0x0000'ffff'0000'ffffULL;
    const U digits = as<U>((block * 10'000 + (block >> 32)) & 0xffff'ffffULL);
    CP_EXPECT(value <= (limit - digits) / U(100'000'000),
              "Fast I/O: integer token out of range.");
    value = as<U>(value * U(100'000'000) + digits);
    input_pos += 8;
    return true;
  }
}

} // namespace detail

inline void flush_output() {
  if (output_pos == 0)
    return;
  const Size written = std::fwrite(output_buffer, 1, output_pos, stdout);
  CP_EXPECT(written == output_pos, "Fast I/O: failed to flush stdout.");
  output_pos = 0;
}

inline void read_char(char& c) {
  c = next_token_char();
}

template <typename T>
inline void read_integer(T& x) {
  using U = cp::MakeUnsignedT<T>;
  char c = next_token_char();

  bool negative = false;
  if constexpr (cp::Signed<T>) {
    if (c == '-') {
      negative = true;
      c = next_input_char();
    }
  }

  const U limit = negative ? U(Limits<T>::max()) + U(1) : U(Limits<T>::max());
  CP_EXPECT(c >= '0' && c <= '9', "Fast I/O: integer token has no digits.");
  U value = as<U>(c - '0');
  while (true) {
    if (detail::append_digit_block(value, limit))
      continue;
    c = next_input_char();
    if (c <= ' ')
      break;
    CP_EXPECT(c >= '0' && c <= '9', "Fast I/O: invalid integer token.");
    const U digit = as<U>(c - '0');
    CP_EXPECT(value <= (limit - digit) / U(10), "Fast I/O: integer token out of range.");
    value = as<U>(value * U(10) + digit);
  }

  if constexpr (cp::Signed<T>) {
    if (negative && value == U(Limits<T>::max()) + U(1))
      x = Limits<T>::min();
    else
      x = negative ? -as<T>(value) : as<T>(value);
  } else {
    x = as<T>(value);
  }
}

inline void read_string(std::string& s) {
  s.clear();
  s.reserve(32);
  char c = next_token_char();

  while (c > ' ') {
    CP_EXPECT(s.size() < as<Size>(CP_FAST_IO_MAX_TOKEN_SIZE),
              "Fast I/O: token exceeds CP_FAST_IO_MAX_TOKEN_SIZE.");
    s.push_back(c);
    c = next_input_char();
  }
}

template <typename T>
inline void read_floating(T& x) {
  std::string token;
  read_string(token);
  errno = 0;
  char* end = nullptr;
  if constexpr (cp::Same<T, F32>)
    x = std::strtof(token.c_str(), &end);
  else if constexpr (cp::Same<T, F64>)
    x = std::strtod(token.c_str(), &end);
  else
    x = std::strtold(token.c_str(), &end);
  CP_EXPECT(end == token.c_str() + token.size() && errno != ERANGE,
            "Fast I/O: invalid or out-of-range floating token.");
}

template <FastIntegral T>
inline void read(T& x) { read_integer(x); }

template <FastFloating T>
inline void read(T& x) { read_floating(x); }
inline void read(char& x) { read_char(x); }
inline void read(std::string& x) { read_string(x); }

template <class T>
concept FastModIntLike = requires(T value, I64 raw) {
  typename T::CPModIntMarker;
  T(raw);
  value.val();
};

template <class T>
concept FastStrongTypeLike = requires(T value, typename T::value_type raw) {
  typename T::CPStrongTypeMarker;
  requires FastIntegral<typename T::value_type>;
  T::from_raw(raw);
  value.get();
};

template <FastModIntLike T>
inline void read(T& x) {
  I64 value;
  read_integer(value);
  x = T(value);
}

template <FastStrongTypeLike T>
inline void read(T& x) {
  typename T::value_type value;
  read_integer(value);
  x = T::from_raw(value);
}

template <typename T>
inline void write_integer(T x) {
  using UnsignedT = cp::MakeUnsignedT<T>;
  UnsignedT ux;
  if constexpr (cp::Signed<T>) {
    ux = x < 0 ? as<UnsignedT>(-(x + 1)) + 1 : as<UnsignedT>(x);
  } else {
    ux = as<UnsignedT>(x);
  }

  I32 begin = I32(sizeof(number_buffer));
  while (ux >= 10'000) {
    const U32 chunk = as<U32>(ux % 10'000);
    ux /= 10'000;
    begin -= 4;
    std::memcpy(number_buffer + begin, four_digit_table.digits[chunk], 4);
  }

  const U32 head = as<U32>(ux);
  if (head >= 1'000) {
    begin -= 4;
    std::memcpy(number_buffer + begin, four_digit_table.digits[head], 4);
  } else if (head >= 100) {
    begin -= 3;
    std::memcpy(number_buffer + begin, four_digit_table.digits[head] + 1, 3);
  } else if (head >= 10) {
    begin -= 2;
    number_buffer[begin]     = as<char>('0' + head / 10);
    number_buffer[begin + 1] = as<char>('0' + head % 10);
  } else {
    number_buffer[--begin] = as<char>('0' + head);
  }

  if constexpr (cp::Signed<T>) {
    if (x < 0)
      number_buffer[--begin] = '-';
  }

  [[assume(begin >= 0)]];
  const U32 len = as<U32>(I32(sizeof(number_buffer)) - begin);
  if (output_pos + len >= BUFFER_SIZE)
    flush_output();
  std::memcpy(output_buffer + output_pos, number_buffer + begin, len);
  output_pos += len;
}

inline void write_char(char c) {
  if (output_pos >= BUFFER_SIZE)
    flush_output();
  output_buffer[output_pos++] = c;
}

inline void write_string(std::string_view s) {
  const char* data = s.data();
  Size remaining = s.size();
  while (remaining > 0) {
    if (output_pos >= BUFFER_SIZE)
      flush_output();
    const U32 space = BUFFER_SIZE - output_pos;
    const U32 chunk = as<U32>(std::min<Size>(remaining, space));
    std::memcpy(output_buffer + output_pos, data, chunk);
    output_pos += chunk;
    data += chunk;
    remaining -= chunk;
  }
}

template <typename T>
inline void write_floating(T x) {
  char local_buffer[512];
  int n = std::snprintf(local_buffer, sizeof(local_buffer), "%.*Lf", CP_FLOAT_PRECISION, as<F80>(x));
  CP_EXPECT(n > 0, "Fast I/O: failed to format floating value.");
  if (as<Size>(n) < sizeof(local_buffer)) {
    write_string(std::string_view(local_buffer, as<Size>(n)));
    return;
  }

  std::string spill(as<Size>(n) + 1, '\0');
  n = std::snprintf(spill.data(), spill.size(), "%.*Lf", CP_FLOAT_PRECISION, as<F80>(x));
  CP_EXPECT(n > 0 && as<Size>(n) < spill.size(), "Fast I/O: failed to format floating value.");
  write_string(std::string_view(spill.data(), as<Size>(n)));
}

template <FastIntegral T>
inline void write(T x) { write_integer(x); }

template <FastFloating T>
inline void write(T x) { write_floating(x); }

inline void write(char x) { write_char(x); }
inline void write(const std::string& x) { write_string(x); }
inline void write(const char* x) {
  CP_EXPECT(x != nullptr, "Fast I/O: cannot write a null C string.");
  write_string(x);
}

template <FastModIntLike T>
inline void write(const T& x) { write_integer(x.val()); }

template <FastStrongTypeLike T>
inline void write(const T& x) { write_integer(x.get()); }

#if CP_IO_ENABLE_COMPOSITE
  #define CP_IO_COMPOSITE_CONTEXT 1
  #include "templates/modules/IO_Composite.hpp"
  #undef CP_IO_COMPOSITE_CONTEXT
#endif

template <class Head, class... Tail>
  requires(sizeof...(Tail) > 0)
inline void read(Head& head, Tail&... tail) {
  read(head);
  read(tail...);
}

template <class Head, class... Tail>
  requires(sizeof...(Tail) > 0)
inline void write(const Head& head, const Tail&... tail) {
  write(head);
  write(' ');
  write(tail...);
}

inline void writeln() { write('\n'); }

template <class... Args>
inline void writeln(const Args&... args) {
  if constexpr (sizeof...(args) > 0)
    write(args...);
  writeln();
}

template <class T>
  requires(!FastIntegral<T> && !FastFloating<T> && !FastModIntLike<T> && !FastStrongTypeLike<T>)
void read(T&) = delete;

template <class T>
  requires(!FastIntegral<T> && !FastFloating<T> && !FastModIntLike<T> && !FastStrongTypeLike<T>)
void write(const T&) = delete;

struct IOFlusher {
  ~IOFlusher() { flush_output(); }
};

inline IOFlusher io_flusher;

} // namespace fast_io

#ifdef CP_IO_IMPL_READ
  #undef CP_IO_IMPL_READ
#endif
#ifdef CP_IO_IMPL_WRITELN
  #undef CP_IO_IMPL_WRITELN
#endif
#ifdef CP_IO_IMPL_FLUSH
  #undef CP_IO_IMPL_FLUSH
#endif

#define CP_IO_IMPL_READ(...) fast_io::read(__VA_ARGS__)
#define CP_IO_IMPL_WRITELN(...) fast_io::writeln(__VA_ARGS__)
#define CP_IO_IMPL_FLUSH() fast_io::flush_output()

#include "templates/modules/IO_Defs.hpp"
