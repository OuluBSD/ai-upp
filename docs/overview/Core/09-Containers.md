# Containers

## What this covers
This file summarizes the main Core container families and the ownership semantics that distinguish them.

## Value vs owning-pointer containers
This is the most important split in Core containers.

### `Vector<T>`
Defined in [`uppsrc/Core/Vcont.h`](../../../uppsrc/Core/Vcont.h), `Vector<T>` stores values directly in contiguous memory. It is the default choice when:

- you want dense storage
- relocation of elements is acceptable
- ownership is value-based

### `Array<T>`
Also in `Vcont.h`, `Array<T>` stores owning pointers. It allocates each element separately with `new` and deletes them on removal or destruction.

Consequences:

- objects have independent addresses
- reordering the array moves pointers, not the objects themselves
- insertion/removal changes pointer positions in the pointer array, but not the pointed-to object addresses

If object identity and stable addresses matter more than compact value storage, `Array<T>` is the relevant container.

## Indexed sets and maps
### `Index<T>`
[`uppsrc/Core/Index.h`](../../../uppsrc/Core/Index.h) combines an ordered key vector with a hash structure. It supports duplicate keys, `FindNext`/`FindPrev`, and an `Unlink`/`Sweep` lifecycle for logical removal without immediate compaction.

Use it when you want:

- indexed iteration order
- fast key lookup
- optional duplicate keys

### `VectorMap<K, V>` and `ArrayMap<K, V>`
[`uppsrc/Core/Map.h`](../../../uppsrc/Core/Map.h) defines `AMap` as a shared base:

- `VectorMap` uses `Vector<V>` for values
- `ArrayMap` uses `Array<V>` for values

The same ownership rule carries through from the underlying value container. The key side is handled by an `Index<K>`.

## Fixed and sorted forms
### `FixedVectorMap` and `FixedArrayMap`
[`uppsrc/Core/FixedMap.h`](../../../uppsrc/Core/FixedMap.h) builds immutable-after-`Finish()` sorted maps over parallel key/value storage. Lookup is binary-search-based rather than hash-based.

These are useful when the map is built once, then queried many times.

### `InVector`, `InArray`, and sorted map variants
[`uppsrc/Core/InVector.h`](../../../uppsrc/Core/InVector.h) provides alternative data structures and sorted map wrappers that trade implementation complexity for different insertion and lookup behavior.

## Other container helpers
Core also includes:

- `One<T>` for nullable single-object ownership
- `Buffer<T>` for raw temporary buffers
- `BiVector` and `BiArray` for double-ended storage
- `Tuple`, `Range`, `Shared`, `LinkedList`, and `CritBitIndex`

## Tradeoffs
Core container choice is meant to communicate semantics:

- `Vector` vs `Array` is mostly a question of value relocation vs owned object identity
- `Index`/`Map` types preserve insertion order while also supporting keyed lookup
- fixed sorted containers trade mutability for compact lookup structure

## Current vs legacy
These container families are central to Core. Some deprecated STL-compatibility helpers remain in headers, but the ownership model itself is current and fundamental.

## See also
- [00-Core-Philosophy.md](00-Core-Philosophy.md)
- [01-Architecture.md](01-Architecture.md)
- [02-Memory-and-Performance.md](02-Memory-and-Performance.md)
- [10-Recycling.md](10-Recycling.md)
