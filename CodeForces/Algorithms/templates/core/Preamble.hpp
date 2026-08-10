#pragma once
#include "templates/core/Config.hpp"
#include "templates/core/Compiler.hpp"
#include "templates/core/StdHeaders.hpp"
#include "templates/core/FeatureDetect.hpp"
#include "templates/core/Debug.hpp"

#ifdef __clang__
  #pragma clang diagnostic pop
#endif

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif
