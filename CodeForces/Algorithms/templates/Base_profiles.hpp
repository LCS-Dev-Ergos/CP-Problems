#pragma once

// Generated from profiles.toml. Do not edit by hand.

#if (defined(CP_IO_PROFILE_FAST_EXTENDED) + defined(CP_IO_PROFILE_FAST_MINIMAL) + defined(CP_IO_PROFILE_SIMPLE)) > 1
  #error "Choose at most one CP_IO_PROFILE_* macro."
#endif

#ifdef CP_IO_PROFILE_SIMPLE
  #ifndef NEED_IO
    #define NEED_IO
  #endif
#endif

#ifdef CP_IO_PROFILE_FAST_MINIMAL
  #ifndef NEED_FAST_IO
    #define NEED_FAST_IO
  #endif
  #ifndef CP_FAST_IO_VARIANT
    #define CP_FAST_IO_VARIANT 0
  #endif
#endif

#ifdef CP_IO_PROFILE_FAST_EXTENDED
  #ifndef NEED_FAST_IO
    #define NEED_FAST_IO
  #endif
  #ifndef NEED_MOD_INT
    #define NEED_MOD_INT
  #endif
  #ifndef CP_FAST_IO_ENABLE_MODINT
    #define CP_FAST_IO_ENABLE_MODINT 1
  #endif
  #ifndef CP_FAST_IO_ENABLE_STRONG_TYPE
    #define CP_FAST_IO_ENABLE_STRONG_TYPE 1
  #endif
  #ifndef CP_USE_ADVANCED
    #define CP_USE_ADVANCED 1
  #endif
#endif

#if defined(NEED_IO) && (defined(NEED_FAST_IO))
  #undef NEED_IO
#endif
