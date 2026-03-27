# Logging

## What this covers
This file explains the actual logging macros and log routing implemented in Core.

## Logging APIs
The main logging surface is in [`uppsrc/Core/Diag.h`](../../../uppsrc/Core/Diag.h) and [`uppsrc/Core/Log.cpp`](../../../uppsrc/Core/Log.cpp):

- `StdLogSetup`, `StdLog`, `GetStdLogPath`
- `UppLog`, `SetUppLog`
- `VppLog`, `SetVppLog`, `SetVppLogName`
- `LogOptions` such as `LOG_FILE`, `LOG_COUT`, `LOG_CERR`, `LOG_DBG`, `LOG_SYS`, `LOG_TIMESTAMP`, and `LOG_APPEND`

## Macro behavior
Actual macro behavior from `Diag.h`:

- `LOG(...)` exists only in `_DEBUG`; in release it compiles to nothing
- `RLOG(...)` always writes to `VppLog()`
- `DLOG(...)` works only in debug-style builds and intentionally becomes invalid syntax in release
- `LOGBLOCK`, `RLOGBLOCK`, `DUMP`, `RDUMP`, `TIMING`, and related helpers build on the same stream

That means the common "safe in release" macro is `RLOG`, not `LOG`, if you are describing Core itself.

## Default log destination
`Log.cpp` computes the default file path lazily.

### POSIX
`SyncLogPath__()` builds:

- `GetFileFolder(GetUserConfigDir()) + "/.local/state/" + GetConfigGroup() + "/log/" + GetAppName()`
- then `sLogFile()` appends `".log"`

In the common home-directory case, that becomes something like:

- `~/.local/state/u++/log/<AppName>.log`

If config was redirected to a writable `.config` near the executable, the log tree becomes a sibling `.local/state/...` near that config root instead.

### Windows
The default log file path is derived from the config path through the Windows branch in `Log.cpp`. The public API is the same, but the implementation uses Win32 file handles and `OutputDebugStringA` for `LOG_DBG`.

## Output options
`LogOut::Line()` can send the same line to multiple sinks:

- log file
- stdout or stderr
- debug console
- syslog on POSIX

Timestamps and elapsed-time prefixes are optional flags, not hardwired behavior.

## Scope
Logging is central and current. The macros also carry historical baggage, especially the split between `LOG`, `RLOG`, and `DLOG`.

## See also
- [02-Memory-and-Performance.md](02-Memory-and-Performance.md)
- [08-Profiling.md](08-Profiling.md)
- [05-Paths-and-Config.md](05-Paths-and-Config.md)
- [README.md](README.md)
