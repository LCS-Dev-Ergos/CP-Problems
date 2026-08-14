#pragma once

#if !defined(CP_IO_IMPL_READ) || !defined(CP_IO_IMPL_WRITELN) || !defined(CP_IO_IMPL_FLUSH)
  #error "No I/O backend selected: define CP_IO_PROFILE_SIMPLE / CP_IO_PROFILE_FAST_MINIMAL / CP_IO_PROFILE_FAST_EXTENDED \
          (or a NEED_IO / NEED_FAST_IO macro) before including templates/Base.hpp."
#endif

#if CP_ENABLE_LEGACY_IO_VEC_MACROS
  #include "templates/core/ContainerAliases.hpp"
#endif

//=====----- [ I/O Macros ] ---------------------------------------------=====//

#define IN(...) CP_IO_IMPL_READ(__VA_ARGS__)
#define OUT(...) CP_IO_IMPL_WRITELN(__VA_ARGS__)
#define FLUSH() CP_IO_IMPL_FLUSH()

#ifndef CP_IO_DECL_MACROS_DEFINED
  #define CP_IO_DECL_MACROS_DEFINED 1
  #define INT(...) I32 __VA_ARGS__; IN(__VA_ARGS__)
  #define LL(...) I64 __VA_ARGS__; IN(__VA_ARGS__)
  #define ULL(...) U64 __VA_ARGS__; IN(__VA_ARGS__)
  #define STR(...) std::string __VA_ARGS__; IN(__VA_ARGS__)
  #define CHR(...) char __VA_ARGS__; IN(__VA_ARGS__)
  #define DBL(...) F64 __VA_ARGS__; IN(__VA_ARGS__)

  #if CP_ENABLE_LEGACY_IO_VEC_MACROS
    #define VEC(type, name, size) Vec<type> name(size); IN(name)
    #define VV(type, name, h, w) Vec2D<type> name(h, Vec<type>(w)); IN(name)
  #endif
#endif

#ifndef CP_IO_ANSWER_HELPERS_DEFINED
  #define CP_IO_ANSWER_HELPERS_DEFINED 1
  inline void YES(bool condition = true) {
    if (condition) OUT("YES");
    else OUT("NO");
  }
  inline void NO(bool condition = true) { YES(!condition); }
  inline void Yes(bool condition = true) {
    if (condition) OUT("Yes");
    else OUT("No");
  }
  inline void No(bool condition = true) { Yes(!condition); }
#endif
