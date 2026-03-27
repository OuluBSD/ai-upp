# Core Overview

`uppsrc/Core/` is the framework's foundational non-GUI runtime layer. It defines the shared semantics for memory, ownership, strings, containers, streams, time, threading, diagnostics, paths, config, and low-level serialization that higher packages build on.

Core overview documents:

- [00-Core-Philosophy.md](00-Core-Philosophy.md): design intent, explicit semantics, and debug-vs-release behavior.
- [01-Architecture.md](01-Architecture.md): package structure and the main subsystems visible in `Core.upp`.
- [02-Memory-and-Performance.md](02-Memory-and-Performance.md): custom heap, diagnostics, and low-level performance helpers.
- [03-Threading.md](03-Threading.md): thread primitives, locks, atomics, and the single-thread compatibility mode.
- [04-Strings-and-Text.md](04-Strings-and-Text.md): `String`, `WString`, Unicode, charset conversion, and text semantics.
- [05-Paths-and-Config.md](05-Paths-and-Config.md): path handling, executable/home/config discovery, and INI behavior.
- [06-Streams.md](06-Streams.md): the stream abstraction and its role in both I/O and serialization.
- [07-Logging.md](07-Logging.md): log macros, routing, and default log-file location.
- [08-Profiling.md](08-Profiling.md): timing and hit-count instrumentation that Core currently provides.
- [09-Containers.md](09-Containers.md): container families and the ownership rules behind them.
- [10-Recycling.md](10-Recycling.md): recycler utilities and pooled object lifetime.
- [11-Callbacks-and-Events.md](11-Callbacks-and-Events.md): `Function`, `Event`, `Gate`, and callback compatibility layers.
- [12-Time.md](12-Time.md): civil time, UTC helpers, parsing/formatting, and elapsed-time utilities.
- [13-Value.md](13-Value.md): the dynamic value system, `ValueArray`, `ValueMap`, and type registration.
- [14-Formatting-and-Conversion.md](14-Formatting-and-Conversion.md): formatter registry, scanners, and `Convert` classes.
- [15-Color.md](15-Color.md): `Color`, `RGBA`, symbolic colors, and color-space helpers.
- [16-Geometry-Primitives.md](16-Geometry-Primitives.md): `Point`, `Size`, `Rect`, and the foundational 2D geometry helpers.
- [17-Localization.md](17-Localization.md): translation lookup, language IDs, and `LanguageInfo`.
- [18-Parsers-and-Serialization.md](18-Parsers-and-Serialization.md): `CParser`, XML, JSON, `Xmlize`, and `Jsonize`.
- [19-Visitor.md](19-Visitor.md): the mode-driven visitor layer used by this repository's extended serialization/runtime flows.
- [20-Pointer-Safety.md](20-Pointer-Safety.md): `Pte`, `Ptr`, and the opt-in tracked-pointer mechanism.
- [21-Compression.md](21-Compression.md): zlib/gzip helpers, CRC32, and the separate LZ4 fast path.
- [22-Topics-Help.md](22-Topics-Help.md): packaged Topic++ help records, `topic://` links, and runtime topic lookup.
- [23-CoWork.md](23-CoWork.md): the global worker pool, scheduling model, and saturation behavior.
- [24-UUID.md](24-UUID.md): random UUID values, formatting, parsing, and `Value`/JSON/XML integration.
- [25-Caching.md](25-Caching.md): `ValueCache`, `MakeValue`, and the `LRUCache<Value>`-based memoization layer.
- [26-Hashing.md](26-Hashing.md): structural hashing, digest helpers, and the bundled xxHash wrappers.
- [27-Networking.md](27-Networking.md): TCP sockets, HTTP, WebSocket, and the current transport boundary.
- [28-Daemon.md](28-Daemon.md): the service host loop and the fork-specific serial/TCP daemon helpers.
- [29-Runtime-Linking.md](29-Runtime-Linking.md): the macro-driven runtime library binding layer in `dli.*`.
- [30-Windows-Specific.md](30-Windows-Specific.md): Win32 utility helpers, UWP stubs, and retained legacy Windows branches.
