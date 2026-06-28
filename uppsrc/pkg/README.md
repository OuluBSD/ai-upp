# pkg

`pkg` is the ai-upp package manager console tool.

It reuses `ide/Core` for headless package and workspace parsing, and it keeps the first implementation focused on CLI shape, package scanning, target profiles, provider planning, and acceptflags auditing.

The `audit-acceptflags` command scans package source files for `flagNAME` usage, compares the result with `.upp` `acceptflags` declarations, and skips global/platform flags such as `GUI`, `DEBUG`, `RELEASE`, `POSIX`, `WIN32`, and compiler/platform macros.
