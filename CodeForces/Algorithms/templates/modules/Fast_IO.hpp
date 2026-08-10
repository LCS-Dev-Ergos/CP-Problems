#pragma once

#ifndef CP_FAST_IO_VARIANT
  #define CP_FAST_IO_VARIANT 1
#endif

#include "templates/core/IdiomAliases.hpp"
#include "templates/core/Macros.hpp"
#include "templates/core/ScalarTypes.hpp"
#include "templates/core/TypeTraits.hpp"

//===----------------------------------------------------------------------===//
/* Buffered I/O (variant-driven) */

#ifndef CP_FAST_IO_NAMESPACE_DEFINED
  #define CP_FAST_IO_NAMESPACE_DEFINED 1
#endif
#ifdef CP_IO_COMPAT_FAST_IO_NAMESPACE_DEFINED
  #error "Fast_IO.hpp must be included before IO.hpp when both I/O backends are used."
#endif

#include "templates/modules/Fast_IO_Fwd.hpp"

namespace fast_io {

static constexpr U32 BUFFER_SIZE = 1U << 20; // 1 MB
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

/* ------------------------------- INPUT API -------------------------------- */

inline void load_input() {
#if CP_FAST_IO_VARIANT == 1
  // memmove + sentinel: preserves tokens spanning a refill boundary.
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
  // Full reload: assumes any single token fits within BUFFER_SIZE.
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

inline void flush_output() {
  if (output_pos == 0)
    return;
  std::fwrite(output_buffer, 1, output_pos, stdout);
  output_pos = 0;
}

inline void read_char(char& c) {
  do {
    c = next_input_char();
  } while (c <= ' ' && c != 0);
}

// Accumulates unsigned: an out-of-range token wraps instead of being UB.
template <typename T>
inline void read_integer(T& x) {
  using U = cp::make_unsigned_t<T>;
  char c;
  do {
    c = next_input_char();
  } while (c <= ' ' && c != 0);

  bool negative = false;
  if constexpr (cp::Signed<T>) {
    if (c == '-') {
      negative = true;
      c = next_input_char();
    }
  }

  U value = 0;
  while (c > ' ') {
    value = as<U>(value * 10 + as<U>(c - '0'));
    c = next_input_char();
  }

  if constexpr (cp::Signed<T>) {
    x = as<T>(negative ? as<U>(U(0) - value) : value);
  } else {
    x = as<T>(value);
  }
}

inline void read_string(std::string& s) {
  s.clear();
  s.reserve(32);
  char c;
  do {
    c = next_input_char();
  } while (c <= ' ' && c != 0);

  while (c > ' ') {
    s.push_back(c);
    c = next_input_char();
  }
}

template <typename T>
inline void read_floating(T& x) {
  char token[64];
  U32 len = 0;
  char c;
  do {
    c = next_input_char();
  } while (c <= ' ' && c != 0);
  while (c > ' ' && len + 1 < sizeof(token)) {
    token[len++] = c;
    c = next_input_char();
  }
  token[len] = '\0';

  char* end = nullptr;
  if constexpr (cp::Same<T, F32>)
    x = std::strtof(token, &end);
  else if constexpr (cp::Same<T, F64>)
    x = std::strtod(token, &end);
  else
    x = std::strtold(token, &end);
}

template <FastIntegral T>
inline void read(T& x) { read_integer(x); }

template <FastFloating T>
inline void read(T& x) { read_floating(x); }
inline void read(char& x) { read_char(x); }
inline void read(std::string& x) { read_string(x); }

/* ------------------------------- OUTPUT API ------------------------------- */

template <typename T>
inline void write_integer(T x) {
  using UnsignedT = cp::make_unsigned_t<T>;
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
  U32 remaining = as<U32>(s.size());
  while (remaining > 0) {
    if (output_pos >= BUFFER_SIZE)
      flush_output();
    U32 space = BUFFER_SIZE - output_pos;
    U32 chunk = (remaining < space) ? remaining : space;
    std::memcpy(output_buffer + output_pos, data, chunk);
    output_pos += chunk;
    data += chunk;
    remaining -= chunk;
  }
}

#ifndef CP_FLOAT_PRECISION
  #define CP_FLOAT_PRECISION 10
#endif

// A large F80 can need thousands of digits; retry on the heap, never truncate.
template <typename T>
inline void write_floating(T x) {
  char local_buffer[512];
  int n = std::snprintf(local_buffer, sizeof(local_buffer), "%.*Lf", CP_FLOAT_PRECISION, as<F80>(x));
  if (n <= 0)
    return;
  if (as<Size>(n) < sizeof(local_buffer)) {
    write_string(std::string_view(local_buffer, as<Size>(n)));
    return;
  }

  std::string spill(as<Size>(n) + 1, '\0');
  n = std::snprintf(spill.data(), spill.size(), "%.*Lf", CP_FLOAT_PRECISION, as<F80>(x));
  if (n > 0)
    write_string(std::string_view(spill.data(), as<Size>(n)));
}

template <FastIntegral T>
inline void write(T x) { write_integer(x); }

template <FastFloating T>
inline void write(T x) { write_floating(x); }

inline void write(char x) { write_char(x); }
inline void write(const std::string& x) { write_string(x); }
inline void write(const char* x) { write_string(x); }

} // namespace fast_io

/* ------------------------------- EXTENSIONS ------------------------------- */

#if CP_FAST_IO_VARIANT == 1
  #ifndef CP_FAST_IO_ENABLE_MODINT
    #ifdef NEED_MOD_INT
      #define CP_FAST_IO_ENABLE_MODINT 1
    #else
      #define CP_FAST_IO_ENABLE_MODINT 0
    #endif
  #endif

  #if CP_FAST_IO_ENABLE_MODINT
    #include "templates/modules/Fast_IO_Ext_ModInt.hpp"
  #endif
#endif // CP_FAST_IO_VARIANT == 1

namespace fast_io {

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

// Single-arg fallbacks: error on types lacking a concrete overload.
template <class T>
  requires(!FastIntegral<T> && !FastFloating<T>)
void read(T&) = delete;

template <class T>
  requires(!FastIntegral<T> && !FastFloating<T>)
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
