//===----------------------------------------------------------------------===//
/**
 * @file: __FILE_NAME__
 * @brief __PROBLEM_BRIEF__
 * @author: C.L.
 *
 * @status: In Progress
 */
//===----------------------------------------------------------------------===//

#ifndef CP_TEMPLATE_PROFILE_STRICT
  #define CP_TEMPLATE_PROFILE_STRICT
#endif
#ifndef CP_USE_GLOBAL_STD_NAMESPACE
  #define CP_USE_GLOBAL_STD_NAMESPACE 1
#endif

#ifndef CP_USE_ADVANCED
  #define CP_USE_ADVANCED 1
#endif

#define NEED_BIT_OPS
#define NEED_CONTAINERS
#define NEED_MACROS
#define NEED_PBDS
#define NEED_TIMER
#define NEED_TYPES
#define CP_IO_PROFILE_FAST_EXTENDED

#include "templates/Base.hpp"

//=====----- [ Solve ] --------------------------------------------------=====//

void solve() {
  // Optimized solution here
}

//=====----- [ Main ] ---------------------------------------------------=====//

auto main() -> int {
#ifdef LOCAL
  Stopwatch timer;
#endif

  INT(T);
  FOR(T) solve();

  return 0;
}
