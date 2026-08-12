# I/O Profile Policy

This project supports explicit I/O profiles defined in `templates/profiles.toml`
and materialized into generated headers. `Base_profiles.hpp` expands profile
macros into `NEED_*` defines; `Base_features.hpp` maps those `NEED_*` defines
to concrete template headers.

## Profiles

### `CP_IO_PROFILE_SIMPLE`

Expands to:

```cpp
#define NEED_IO
```

Pulls in: `modules/IO.hpp` (lightweight stream-based I/O helpers).

Use when standard `cin`/`cout` is adequate and you want the minimum overhead.

```cpp
#define CP_IO_PROFILE_SIMPLE
#include "templates/Base.hpp"
```

---

### `CP_IO_PROFILE_FAST_MINIMAL`

Expands to:

```cpp
#define NEED_FAST_IO
#define CP_FAST_IO_VARIANT 0
```

Pulls in: `modules/Fast_IO.hpp` compiled in variant 0. Tokens are validated and
bounded by `CP_FAST_IO_MAX_TOKEN_SIZE` (16 MiB by default).

Use when throughput matters and modular/strong types are not needed.

```cpp
#define CP_IO_PROFILE_FAST_MINIMAL
#include "templates/Base.hpp"
```

---

### `CP_IO_PROFILE_FAST_EXTENDED`

Expands to:

```cpp
#define NEED_FAST_IO
#define NEED_MOD_INT
#define CP_USE_ADVANCED 1
```

Pulls in: `modules/Fast_IO.hpp`, `modules/Mod_Int.hpp`, and the advanced layer
(`advanced/Cast.hpp`, `advanced/RangeStreamConcepts.hpp`,
`advanced/Strong_Type.hpp`).

Use for advanced rounds where both fast I/O and modular arithmetic with strong
typing are in scope.

```cpp
#define CP_IO_PROFILE_FAST_EXTENDED
#include "templates/Base.hpp"
```

---

Only one `CP_IO_PROFILE_*` macro may be defined in a single source file. The
preprocessor emits an `#error` if more than one is present.

## Direct NEED_* usage

Profiles are convenience shortcuts. Direct `NEED_*` defines remain fully
supported and are preferable when you need a non-standard combination:

```cpp
#define NEED_FAST_IO
#define NEED_CORE
#define NEED_HASHING
#include "templates/Base.hpp"
```

In relaxed mode, defining both `NEED_IO` and `NEED_FAST_IO` drops `NEED_IO`.
With `CP_TEMPLATE_PROFILE_STRICT` or `CP_STRICT_TEMPLATE_NEEDS`, the same
collision is a compile-time error.

## Fast_IO Customization

`Fast_IO.hpp` uses constrained overloads to recognize types exposing the
`cp_modint_marker` or `cp_strong_type_marker` protocol. There are no secondary
extension toggles, extension headers, or include-order dependencies.

## Relationship to Base.hpp

`Base.hpp` includes `Base_profiles.hpp` first, which expands the active profile
into its constituent `NEED_*` defines and normalizes backend collisions. The
generated `Base_contracts.hpp` validates public switches, then
`Base_features.hpp` maps each feature to one entry header. Transitive
dependencies come from the real C++ include graph rather than duplicated TOML
closure lists.

Do not define `NEED_*` macros and a profile macro simultaneously unless you
intend the union of both effects.
