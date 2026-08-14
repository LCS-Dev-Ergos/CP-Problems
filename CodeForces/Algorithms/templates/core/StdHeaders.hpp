#pragma once
#include "templates/core/Config.hpp"

//=====----- [ STL ] ----------------------------------------------------=====//

// Config.hpp already downgraded the flag when <bits/stdc++.h> is unreachable.
#if CP_USE_BITS_HEADER
  #include <bits/stdc++.h>
#else
  #include "templates/core/PortableStdHeaders.hpp"
#endif
