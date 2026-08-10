#pragma once
#include "templates/core/Preamble.hpp"

//===----------------------------------------------------------------------===//
/* Modular Inclusions Based on Requirements */

#include "templates/Base_profiles.hpp"

// Type-safety utilities (concepts, strong types, cast wrappers) are an opt-in layer.
// Define CP_USE_ADVANCED to 1 before including this header to enable them (it is a
// 0/1 switch: leaving it undefined or setting it to 0 keeps the layer off). The
// legacy NEED_TYPE_SAFETY macro is treated as a synonym.
#if defined(NEED_TYPE_SAFETY) && !defined(CP_USE_ADVANCED)
  #define CP_USE_ADVANCED 1
#endif

#if CP_USE_ADVANCED
  #include "templates/advanced/Cast.hpp"
  #include "templates/advanced/Concepts.hpp"
  #include "templates/advanced/Strong_Type.hpp"
  // Wired here, not in Fast_IO.hpp: only this layer may include advanced/.
  #if defined(CP_FAST_IO_ENABLE_STRONG_TYPE) && CP_FAST_IO_ENABLE_STRONG_TYPE
    #include "templates/advanced/Fast_IO_Ext_StrongType.hpp"
  #endif
#endif

#include "templates/Base_features.hpp"

// Namespace usage.
#if CP_USE_GLOBAL_STD_NAMESPACE
using namespace std;
#endif
