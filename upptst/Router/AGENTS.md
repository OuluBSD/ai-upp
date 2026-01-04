AGENTS

Scope
- Applies to `upptst/Router`.

Purpose
- Hosts focused console tests that exercise router descriptors, serialization helpers, and metadata plumbing.
- Provides an early warning harness whenever `RouterPortDesc` / `RouterConnectionDesc` schemas evolve.

Guidelines
- Depend on `EonRouterSupport` for helper scaffolding; keep additional dependencies minimal.
- Add new tests whenever descriptor metadata (vd payload, optional flags, flow-control hints) gains new fields.
- Tests should log via `StdLogSetup` and exit with nonzero status when assertions fail.
