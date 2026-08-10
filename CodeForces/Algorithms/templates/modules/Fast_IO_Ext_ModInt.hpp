#pragma once
#include "templates/modules/Fast_IO_Fwd.hpp"
#include "templates/modules/Mod_Int.hpp"

///@brief Optional Fast I/O extension for ModInt.
namespace fast_io {

template <I64 MOD>
inline void read(::ModInt<MOD>& x) {
  I64 value;
  read_integer(value);
  x = ::ModInt<MOD>(value);
}

template <I64 MOD>
inline void write(const ::ModInt<MOD>& x) { write_integer(I64(x)); }

template <U32 Id>
inline void read(::DynModInt<Id>& x) {
  I64 value;
  read_integer(value);
  x = ::DynModInt<Id>(value);
}

template <U32 Id>
inline void write(const ::DynModInt<Id>& x) {
  write_integer(I64(x));
}

} // namespace fast_io
