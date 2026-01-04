AGENTS

Scope
- Applies to `upptst/EonRouterSupport` (shared test support for router spikes).

Purpose
- Hosts `RouterNetContext` + helper utilities so multiple `upptst/Eon*` packages can run router-style method 3 harnesses without duplicating code.
- Mirrors the loop builder contract used by `Eon00` and upcoming router pilots, enabling shared logging/side-link validation.

Usage
- Include `<EonRouterSupport/EonRouterSupport.h>` from test packages that need router prototypes.
- Keep router docs/task notes updated when helpers change; other packages rely on this behavior matching the descriptions in `task/PacketRouter.md`.
