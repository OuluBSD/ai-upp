# Windows Specific

## What this covers
This file documents the Windows-only support code exposed by [`uppsrc/Core/Win32Util.h`](../../../uppsrc/Core/Win32Util.h), [`uppsrc/Core/Win32Util.cpp`](../../../uppsrc/Core/Win32Util.cpp), and the UWP compatibility shims in [`uppsrc/Core/Uwp.h`](../../../uppsrc/Core/Uwp.h).

## Main helpers
The visible Win32 support layer includes:

- OS-version helpers such as `IsWin2K()`, `IsWinXP()`, `IsWinVista()`, `IsWin7()`, and `IsWin11()`
- registry read/write helpers
- `GetDllFn(...)` for ad hoc symbol lookup
- `Win32CreateProcess(...)`
- system-directory and module-file queries

This is an explicit platform utility layer, not just a few scattered `#ifdef`s.

## Version detection details
The code is mixed in age and strictness:

- `IsWin2K()` through `IsWin7()` use `GetVersionEx`
- `IsWin11()` uses `RtlGetVersion` from `ntdll` and treats build numbers greater than `22000` as Windows 11

So the Windows 11 check is practical rather than richly version-modeled.

## Registry and DLL utilities
The registry helpers wrap `RegOpenKeyEx`, `RegQueryValueEx`, `RegCreateKeyEx`, `RegSetValueEx`, and deletion helpers.

`GetDllFn(...)` is intentionally simple:

- `LoadLibrary(dll)`
- `GetProcAddress(fn)`

It does not keep a long-lived module wrapper here; it is just a convenience symbol fetch helper.

## UWP behavior
`flagUWP` materially changes behavior. [`uppsrc/Core/Uwp.h`](../../../uppsrc/Core/Uwp.h) replaces many Win32 APIs with stub inline functions.

Examples:

- `LoadLibrary(...)` returns `0`
- registry APIs return failure-like values
- `GetWindowsDirectoryW(...)` and `GetSystemDirectoryW(...)` return `0`

So the UWP branch is a compatibility shim that allows compilation while making many classic desktop Win32 facilities unavailable.

## Legacy and deprecated parts
Some compatibility branches are visibly old:

- `PLATFORM_WINCE` special cases remain in `Win32Util.h`
- `Win32Event` and `SyncObject` are guarded by `#ifdef DEPRECATED`

These are legacy compatibility remnants, not the preferred modern portability story for Core.

## Current vs legacy
The Win32 utility layer is current where the package still needs desktop Windows integration. The older OS checks and deprecated wrappers are retained for compatibility, and the UWP branch is intentionally reduced/stubbed.

## See also
- [05-Paths-and-Config.md](05-Paths-and-Config.md)
- [29-Runtime-Linking.md](29-Runtime-Linking.md)
