AGENTS

Scope
- Applies to `upptst/DbgCrashSmoke`.

Purpose
- Debugger backend smoke executable that intentionally crashes for future call-stack validation.

Rules
- Keep this package headless and console-only.
- The default no-argument path and `--crash` must crash deterministically.
- `--help` and `--no-crash` must exit with status 0.
- This package is for backend smoke validation later; `dbg` does not launch it yet.
