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

Package Notes
- `TcpProxy.*` are legacy MCP proxy files.
- They are not part of `dbg.upp` and must stay out of the package manifest unless the package role changes again.

File Map
- `dbg.h` : umbrella header
- `Backend.h/cpp` : backend registry, planned backend metadata, CLI dispatch
- `main.cpp` : console entrypoint, delegates to `RunDbgCli`
