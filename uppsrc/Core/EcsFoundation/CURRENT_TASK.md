CURRENT TASK

Focus
- Extract ECS metadata headers from `Core2` without breaking initialization macros or serialized IDs.

Next Steps
- Move `EcsDefs.h`, `Keys.h`, and type-class headers (`AtomType.*`, `LinkType.*`, `Interface.*`) here.
- Update `Core2/Core.h` to include `Core/EcsFoundation.h` and adjust dependent packages accordingly.
- Document registration flows once files are in place to ease contributor onboarding.

Notes
- Keep INITBLOCK registrations in `Core/CompatExt` until all consumers switch to the new headers.
