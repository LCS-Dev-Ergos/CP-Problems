#pragma once
#include "templates/core/Config_defaults.hpp"

//===----------------------------------------------------------------------===//
/* Toolchain-Driven Config Corrections */

// <bits/stdc++.h> is libstdc++-only; fall back to PortableStdHeaders on libc++.
#if CP_USE_BITS_HEADER && !__has_include(<bits/stdc++.h>)
  #undef CP_USE_BITS_HEADER
  #define CP_USE_BITS_HEADER 0
#endif
