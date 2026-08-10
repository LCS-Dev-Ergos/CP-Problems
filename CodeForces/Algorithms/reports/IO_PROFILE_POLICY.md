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

Pulls in: `modules/Fast_IO.hpp` compiled in variant 0 — full-reload input (a
single token must fit in the 1 MB buffer) and no extension hooks.

Use when throughput matters but the compile-time checking baggage for ModInt
and StrongType extensions is not needed.

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
#define CP_FAST_IO_ENABLE_MODINT   1
#define CP_FAST_IO_ENABLE_STRONG_TYPE 1
#define CP_USE_ADVANCED            1
```

Pulls in: `modules/Fast_IO.hpp`, `modules/Mod_Int.hpp`, and the advanced layer
(`advanced/Cast.hpp`, `advanced/Concepts.hpp`, `advanced/Strong_Type.hpp`).

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

Defining both `NEED_IO` and `NEED_FAST_IO` simultaneously is handled
gracefully: `Base_profiles.hpp` automatically drops `NEED_IO` when the fast
backend is present.

## Fast_IO extension toggles

These can be set independently of any profile:

| Macro                           | Default                            | Requirement                |
| ------------------------------- | ---------------------------------- | -------------------------- |
| `CP_FAST_IO_ENABLE_MODINT`      | `1` when `NEED_MOD_INT` is defined | `CP_FAST_IO_VARIANT == 1`  |
| `CP_FAST_IO_ENABLE_STRONG_TYPE` | `0`                                | requires `CP_USE_ADVANCED` |

`CP_FAST_IO_ENABLE_STRONG_TYPE` is consumed by `Base.hpp`, not by
`modules/Fast_IO.hpp`: the extension lives in `advanced/` and only the top-level
header may include that layer.

Set them to `0` before the include to suppress a specific extension even when
its parent feature is active:

```cpp
#define CP_IO_PROFILE_FAST_EXTENDED
#define CP_FAST_IO_ENABLE_STRONG_TYPE 0   // disable strong-type I/O overloads
#include "templates/Base.hpp"
```

## Relationship to Base.hpp

`Base.hpp` includes `Base_profiles.hpp` first, which expands the active profile
into its constituent `NEED_*` defines and normalizes backend collisions. It then
includes `Base_features.hpp`, generated from the `[feature.NEED_*]` manifest in
`profiles.toml`, to process those defines in the standard conditional-include
chain.

Do not define `NEED_*` macros and a profile macro simultaneously unless you
intend the union of both effects.
