# UUID

## What this covers
This file documents Core's `Uuid` type, its string forms, parsing rules, and integration with `Value`, JSON, and XML.

## Representation
[`uppsrc/Core/Uuid.h`](../../../uppsrc/Core/Uuid.h) defines `Uuid` as a movable Value-compatible type backed by:

- `uint64 v[2]`

The type is registered with the dynamic value system in [`uppsrc/Core/Uuid.cpp`](../../../uppsrc/Core/Uuid.cpp), so it participates in `Value`, hashing, comparison, and serialization-style helpers.

## Generation
`Uuid::New()` fills the two 64-bit words with `Random64(...)` until the value is non-null.

That means the implementation guarantees a nonzero random identifier. It does not visibly enforce RFC version or variant bits in the code shown here, so the docs should not overstate standards compliance.

## Formatting
Core exposes two main printable forms:

- `Format(const Uuid&)`: 32 uppercase hexadecimal digits with no dashes
- `FormatWithDashes(const Uuid&)`: dashed UUID-style text

The plain `ToString()` path uses the undashed uppercase representation.

## Parsing
`ScanUuid(...)` accepts both dashed and undashed input by stripping non-hex characters and then decoding hex digits.

The parser requires at least 32 hex digits' worth of data. In practice that makes it forgiving about separators while still demanding a full UUID payload.

## Data-exchange hooks
`Uuid` is integrated into Core's serialization-style layers:

- `Jsonize` stores it as a string
- `Xmlize` stores it in an attribute named `value`
- `UuidValueGen()` produces a random UUID string as a `Value`

So `Uuid` fits naturally into the same object-mapping stack used by other Core types.

## Current vs legacy
This is current utility code. It is a small subsystem, but it is fully integrated with `Value` and the parser/serialization helpers around it.

## See also
- [13-Value.md](13-Value.md)
- [18-Parsers-and-Serialization.md](18-Parsers-and-Serialization.md)
