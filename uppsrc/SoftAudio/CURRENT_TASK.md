# SoftAudio: Current Task — Graph-Based Engine (Non-GUI)

This file reflects what we are actively doing for `SoftAudio` now. See `uppsrc/SoftAudio/AGENTS.md` for background and conventions.

## Phase 1 — Interfaces and Minimal Engine (In Progress)

Status: scaffolding complete; first example chain wired; iterating on wrappers.

Done
- Created subpackage: `uppsrc/SoftAudio/Graph` with main header `Graph.h`.
- Implemented core pieces: `Node`, `PortSpec`, `ProcessContext`, `Edge`, and `Graph` with topological sort and per‑block processing.
- Added generic `GainNode` (package: `SoftAudio/Graph`).
- Added SoftAudio wrappers (package: `SoftAudio`):
  - `SineNode` (wraps `SineWave`) — mono source.
  - `FreeVerbNode` (wraps `FreeVerb`) — stereo effect.
  - `FileOutNode` (wraps `FileWaveOut`) — sink for offline renders.
- Added reference app `reference/SoftAudioGraph` with an example chain.

Example Chain (reference app)
- `SineNode → GainNode → FreeVerbNode → FileOutNode`
- Renders a short WAV file offline using a block pull loop.

Next
- Add `MixerNode` (N‑in stereo mix with per‑input gain/pan).
- Add `RouterNode` for mono↔stereo adapters and channel mapping.
- Wrap one more effect (e.g., `Compressor`) and one instrument (`Voicer`‑driven).
- Add minimal parameter API surface for wrappers (e.g., `SetParam(String id, float value)`).
- Validate clip handling and DC checks in the reference app.

Notes & Constraints
- Graph currently supports DAG only; cycles/feedback are rejected at compile time.
- Buffering: each node owns an output `Bus` sized at `block_size`; wrappers preallocate `AudioFrames` for bridging.
- RT safety: graph construction/compile can allocate; per‑block processing aims to be allocation‑free.

## Phase 2 — Wrappers and Basic Nodes (Queued)

Targets
- `MixerNode`, `RouterNode`, and a lightweight `BypassNode`.
- MIDI path (basic): a `MidiInputNode` feeding `Instrument::HandleEvent()` via a small event queue (block accuracy).

## Phase 3+ — Control, Runtime, Persistence (Planned)

- Sample‑accurate automation and feedback support via explicit `FeedbackNode` with enforced delay.
- Live output via PortAudio sink node and transport helpers.
- Graph serialization to a `ValueMap`/JSON‑like format.

## Open Questions

- Keep subpackage strictly independent of `SoftAudio` (current approach) vs. allow direct dependency for tighter `AudioFrames` integration?
- Default channel conventions (mono sources, stereo bus) — confirm and codify.


