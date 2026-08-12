#pragma once

//===----------------------------------------------------------------------===//
/* Runtime Contracts */

namespace cp::detail {

[[noreturn]] [[gnu::cold]] inline void contract_violation() noexcept { __builtin_trap(); }

[[gnu::always_inline]] constexpr void expect(bool condition) noexcept {
  if (!condition) [[unlikely]]
    contract_violation();
}

} // namespace cp::detail

#define CP_EXPECT(condition, ...) cp::detail::expect(bool(condition))
