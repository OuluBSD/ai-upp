# Threading

## What this covers
This file documents Core's concurrency surface: thread objects, locks, condition variables, atomics, one-time initialization, and the single-thread compatibility path.

## Two backends, one API
`Core.h` selects:

- [`uppsrc/Core/Mt.h`](../../../uppsrc/Core/Mt.h) when multithreading is enabled
- [`uppsrc/Core/St.h`](../../../uppsrc/Core/St.h) when `flagST` forces single-thread mode

The public class names stay the same: `Thread`, `Mutex`, `RWMutex`, `Semaphore`, `ConditionVariable`, `StaticMutex`, `SpinLock`, and `OnceFlag`.

## Multithreaded implementation
`Mt.h` provides real OS-backed implementations:

- `Thread` wraps `HANDLE` on Windows and `pthread_t` on POSIX
- `Mutex` is `CRITICAL_SECTION` on Windows and `pthread_mutex_t` on POSIX
- `RWMutex` uses native reader-writer primitives on both platforms
- `ConditionVariable` uses the native API where available and an internal semaphore-based fallback on older Windows
- `Semaphore` wraps Win32 semaphores, POSIX semaphores, or GCD semaphores on macOS

`Atomic` itself is very small in [`uppsrc/Core/Atomic.h`](../../../uppsrc/Core/Atomic.h): it is just `std::atomic<int>` plus `AtomicInc` and `AtomicDec`.

## Single-thread mode
`St.h` keeps the same class names but strips out real concurrency:

- `Thread::IsOpen()` is always false
- `Mutex::Enter()` and `Leave()` are empty
- `SpinLock` is effectively a no-op
- `ConditionVariable` and `Semaphore` keep the signatures but not OS synchronization behavior

This is a compatibility mode, not a full concurrency subsystem. It is useful when code wants the same source-level API in non-threaded builds, but it does not preserve multithreaded semantics.

## One-time initialization
`ONCELOCK` and `ONCELOCK_` are central convenience macros:

- in `Mt.h` they use `std::atomic<bool>` plus a `Mutex`
- in `St.h` they collapse to simple static-state loops

This is used widely in Core for lazy initialization of logs, executable paths, and profiling support.

## Higher-level scheduling
[`uppsrc/Core/CoWork.h`](../../../uppsrc/Core/CoWork.h) adds a worker-pool layer:

- `CoWork::Do` queues work into a global pool
- `CoFor` distributes range-style loops across workers
- `AsyncWork` wraps asynchronous result collection

This is current code, not a deprecated side utility. It depends on the thread primitives underneath.

## Tradeoffs
- the API is deliberately simple and portable rather than feature-complete
- `flagST` keeps old or low-overhead build modes possible, but many higher layers are effectively designed for the multithreaded path
- `Atomic` is intentionally narrow; more complex atomic patterns are built with `Mutex`, `OnceFlag`, or custom logic

## Current vs legacy
The threading API itself is active. The single-thread branch is still present and maintained enough to compile, but it should be treated as a constrained compatibility configuration.

## See also
- [01-Architecture.md](01-Architecture.md)
- [02-Memory-and-Performance.md](02-Memory-and-Performance.md)
- [08-Profiling.md](08-Profiling.md)
- [11-Callbacks-and-Events.md](11-Callbacks-and-Events.md)
