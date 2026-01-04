AGENTS

Scope
- Applies to `upptst/RouterPool`.

Purpose
- Reproduces router credit/packet-pool edge cases so RouterNetContext metadata mirrors the legacy loop quotas.
- Validates that flow-control metadata survives Store/Load helpers when credits are throttled or when senders idle.

Guidelines
- Focus on tiny test atoms and deterministic ValueMap assertions.
- Add new scenarios per pool policy (idle credit reclaim, burst caps, etc.) as router behavior evolves.
- Keep console logs minimal; prefer ASSERT failure text for clarity.
