Scope: uppsrc/dbg

Purpose
- Headless debugger CLI skeleton.
- Owns the first debugger command surface for future `dbg` work.
- Current role is registry/dispatch scaffolding, not a real debugger backend.

Current Commands
- `--help`
- `--backends`
- `--backend <name> --help`
- `--backend <name> run <program> [args...]`

Planned Backends
- `vs`
- `gdb`
- `lldb`

Build
- `bin/build.exe -m MSVS26x64 -j12 dbg`
- Binary: `bin/dbg.exe`

Debugger Toolchain Notes
- Future backend smoke tests should use a small crashing test executable and verify that each backend
  can produce an automatic call stack.
- On this Windows workstation, LLVM is installed and `lldb.exe` is expected under
  `C:\Program Files\LLVM\bin`.
- LLDB may fail if it loads the wrong Python runtime. Python 3.11 is installed at `C:\Python311`;
  prepend `C:\Python311` and, if needed, `C:\Python311\Scripts` to `PATH` before running LLDB smoke
  tests.
- Do not hard-code this workstation path in production code. Keep it as a local test/setup note until
  backend configuration is formalized.
- `gdb.exe` availability should be detected in the test environment before enabling GDB smoke tests.

Package Notes
- `TcpProxy.*` are legacy MCP proxy files.
- They are not part of `dbg.upp` and must stay out of the package manifest unless the package role changes again.

File Map
- `dbg.h` : umbrella header
- `Backend.h/cpp` : backend registry, planned backend metadata, CLI dispatch
- `main.cpp` : console entrypoint, delegates to `RunDbgCli`
