# Caching

## What this covers
This file documents the caching layer that is actually present in Core: `ValueCache`, `MakeValue`, and the `LRUCache<Value>`-based helpers in [`uppsrc/Core/ValueCache.h`](../../../uppsrc/Core/ValueCache.h).

## Design intent
Core does not expose a broad caching framework here. The visible implementation is specifically a cache for lazily constructed `Value` objects keyed by strings plus type-derived key material.

The public surface is centered on:

- `MakeValue(ValueMaker&)`
- `MakeValueSz(ValueMaker&, int& sz)`
- templated `MakeValue(...)` key/maker helpers
- `MakeValueTL(...)` for main-thread or thread-local caching

## Global cache behavior
[`uppsrc/Core/ValueCache.cpp`](../../../uppsrc/Core/ValueCache.cpp) shows one process-wide cache:

- `TheValueCache()` returns a static `LRUCache<Value>`
- access is serialized by `ValueCacheMutex`
- capacity is controlled by `ValueCacheMaxSize` and `ValueCacheMaxCount`

Unless fixed manually with `SetupValueCache(...)`, `AdjustValueCache()` derives the cache budget from available system memory and then shrinks the cache to fit.

## Thread-local variant
`MakeValueTL(...)` is separate from the global cache:

- on the main thread it uses a static local `LRUCache<Value>`
- on worker threads it uses `thread_local LRUCache<Value>`

So Core supports both a shared global cache and a lightweight thread-local path, but both are still `Value`-oriented rather than general-purpose memoization frameworks.

## Semantics
- eviction is LRU-based because the backing store is `LRUCache<Value>`
- cache keys are explicit strings, often prefixed with static type numbers
- cached objects are `Value`, so callers usually rely on the `Value` type system for payload transport
- `IsValueCacheActive()` only reports whether the global static cache object has not yet been torn down

## Current vs legacy
This code is current, but narrow in scope. It is infrastructure for `Value` production and reuse, not a universal caching subsystem for all Core types.

## See also
- [13-Value.md](13-Value.md)
- [14-Formatting-and-Conversion.md](14-Formatting-and-Conversion.md)
