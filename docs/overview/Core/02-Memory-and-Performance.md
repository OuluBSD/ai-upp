# Memory And Performance

## What this covers
This file explains the allocator and low-level performance mechanisms in Core: heap selection, diagnostics, memory helpers, and architecture-specific fast paths.

## Heap model
Core exposes a stable memory API in [`uppsrc/Core/Heap.h`](../../../uppsrc/Core/Heap.h):

- `MemoryAlloc`, `MemoryFree`, `MemoryAllocSz`
- `MemoryAlloc32` and `MemoryFree32`
- `MemoryTryRealloc`
- diagnostics such as `MemoryCheck`, `MemoryDumpLeaks`, `MemoryProfile`

Whether that API uses the custom heap depends on build flags:

- if neither `flagUSEMALLOC` nor `flagSO` is set, `Defs.h` enables `UPP_HEAP`
- otherwise many functions inline directly to `malloc` and `free`

That distinction matters for both behavior and tooling. The custom heap provides size-class allocation, thread-local heap state, remote-free paths, and leak diagnostics. The malloc path preserves the API but loses most of those extras.

## Custom heap behavior
The implementation is split across `heap.cpp`, `sheap.cpp`, `lheap.cpp`, `hheap.cpp`, `HeapImp.h`, and `heapdbg.cpp`.

Observed characteristics from code:

- small allocations use heap-local fast paths in `sheap.cpp`
- larger blocks use separate logic in `lheap.cpp` and huge-block handling in `hheap.cpp`
- `HeapImp.h` shows per-thread heaps plus remote-free queues guarded by `StaticMutex`
- the allocator has explicit support for diagnostics, corruption checks, leak dumps, and allocation statistics

This is a performance-first allocator, but the code also spends significant effort on debug-time validation.

## Diagnostics
When `UPP_HEAP` and debugging are enabled, Core exposes:

- `MemoryInitDiagnostics`
- `MemoryBreakpoint`
- `MemoryDumpLeaks`
- `MemoryIgnoreLeaksBegin` / `MemoryIgnoreLeaksEnd`
- `PeakMemoryProfile`

In malloc mode these mostly collapse to stubs or "not available" behavior. For example, `MemoryProfile` stringification falls back to `"Using malloc - no memory profile available"` in [`uppsrc/Core/Diag.h`](../../../uppsrc/Core/Diag.h).

## Copy, compare, hash, and SIMD helpers
Low-level helpers are spread across:

- [`uppsrc/Core/Mem.h`](../../../uppsrc/Core/Mem.h) and [`uppsrc/Core/Mem.cpp`](../../../uppsrc/Core/Mem.cpp)
- [`uppsrc/Core/Hash.h`](../../../uppsrc/Core/Hash.h), `MD5.cpp`, `SHA1.cpp`, `SHA256.cpp`, `xxHsh.cpp`
- [`uppsrc/Core/SIMD_SSE2.h`](../../../uppsrc/Core/SIMD_SSE2.h), [`uppsrc/Core/SIMD_NEON.h`](../../../uppsrc/Core/SIMD_NEON.h), and `SIMD.cpp`

`Core.h` includes x86 intrinsics on x86 builds and exposes both SSE2 and NEON headers. That tells you the package is willing to use architecture-specific acceleration when it can still preserve one public API.

## Performance style
The package prefers specialized primitives over generic abstraction layers:

- custom containers instead of allocator-parameterized STL containers
- custom stream serialization instead of iostream formatting
- custom heap instead of always delegating to the system allocator
- explicit packed encodings in `Stream`

That is a hard fact from the code. A broader design inference is that Core values predictable whole-stack behavior more than substitutability.

## Current vs optional
The memory API is central. The custom heap is central in default non-`USEMALLOC` builds. Some diagnostics are optional or debug-only. SIMD acceleration is platform-dependent and opportunistic rather than guaranteed everywhere.

## See also
- [01-Architecture.md](01-Architecture.md)
- [03-Threading.md](03-Threading.md)
- [07-Logging.md](07-Logging.md)
- [08-Profiling.md](08-Profiling.md)
