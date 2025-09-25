CURRENT TASK

Focus
- Stage out legacy VFS helpers from `Core2` into this package without breaking consumers.
- Document how each migrated file maps onto the new `uppsrc/Vfs/*` packages.

Next Steps
- Move `VFS.*` and `Mount.*` into `Core/VfsBase` and update `Core2/Core.h` includes.
- Port `WorldState.*` (deriving from `BinaryWorldState`) and clarify its relationship to overlay subsets.
- Add unit coverage or smoke tests capturing existing semantics before further refactors.

Notes
- Keep compatibility wrappers minimal; long-term home for modern APIs is under `uppsrc/Vfs`.
