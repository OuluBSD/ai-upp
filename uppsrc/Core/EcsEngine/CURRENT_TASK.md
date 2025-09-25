CURRENT TASK

Focus
- Relocate ECS runtime (`Engine*`, `Realtime`, `LinkSystem`, `Verifier`, etc.) out of `Core2`.

Next Steps
- Move headers and implementations into this package and adjust include paths to use `EcsEngine.h`.
- Keep INITBLOCK registrations in sync and document lifecycle expectations once migration is complete.
- Verify threadsafety assumptions after relocation (especially `RunningFlagSingle` usage).

Notes
- Coordinate with `Core/EcsDataflow` for data-plane structures; avoid circular dependencies by including only the necessary headers.
