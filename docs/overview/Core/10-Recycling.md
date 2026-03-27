# Recycling

## What this covers
This file documents the recycler utilities in [`uppsrc/Core/Recycler.h`](../../../uppsrc/Core/Recycler.h): what they actually do with object lifetime and where they fit relative to normal containers.

## Main types
`Recycler.h` defines:

- `RecyclerPool<T, keep_as_constructed>`
- `Recycler<T, keep_as_constructed>`
- `RecyclerRefBase<T>`
- `SharedRecycler<T>`
- `BiVectorRecycler<T, keep_as_constructed>`

## Lifetime semantics
`RecyclerPool` stores raw pointers in an internal pool backed by `MemoryAlloc`/`MemoryFree`.

Behavior depends on `keep_as_constructed`:

- when `false`:
  - `New(...)` constructs an object with placement new when taken from the pool
  - `Return(...)` calls the destructor before putting the memory back into the pool
- when `true`:
  - returned objects stay constructed while pooled
  - `Clear()` and destruction explicitly run `~T()` before freeing memory

So the pool always reuses storage, but it does not always preserve constructed object state.

## Relation to linked and deque-like structures
`BiVectorRecycler` combines a `BiVector<T*>` queue with an internal `RecyclerPool<T,...>`. It is useful when you want:

- frequent push/pop at both ends
- pooled object allocation
- direct object access through references while keeping pooled backing storage

## Threading
`RecyclerPool` uses a `Mutex` around its internal free-list vector. It is thread-safe at the pool level, but it is intentionally simple and not lock-free.

## Scope and status
This facility exists in the current tree and is functional. It does not appear in the historical Core `srcdoc` material and is not one of the canonical upstream U++ concepts usually discussed first. Based on that repository evidence, it is best treated as a fork-specific or at least non-central utility rather than a primary Core abstraction.

That is an inference from the repository layout, not a claim about the full external history of every U++ branch.

## See also
- [02-Memory-and-Performance.md](02-Memory-and-Performance.md)
- [03-Threading.md](03-Threading.md)
- [09-Containers.md](09-Containers.md)
- [11-Callbacks-and-Events.md](11-Callbacks-and-Events.md)
