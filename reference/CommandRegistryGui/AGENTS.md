# AGENTS - reference/CommandRegistryGui

- This package is a GUI wrapper over `bin/CommandRegistry.exe`.
- Keep command discovery and execution process-backed; do not link command implementations here.
- Put shared process/JSON logic in non-`Ctrl` code so GUI and headless smoke modes use the same path.
- Use stdout JSON for headless diagnostics and GUI process communication.
