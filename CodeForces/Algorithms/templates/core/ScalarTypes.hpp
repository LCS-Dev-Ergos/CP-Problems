#pragma once
#ifndef CP_TYPES_NO_PREAMBLE
  #include "templates/core/Preamble.hpp"
#endif
#include "templates/core/FeatureDetect.hpp"

//===----------------------------------------------------------------------===//
/* Scalar Type Aliases */

using I8  = std::int8_t;
using I16 = std::int16_t;
using I32 = std::int32_t;
using I64 = std::int64_t;
using U8  = std::uint8_t;
using U16 = std::uint16_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;

#if HAS_INT128
  __extension__ using I128 = __int128;
  __extension__ using U128 = unsigned __int128;
#else
  using I128 = std::int64_t;
  using U128 = std::uint64_t;
#endif

using F32 = float;
using F64 = double;
using F80 = long double;

#if HAS_FLOAT128
  using F128 = __float128;
#else
  using F128 = long double;
#endif

template <typename T>
using Limits = std::numeric_limits<T>;

using Size = std::size_t;
using Diff = std::ptrdiff_t;
using Byte = std::byte;
