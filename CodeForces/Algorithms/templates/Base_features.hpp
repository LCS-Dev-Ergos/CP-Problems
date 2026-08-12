#pragma once

// Generated from profiles.toml. Do not edit by hand.

#ifdef NEED_PBDS
  #include "templates/modules/PBDS.hpp"
#endif

#ifdef NEED_CORE
  #include "templates/DefaultToolkit.hpp"
#endif

#ifdef NEED_DEFAULT_TOOLKIT
  #include "templates/DefaultToolkit.hpp"
#endif

#ifdef NEED_SCALAR_TYPES
  #include "templates/core/ScalarTypes.hpp"
#endif

#ifdef NEED_TYPES
  #include "templates/core/Types.hpp"
#endif

#ifdef NEED_CONSTANTS
  #include "templates/core/Constants.hpp"
#endif

#ifdef NEED_MACROS
  #include "templates/core/Macros.hpp"
#endif

#ifdef NEED_MATH
  #include "templates/core/MinMax.hpp"
  #include "templates/modules/Random.hpp"
  #include "templates/modules/IntegerMath.hpp"
#endif

#ifdef NEED_INTEGER_MATH
  #include "templates/modules/IntegerMath.hpp"
#endif

#ifdef NEED_MINMAX
  #include "templates/core/MinMax.hpp"
#endif

#ifdef NEED_RANDOM
  #include "templates/modules/Random.hpp"
#endif

#ifdef NEED_TIMER
  #include "templates/core/Timer.hpp"
#endif

#ifdef NEED_HASHING
  #include "templates/modules/Hashing.hpp"
#endif

#ifdef NEED_FAST_IO
  #include "templates/modules/Fast_IO.hpp"
#endif

#ifdef NEED_IO
  #include "templates/modules/IO.hpp"
#endif

#ifdef NEED_BIT_OPS
  #include "templates/modules/Bit_Ops.hpp"
#endif

#ifdef NEED_RANGE_REDUCERS
  #include "templates/modules/RangeReducers.hpp"
#endif

#ifdef NEED_MOD_INT
  #include "templates/modules/Mod_Int.hpp"
#endif

#ifdef NEED_CONTAINERS
  #include "templates/modules/ContainerAlgorithms.hpp"
  #include "templates/modules/NdVec.hpp"
  #include "templates/modules/Search.hpp"
#endif

#ifdef NEED_CONTAINER_ALGORITHMS
  #include "templates/modules/ContainerAlgorithms.hpp"
#endif

#ifdef NEED_NDVEC
  #include "templates/modules/NdVec.hpp"
#endif

#ifdef NEED_SEARCH
  #include "templates/modules/Search.hpp"
#endif
