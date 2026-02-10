# Task: Memory Leak Hunting & Fix
# Status: DONE

## Objective
Investigate and fix the heap memory leaks detected in `NetContext` and `ChainContext` teardown during `Eon03` execution.

## Summary of Fixes
- Fixed `SemanticParser` to handle combined `TK_RETURN_HINT` (->) tokens in net connections.
- Implemented `Uninitialize()` in `BufferT` and `BufferStageT` to explicitly call `ClearTex()` and free heap images.
- Integrated `BufferT::Uninitialize()` into `GfxAccelAtom::Uninitialize()`.
- Fixed missing `accel_zbuf.Clear()` in `ScrX11Sw::SinkDevice_Uninitialize`.
- Resolved `undefined-var-template` warning for `GfxAccelAtom::latest`.
- Verified clean exit in `Eon00` and `Eon03` scenarios using U++ heap debugger.
