#pragma once
#include "templates/Base_profiles.hpp"
#include "templates/core/Preamble.hpp"
#include "templates/Base_contracts.hpp"

#if CP_USE_ADVANCED
  #include "templates/advanced/Cast.hpp"
  #include "templates/advanced/RangeStreamConcepts.hpp"
  #include "templates/advanced/Strong_Type.hpp"
#endif

#include "templates/Base_features.hpp"

#if CP_USE_GLOBAL_STD_NAMESPACE
using namespace std;
#endif
