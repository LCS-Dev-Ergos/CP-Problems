#pragma once

//=====----- [ Features ] -----------------------------------------------=====//

#ifndef HAS_INT128
  #ifdef __SIZEOF_INT128__
    #define HAS_INT128 1
  #else
    #define HAS_INT128 0
  #endif
#endif

// __SIZEOF_FLOAT128__ is GCC's spelling, __FLOAT128__ is Clang's.
#ifndef HAS_FLOAT128
  #if defined(__SIZEOF_FLOAT128__) || defined(__FLOAT128__)
    #define HAS_FLOAT128 1
  #else
    #define HAS_FLOAT128 0
  #endif
#endif

#if defined(__has_include) && __has_include(<ext/pb_ds/assoc_container.hpp>) \
 && __has_include(<ext/pb_ds/tree_policy.hpp>)
 #ifndef PBDS_AVAILABLE
  #define PBDS_AVAILABLE 1
#endif
#elif !defined(PBDS_AVAILABLE)
  #define PBDS_AVAILABLE 0
#endif
