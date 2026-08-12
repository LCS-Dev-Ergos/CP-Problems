#pragma once

// Generated from profiles.toml. Do not edit by hand.

#if CP_USE_ADVANCED != 0 && CP_USE_ADVANCED != 1
  #error "CP_USE_ADVANCED must be 0 or 1."
#endif
#if CP_FAST_IO_VARIANT != 0 && CP_FAST_IO_VARIANT != 1
  #error "CP_FAST_IO_VARIANT must be 0 (minimal) or 1 (refill)."
#endif
#if CP_FLOAT_PRECISION < 0
  #error "CP_FLOAT_PRECISION must be non-negative."
#endif
#if CP_FAST_IO_MAX_TOKEN_SIZE <= 0
  #error "CP_FAST_IO_MAX_TOKEN_SIZE must be positive."
#endif
