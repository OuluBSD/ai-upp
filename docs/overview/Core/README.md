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
