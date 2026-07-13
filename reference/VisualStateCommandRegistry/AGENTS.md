# AGENTS - reference/VisualStateCommandRegistry

- This package registers VisualStateModel pilot commands through `CoreCommandRegistry`.
- Keep VSM behavior process-backed unless shared library entrypoints already exist.
- Do not duplicate recognition, comparison, or pipeline logic from existing VSM reference executables.
- Register commands in `.icpp` with `INITBLOCK`.
- Use `CoreCommandRegistryMain()` as the only console protocol implementation.
