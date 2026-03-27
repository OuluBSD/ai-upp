# CoWork

## What this covers
This file documents Core's lightweight job-parallelism helper: `CoWork`, its global worker pool, scheduling model, and the practical limits visible in implementation.

## Design intent
[`uppsrc/Core/CoWork.h`](../../../uppsrc/Core/CoWork.h) presents `CoWork` as a simple way to schedule independent jobs and wait for them as a group.

The implementation is intentionally direct, not an open-ended executor framework.

## Pool model
[`uppsrc/Core/CoWork.cpp`](../../../uppsrc/Core/CoWork.cpp) shows one important limitation clearly:

- there is a single global pool returned by `CoWork::GetPool()`
- the default worker count is initialized from `CPU_Cores() + 2`
- `SetPoolSize(int)` rebuilds that one shared pool

So `CoWork` is process-global. It is not per-instance and it does not expose multiple independent pools.

## Scheduling behavior
Jobs are stored in a fixed slot array with:

- `Pool::SCHEDULED_MAX = 2048`

When `Do0` cannot obtain a free scheduled slot, the code falls back to running the job synchronously in the calling thread.

`TrySchedule` exposes the same limit more explicitly by returning `false` when no free slot is available.

That means saturation does not necessarily drop work, but it can silently reduce parallelism.

## Completion and exceptions
`Finish()` waits for scheduled jobs and may also execute queued work inline while draining the pool.

Worker exceptions are captured as `std::exception_ptr` and rethrown in `Finish0()`. So failures are deferred to synchronization time rather than immediately escaping from the worker thread.

## Tradeoffs
- very convenient for local fork-join style work
- shared global pool keeps the API small
- fixed slot count and synchronous fallback make behavior predictable but not elastic
- not a replacement for a richer task scheduler, custom queueing policy, or isolated executors

## Current vs legacy
`CoWork` is current and useful, but deliberately simple. Its single-pool design is a real implementation constraint, not just an API omission.

## See also
- [03-Threading.md](03-Threading.md)
- [08-Profiling.md](08-Profiling.md)
