# Core Architecture

## What this covers
This file summarizes how `Core.upp` is organized and how the main subsystems inside `uppsrc/Core/` fit together.

## Package layout from `Core.upp`
`uppsrc/Core/Core.upp` is the best high-level map of the package. Its file list is grouped with visible separators:

- base setup: `Core.h`, `config.h`, `Defs.h`, `Ops.h`, `Fn.h`
- CPU, memory, SIMD, threading: `Cpu.cpp`, `Mem.*`, `SIMD*`, `Atomic.h`, `Mt.*`, `St.*`, `Heap*`
- text: `String*`, `WString.cpp`, `CharSet*`, `Utf*`, `UnicodeInfo.cpp`, `Bom.cpp`
- filesystem and app environment: `Path.*`, `App.*`, `Ini.cpp`, `LocalProcess.*`
- streams and binary utilities: `Stream.*`, `BlockStream.cpp`, `FilterStream.*`, `FileMapping.*`, `StringsStream.cpp`
- diagnostics: `Profile.h`, `Diag.h`, `Log.cpp`, `Debug.cpp`
- algorithms and containers: `Algo.h`, `Sort.h`, `Vcont*`, `Index*`, `Map*`, `FixedMap.h`, `InVector*`, `Tuple.h`, `Recycler.h`, `Shared.h`
- values and formatting: `TimeDate.*`, `Value*`, `Format*`, `Convert*`, `Color*`, `Gtypes*`
- language and i18n: `i18n.h`, `Lang.*`, `LangInfo.cpp`, translation data
- parsing and serialization: `Parser.h`, `XML.*`, `Xmlize*`, `JSON.*`, `Visitor.*`, `Topic.*`
- hashing, caching, and compression: `Hash.h`, `MD5.cpp`, `SHA1.cpp`, `SHA256.cpp`, `xxhash`, `ValueCache.*`, `lz4`, `z`
- concurrency helper layer: `CoWork.*`
- web/system helpers: `Inet.*`, `Socket.cpp`, `Http.cpp`, `WebSocket.cpp`, `Daemon.*`, `dli.*`, `Win32Util.*`, `Uwp.h`

## Central domains
### Platform and build configuration
[`uppsrc/Core/Core.h`](../../../uppsrc/Core/Core.h) selects multithreaded vs single-threaded mode, heap mode, platform headers, path separator rules, and architecture-specific includes such as x86 intrinsics.

### Base types and macros
[`uppsrc/Core/Defs.h`](../../../uppsrc/Core/Defs.h) defines basic integer aliases, `wchar` as `uint32`, assertion/panic macros, one-time initialization macros, and the `pick` move convention used pervasively across the package.

### Memory
The heap API lives in [`uppsrc/Core/Heap.h`](../../../uppsrc/Core/Heap.h). When `UPP_HEAP` is enabled it routes through the custom allocator implementation in `heap.cpp`, `sheap.cpp`, `lheap.cpp`, `hheap.cpp`, and `heapdbg.cpp`. When it is disabled, the same functions inline to `malloc`/`free`.

### Containers and ownership
`Vcont.h`, `Index.h`, `Map.h`, `FixedMap.h`, `InVector.h`, and `BiCont.h` define the main container families. The architecture deliberately splits value containers from owning-pointer containers instead of hiding ownership behind allocator policy.

### Strings and text
`String`, `WString`, charset conversion, Unicode helpers, split/merge helpers, and BOM handling all live in Core because higher layers are expected to use Core text rules consistently.

### Streams and serialization
`Stream` is both an I/O abstraction and a serialization substrate. The same layer supports file streams, memory streams, filter streams, packed integer encoding, and `Serialize`/`Xmlize`/`Jsonize` utilities.

### Time and timing
`TimeDate.*`, `Profile.h`, and parts of `Util.h` provide wall-clock values, UTC/system conversion helpers, elapsed-time measurement, and low-level profiling/logging hooks.

### Threading and work scheduling
`Mt.h` and `St.h` provide the same public concurrency API with different backends. `CoWork` builds a worker-pool abstraction on top.

### Filesystem, process, and app environment
`Path.*`, `App.*`, `Ini.cpp`, and `LocalProcess.*` handle executable path discovery, home/config/temp locations, file operations, INI-style configuration, and subprocess launching.

### Diagnostics and logging
`Diag.h`, `Log.cpp`, `Debug.cpp`, and heap diagnostics define assertions, logging, timing output, crash support, and optional memory debugging.

## Architectural tradeoffs
- Core centralizes a lot of unrelated-looking primitives because the framework prefers one coherent runtime model over many narrow libraries.
- platform differences are normalized at the API layer, but implementation differences remain explicit in source files and often in behavior.
- legacy compatibility is kept inside the package instead of split into a separate compatibility module.

## Current status
The central runtime pieces are active and heavily integrated. Some areas are clearly older or compatibility-oriented, especially the callback bridge, deprecated helper blocks, and the retained single-thread mode.

## See also
- [00-Core-Philosophy.md](00-Core-Philosophy.md)
- [03-Threading.md](03-Threading.md)
- [06-Streams.md](06-Streams.md)
- [09-Containers.md](09-Containers.md)
