# Value

## What this covers
This file documents Core's dynamic value system: `Value`, `ValueType`, `ValueArray`, `ValueMap`, error values, and the small-vs-rich storage split used by the runtime.

## Design intent
`Value` is Core's type-erased runtime value container. It exists so generic formatting, serialization, JSON/XML conversion, argument passing, and loosely typed data structures can share one common transport type.

The implementation is explicit about type identity:

- built-in type numbers such as `INT_V`, `STRING_V`, `DATE_V`, `VALUEARRAY_V`, and `VALUEMAP_V` live in [`uppsrc/Core/Value.h`](../../../uppsrc/Core/Value.h)
- custom types participate through `ValueType<T,...>` and explicit registration
- `Value::RegisterStd()` wires up built-in rich types in [`uppsrc/Core/Value.cpp`](../../../uppsrc/Core/Value.cpp)

## Storage model
Two major storage modes are visible in code:

- small values use the internal `String` payload directly when `FitsSvoValue<T>()` holds, which is currently `sizeof(T) <= 8`
- larger or polymorphic values use `Value::Void` with reference counting and virtual operations

That is why `Value.h` exposes both:

- `SvoToValue(...)` for small-value optimization
- `RichToValue(...)` and raw-value helpers for heap-backed value objects

The model is explicit, not opaque. A type can be cheap inline data or a refcounted heap object depending on its size and registration path.

## Main abstractions
### `Value`
`Value` supports:

- runtime type inspection with `GetType()` and `GetTypeName()`
- typed extraction with `Is<T>()`, `To<T>()`, and `Get<T>()`
- comparison and hashing
- serialization, XML, and JSON conversion
- array-like and map-like access through `GetCount()`, `operator[](int)`, and keyed operators

### `ValueType<T,...>`
`ValueType` is the base mixin for user-defined Value-compatible types. It defines the hooks that rich types are expected to support:

- `Serialize`
- `Xmlize`
- `Jsonize`
- `GetHashValue`
- `ToString`
- `Compare`
- `PolyCompare`

### `ValueArray`
`ValueArray` is a refcounted wrapper around `Vector<Value>`. It is copy-on-write through `Clone()` in [`uppsrc/Core/ValueUtil.cpp`](../../../uppsrc/Core/ValueUtil.cpp).

### `ValueMap`
`ValueMap` stores:

- `Index<Value> key`
- `ValueArray value`

So map keys preserve index order while still supporting keyed lookup. `ValueMap` is also copy-on-write.

## Semantics and tradeoffs
- `Value` is not a variant with only compile-time-known alternatives; it is a registry-backed runtime type system.
- `ValueArray` and `ValueMap` are first-class value types, not just convenience wrappers.
- keyed access is forgiving: missing keyed lookups often return `ErrorValue()` rather than crashing immediately.
- serialization requires real registered type IDs; `Value.cpp` asserts this before storing.

## Current vs legacy
`Value` is central and current. It is used by formatting, conversion, JSON/XML helpers, and command-line parsing utilities in Core.

The historical aspect is the amount of compatibility machinery around registration and polymorphic comparison. The dynamic value system itself is active, not legacy.

## See also
- [06-Streams.md](06-Streams.md)
- [14-Formatting-and-Conversion.md](14-Formatting-and-Conversion.md)
- [18-Parsers-and-Serialization.md](18-Parsers-and-Serialization.md)
- [19-Visitor.md](19-Visitor.md)
