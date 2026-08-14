#pragma once
#include "templates/core/FeatureDetect.hpp"
#include "templates/core/Contracts.hpp"

//=====----- [ Debug ] --------------------------------------------------=====//

#ifdef LOCAL
  #include "debug.h"
#else
  #define debug(...)
  #define debug_if(...)
  #define debug_tree(...)
  #define debug_tree_verbose(...)
  #define debug_line()
  #define my_assert(cond) ((void)0)
  #define COUNT_CALLS(...)
#endif
