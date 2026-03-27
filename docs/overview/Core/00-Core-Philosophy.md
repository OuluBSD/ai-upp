# Core Philosophy

## What this covers
This file explains what `uppsrc/Core/` is for, what design habits it pushes onto the rest of the framework, and where its semantics differ from ordinary standard-library-first C++ code.

## Design intent
`Core` is the universal non-GUI dependency layer for the framework. `Core.upp` groups platform detection, base types, memory, containers, strings, streams, time, threading, diagnostics, serialization, networking helpers, and process utilities into one package that higher layers can assume exists.

The package does not try to be a thin wrapper over the C++ standard library. `Core.h` pulls in large parts of the C and C++ runtime, then establishes U++ conventions on top: custom basic types in [`uppsrc/Core/Defs.h`](../../../uppsrc/Core/Defs.h), the U++ heap in [`uppsrc/Core/Heap.h`](../../../uppsrc/Core/Heap.h), U++ containers in [`uppsrc/Core/Vcont.h`](../../../uppsrc/Core/Vcont.h), U++ strings in [`uppsrc/Core/String.h`](../../../uppsrc/Core/String.h), and U++ streams in [`uppsrc/Core/Stream.h`](../../../uppsrc/Core/Stream.h).

## Explicit semantics over hidden convenience
The dominant pattern in Core is to make storage and ownership visible in the type:

- `Vector<T>` stores values contiguously.
- `Array<T>` stores owning pointers and keeps pointed-to objects separately allocated.
- `String` is byte-oriented and optimized for short text plus shared large storage.
- `WString` is a separate 32-bit-code-point string type, not just `String` with a different view.
- `Stream` is an explicit read/write/seek object rather than an iostream-style formatting stack.

This is not only API style. The implementation usually exposes the intended tradeoff directly in code: inline-vs-heap string tiers, value-vs-owning containers, packed-vs-raw stream serialization, single-thread shims vs multithreaded implementations, and explicit config/log path overrides.

## Debug should fail early
`Defs.h` and `Diag.h` make debug builds opinionated:

- `ASSERT` and `VERIFY` route to `AssertFailed`.
- `LOG`, `DUMP`, `TIMING`, and related helpers are enabled in `_DEBUG`.
- `DLOG`-family macros intentionally turn into invalid tokens in non-debug builds, forcing cleanup before release.

That "fail early" stance is visible in other places too: heap diagnostics under `UPP_HEAP`, explicit magic checks, log-etalon testing, and profiling helpers that write summaries at teardown.

## Release builds stay usable
Release code paths usually keep the API shape but drop expensive checking:

- `ASSERT` becomes a no-op.
- `LOG` disappears, but `RLOG` remains available.
- the single-thread headers in [`uppsrc/Core/St.h`](../../../uppsrc/Core/St.h) keep thread APIs callable even when there is no real OS-thread backing.
- if `flagUSEMALLOC` or `flagSO` disables `UPP_HEAP`, the memory API still exists, but it falls back to `malloc`/`free`.

The package hides platform differences at the API boundary, but it does not pretend the implementations are identical. `Path`, `Thread`, `FileStream`, `FindFile`, time conversion, logging destinations, and config lookup all branch by platform.

## Current vs legacy
Some Core surfaces are clearly current and central: strings, containers, streams, path helpers, logging, time, and threading. Some are compatibility layers or migration paths:

- [`uppsrc/Core/Callback.h`](../../../uppsrc/Core/Callback.h) starts with `// Backward compatibility; use Function/Event in the new code`.
- deprecated compatibility blocks still exist throughout headers.
- `flagST` keeps a single-thread configuration alive, but much of the ecosystem assumes the multithreaded path.

Treat Core as a stable foundation with accumulated history, not as a freshly minimal runtime.

## See also
- [01-Architecture.md](01-Architecture.md)
- [02-Memory-and-Performance.md](02-Memory-and-Performance.md)
- [09-Containers.md](09-Containers.md)
- [README.md](README.md)
