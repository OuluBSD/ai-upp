# Profiling

## What this covers
This file documents the profiling and timing facilities that are actually present in Core today.

## Main facilities
Profiling is low-level and macro-driven. The key pieces are:

- [`uppsrc/Core/Profile.h`](../../../uppsrc/Core/Profile.h): `TimingInspector`, `HitCountInspector`, `RTIMING`, `RHITCOUNT`
- [`uppsrc/Core/Util.h`](../../../uppsrc/Core/Util.h): `TimeStop`, `TimeStopper`, `RTIMESTOP`
- [`uppsrc/Core/Diag.h`](../../../uppsrc/Core/Diag.h): debug-facing aliases `TIMING`, `HITCOUNT`, `TIMESTOP`
- [`uppsrc/Core/Debug.cpp`](../../../uppsrc/Core/Debug.cpp): implementation and log dumping

## How it works
`RTIMING("name")` creates a static `TimingInspector` plus a scoped `Routine` object. On scope exit it records elapsed time. `HitCountInspector` just counts hits and logs the total at destruction time.

`RTIMESTOP("name")` is simpler: it measures one scope with `TimeStop` and logs the elapsed result through `RLOG` when the scope ends.

## Output behavior
Profiling data is written to the normal log stream:

- `TimingInspector::~TimingInspector()` writes `Dump()` output to `StdLog()`
- `HitCountInspector::~HitCountInspector()` logs through `RLOG`
- `TimeStopper` logs through `RLOG`

There is no evidence in Core itself of a structured profiler database, timeline UI, or IDE-specific visualization protocol.

## Activation and debug/release behavior
`TimingInspector` has a runtime `active` flag and `TimingInspector::Activate(bool)`.

Macro exposure differs by build:

- `TIMING`, `HITCOUNT`, and `TIMESTOP` are debug-only aliases
- `RTIMING`, `RHITCOUNT`, and `RTIMESTOP` are available regardless and are the lower-level primitives

## Tradeoffs
This profiler is lightweight and easy to scatter through code, but it is also coarse:

- scope-based only
- log-oriented output
- no call tree persistence beyond the aggregated counters
- millisecond timing in `TimingInspector`, microsecond helper for `TimeStop`

## Current vs future
Current support is real but low-level. If the surrounding IDE or tools visualize these logs elsewhere, that is outside what Core itself proves. A reasonable future direction would be structured emission instead of plain log lines, but that is not current behavior.

## See also
- [02-Memory-and-Performance.md](02-Memory-and-Performance.md)
- [03-Threading.md](03-Threading.md)
- [07-Logging.md](07-Logging.md)
- [12-Time.md](12-Time.md)
