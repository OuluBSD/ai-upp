# PacketRouter Phase 4 – Runtime Atom Conversions

## Goal
Gradually rewire backend atoms so they emit packets through the router instead of the legacy Link/Exchange paths. This builds on the DSL/net work from earlier phases and ensures `NetContext::ProcessFrame()` can drive actual workloads without falling back to old loops.

## Progress
- `CustomerBase::Send()` now populates its packet via `ForwardPacket()` and calls `AtomBase::EmitViaRouter()` when a `PacketRouter` is registered. Legacy loops remain unaffected because the router path is guarded by `packet_router` being present.
- The new code keeps the `PacketValue` content intact for downstream consumers by moving it into a temporary `Packet`, routing it, and then restoring it so the existing LinkSystem logic still observes the same `out` data.
- Camera, media-file, image, and SynSoft audio generators now register ports via the default `AtomBase::RegisterPorts`, request credits before emitting, and call `EmitViaRouter()`/`AckCredits()` so router-based nets drive the actual workloads alongside the legacy Link path. The V4L2/OpenCV camera now returns `false` when credits are denied so router diagnostics stay accurate.
- SynFluidsynth, SynFmSynth, SynCoreSynth, and SynCoreDrummer now request credits, emit packets via `EmitViaRouter()`, and acknowledge those credits after routing so the broader synth line works through PacketRouter-managed flow control.
- `MidiFileReaderAtom` now requests credits, routes MIDI packets via `EmitViaRouter()`, acknowledges the router before restoring the `PacketValue`, and returns the router result so metadata-driven flow control governs MIDI file playback in router nets.
- `SynLV2::Instrument_Send` now reads the LV2 host buffers, mixes each output port into the packet payload, and runs the resulting audio through `EmitViaRouter()`/`AckCredits()` so LV2 hosts share the same metadata-driven flow control as the other synth sources.
- `HalSdl::EventsBase_Send` now requests credits and emits SDL event packets via `EmitViaRouter()` before clearing the sendable flag, keeping the SDL event bridge diagnostics aligned with the router metadata and preventing events from bubbling up without honoring the credit helpers.
- Volumetric static sources (e.g., `VolRawByte`) and the PortMidi hardware source now request router credits and emit their packets via `EmitViaRouter()`, which ensures the router metadata reflects those workloads as soon as the data leaves the hardware.

## Next
- Continue migrating the remaining SDL-backed audio/video bridges and any other hardware synth hosts so each emission honors router credits, keeping router diagnostics/VFS metadata in lock-step with the live workloads as more sources adopt the router path.
