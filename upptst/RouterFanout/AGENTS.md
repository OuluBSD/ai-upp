AGENTS

Scope
- Applies to `upptst/RouterFanout`.

Purpose
- Hosts router fan-out regression tests that model atoms with multiple simultaneous source ports.
- Ensures RouterNetContext captures multi-port metadata and flow-control hints for these scenarios.

Guidelines
- Keep dependencies limited to Core/EonRouterSupport/Vfs serialization helpers.
- Prefer small, deterministic tests (`ASSERT`-driven) so failures flag the exact connection/metadata that regressed.
- Each scenario should log via `StdLogSetup` for parity with the main Router suite.
