# Paths And Config

## What this covers
This file explains path handling, filesystem helpers, executable/home/config discovery, and Core's config-file placement rules on Windows and POSIX.

## Path abstraction
[`uppsrc/Core/Core.h`](../../../uppsrc/Core/Core.h) sets `DIR_SEP`, `DIR_SEPS`, and `PLATFORM_PATH_HAS_CASE` per platform. [`uppsrc/Core/Path.h`](../../../uppsrc/Core/Path.h) and [`uppsrc/Core/Path.cpp`](../../../uppsrc/Core/Path.cpp) then build the public helpers on top:

- path decomposition: `GetFileDirectory`, `GetFileFolder`, `GetFileTitle`, `GetFileExt`, `GetFileName`
- path construction: `AppendFileName`, `AppendExt`, `ForceExt`, `NativePath`, `UnixPath`, `WinPath`
- normalization and comparison: `NormalizePath`, `NormalizeUnixPath`, `PathIsEqual`
- directory/file operations: `FileCopy`, `FileMove`, `FileDelete`, `DirectoryCreate`, `RealizePath`
- enumeration: `FindFile`, `FindAllPaths`

`PathIsEqual` is platform-aware: case-insensitive on Windows after normalization, case-sensitive on POSIX.

## Home and executable location
[`uppsrc/Core/App.cpp`](../../../uppsrc/Core/App.cpp) supplies the environment layer:

- `GetHomeDirectory()` reads `HOMEDRIVE` + `HOMEPATH` on Windows and `$HOME` on POSIX
- `GetExeFilePath()` uses the module filename on Windows and `/proc/.../exe` or platform equivalents on POSIX, with PATH-based fallback if needed
- `GetExeDirFile()` and `GetExeFolder()` derive sibling locations from the executable

That means "home directory" and "config directory" are separate concepts in Core.

## Config placement
The main APIs are:

- `SetConfigDirectory`
- `SetConfigName`
- `SetConfigGroup`
- `UseHomeDirectoryConfig`
- `ConfigFile(const char*)`
- `GetConfigFolder()`

### Windows
Default behavior is portable-app style:

- `ConfigFile("x")` resolves under the executable directory

If `UseHomeDirectoryConfig(true)` is enabled, Core switches to a per-app directory under `GetHomeDirectory()`.

### POSIX
The behavior is more layered:

1. if `SetConfigDirectory` was used, Core uses it directly
2. otherwise it tries to find a writable `.config` directory while walking upward from the executable folder, unless `UseHomeDirectoryConfig(true)` disabled that search
3. if that fails, it uses `XDG_CONFIG_HOME`
4. if that is missing or nonexistent, it falls back to `~/.config`
5. it then appends the config group, defaulting to `u++`
6. it appends the config name or app name

This is explicitly visible in `GetUserConfigDir()` and `ConfigFile()` in `App.cpp`.

## Portable-app implications
On POSIX, the upward search for a writable `.config` near the executable is a real code path. That is a portable/deployed-bundle assumption, not just an abstract concept. On Windows, the default executable-relative config path is even more direct.

## INI layer
[`uppsrc/Core/Ini.cpp`](../../../uppsrc/Core/Ini.cpp) builds an INI-like configuration reader on top of `ConfigFile("q.ini")`, with support for:

- `@include`
- optional environment-variable expansion
- fallback locations such as `GetExeDirFile("q.ini")` on Windows and `GetHomeDirFile("q.ini")` on POSIX

## See also
- [01-Architecture.md](01-Architecture.md)
- [06-Streams.md](06-Streams.md)
- [07-Logging.md](07-Logging.md)
- [12-Time.md](12-Time.md)
