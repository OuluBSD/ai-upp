# Hashing

## What this covers
This file documents the hashing facilities that are visibly implemented in Core: generic `GetHashValue` / `CombineHash`, cryptographic digest helpers, xxHash wrappers, and related hashing-by-serialization utilities.

## Generic hashing API
The foundational hashing helpers live in [`uppsrc/Core/Topt.h`](../../../uppsrc/Core/Topt.h):

- `GetHashValue(...)` overloads for primitives, pointers, and user types
- `CombineHash` for multi-field hashing
- `GetPtrHashValue(...)`
- `memhash(...)` for raw byte hashing

This is the layer used by containers and value types throughout Core.

Examples visible in code:

- geometry and color types define `GetHashValue()` with `CombineHash`
- `ValueArray` and `ValueMap` fold child hashes in [`uppsrc/Core/ValueUtil.cpp`](../../../uppsrc/Core/ValueUtil.cpp)
- `HashBySerialize(...)` in [`uppsrc/Core/Util.h`](../../../uppsrc/Core/Util.h) hashes an object's serialized form with `xxHashStream`

## Digest algorithms in `Hash.h`
[`uppsrc/Core/Hash.h`](../../../uppsrc/Core/Hash.h) exposes four concrete digest families:

- MD5
- SHA-1
- SHA-256
- xxHash / xxHash64

For MD5, SHA-1, and SHA-256, Core provides both:

- one-shot functions for memory and `String`
- stream-style wrappers derived from `OutStream`

The implementations live in:

- [`uppsrc/Core/MD5.cpp`](../../../uppsrc/Core/MD5.cpp)
- [`uppsrc/Core/SHA1.cpp`](../../../uppsrc/Core/SHA1.cpp)
- [`uppsrc/Core/SHA256.cpp`](../../../uppsrc/Core/SHA256.cpp)

## xxHash
[`uppsrc/Core/xxHsh.cpp`](../../../uppsrc/Core/xxHsh.cpp) wraps the bundled xxHash library from [`uppsrc/Core/lib/xxhash.c`](../../../uppsrc/Core/lib/xxhash.c).

Visible features:

- `xxHashStream` with optional 32-bit seed
- `xxHash64Stream` with optional 32-bit seed
- one-shot `xxHash(...)` and `xxHash64(...)`

This is the fast non-cryptographic hashing path used by generic helpers such as `HashBySerialize(...)`.

## Semantics and tradeoffs
- `GetHashValue` / `CombineHash` are the normal structural-hash mechanism in Core
- xxHash is the fast utility hash
- MD5, SHA-1, and SHA-256 are explicit digest algorithms, not interchangeable with container hash semantics

The code does not claim one unified "best hash." Core keeps both structural hashing and named digest algorithms because they solve different problems.

## Current vs legacy
This area is current. The only legacy aspect is algorithm choice at the security level: MD5 and SHA-1 remain available because the package exposes them, not because the code treats them as modern security defaults.

## See also
- [13-Value.md](13-Value.md)
- [21-Compression.md](21-Compression.md)
