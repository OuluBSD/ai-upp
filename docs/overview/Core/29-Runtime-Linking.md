# Runtime Linking

## What this covers
This file documents Core's runtime dynamic-loading helpers in [`uppsrc/Core/dli.h`](../../../uppsrc/Core/dli.h) and [`uppsrc/Core/Dli.cpp`](../../../uppsrc/Core/Dli.cpp).

## Design intent
The `dli.h` macro layer generates small loader structs for optional shared-library dependencies.

The pattern is declarative:

- name a module
- list exported functions or optional functions
- call `Load()` / `Force()`

This is a compact runtime binding layer, not a general plugin discovery framework.

## Main behavior
The generated loader struct keeps:

- a library name string
- the native module handle
- function pointers populated at load time
- a `checked` flag so loading is attempted once until reset

`LoadDll__(...)` in [`uppsrc/Core/Dli.cpp`](../../../uppsrc/Core/Dli.cpp) accepts multiple candidate library names in one string:

- names are split by `;` on Windows
- names are split by `;` or `:` on POSIX

The first candidate whose required exports resolve successfully becomes the chosen library name.

## Export resolution
`dli.h` distinguishes required and optional symbols:

- `FN(...)` entries are required
- `FN0(...)` entries are optional and are prefixed with `?` internally

Platform-specific resolution differs:

- Windows uses `LoadLibrary` and `GetProcAddress`, with extra PE export-name matching logic for decorated names
- POSIX uses `dlopen(..., RTLD_LAZY | RTLD_GLOBAL)` and `dlsym`

## Limitations and caveats
- this is runtime symbol binding, not hot-reload orchestration
- `Force()` exits the process if loading fails
- the POSIX implementation logs missing required symbols, but the visible code does not increment its local `missing` counter before the final check, so docs should not overstate the strictness of that branch
- `flagUWP` disables the Windows `CheckDll__` path by returning `0`

## Current vs legacy
This is current support code. It is specialized and fairly low-level, but it remains part of the live package surface.

## See also
- [01-Architecture.md](01-Architecture.md)
- [30-Windows-Specific.md](30-Windows-Specific.md)
