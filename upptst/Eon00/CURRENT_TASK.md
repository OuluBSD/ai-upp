# Eon00 – Current Task

## Packet Router Prototype
- Provide a local `RouterNetContext` spike inside `00a_audio_gen.cpp` to mimic router-style APIs before the actual runtime exists.
- `method=3` now maps router atoms/ports/connections into the existing `ChainContext` machinery so we can capture logs + failures without touching the parser flow.
- Use this harness to explore credit metadata, named ports, and connection validation before migrating other tests (`00b/00c`).
- Router helper moved into `upptst/EonRouterSupport` so other `upptst/Eon*` packages (starting with `Eon02`) can reuse the same `BuildRouterChain` plumbing for their method 3 runs.
- Follow-ups: share the helper across tests, surface router topology in test output, and align with the real PacketRouter core once ready.
