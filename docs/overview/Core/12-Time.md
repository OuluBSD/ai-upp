# Time

## What this covers
This file documents `Date`, `Time`, timezone helpers, UTC conversion helpers, and elapsed-time utilities in Core.

## `Date` and `Time`
[`uppsrc/Core/TimeDate.h`](../../../uppsrc/Core/TimeDate.h) defines the two main value types.

### `Date`
Storage:

- `byte day`
- `byte month`
- `int16 year`

Observed public range helpers:

- `Date::Low()` is `(-4000, 1, 1)`
- `Date::High()` is `(4000, 1, 1)`

Arithmetic is day-based:

- `Date - Date` returns day count
- `Date + int` and `Date - int` move by days
- helpers such as `AddMonths`, `AddYears`, `GetWeekDate`, and `EasterDay` sit on top

### `Time`
`Time` inherits from `Date` and adds:

- `hour`
- `minute`
- `second`

Arithmetic is second-based:

- `Time - Time` returns `int64` seconds
- `Time + int64` and `Time - int64` move by seconds

`Time::Low()` and `High()` use the same visible year bounds as `Date`.

## Local time, UTC, and file time
Core provides:

- `GetSysTime()`
- `GetUtcTime()`
- `SetSysTime()`
- `Time(FileTime)` and `AsFileTime()`
- `GetUTCSeconds()` and `TimeFromUTC()`

The implementation is platform-specific:

- Windows uses `FILETIME`, `SYSTEMTIME`, `GetLocalTime`, and `GetSystemTime`
- POSIX uses `time`, `gmtime`, `localtime`, and related libc APIs

## Timezones and leap seconds
`TimeDate.h` exposes:

- `GetTimeZone()`
- `GetTimeZoneText()`
- `ScanTimeZone()`
- `GetLeapSeconds(Date)`

This is more than a bare wall-clock wrapper, but it is still a relatively low-level utility layer. There is no full timezone database management in Core itself.

## Formatting and parsing
Core also includes:

- `Format(Date, ...)`
- `Format(Time, ...)`
- `ScanDate`, `ScanTime`
- mutable global/thread format settings through `SetDateFormat`, `SetDateScan`, `SetDateFilter`

`TimeDate.cpp` keeps separate main-thread and thread-local scan/format state, so formatting behavior is configurable without requiring every call site to pass an explicit formatter object.

## Elapsed-time helpers
For performance measurement rather than civil time, Core uses:

- `GetTickCount()`
- `msecs()`
- `usecs()`
- `TimeStop`
- profiling helpers described in [08-Profiling.md](08-Profiling.md)

## See also
- [04-Strings-and-Text.md](04-Strings-and-Text.md)
- [05-Paths-and-Config.md](05-Paths-and-Config.md)
- [08-Profiling.md](08-Profiling.md)
- [README.md](README.md)
