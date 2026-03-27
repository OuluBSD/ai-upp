# Streams

## What this covers
This file documents Core's stream abstraction, the main derived stream types, and the serialization idioms built on top of them.

## The base abstraction
[`uppsrc/Core/Stream.h`](../../../uppsrc/Core/Stream.h) defines `Stream` as the common buffered interface for:

- reading and writing bytes
- seeking
- error reporting
- line and UTF-8 helpers
- object serialization

Core stream state is explicit through flags such as `STRM_READ`, `STRM_WRITE`, `STRM_SEEK`, `STRM_LOADING`, and `STRM_THROW`.

## Serialization role
`Stream` is not only for raw I/O. It also serves as the standard serialization transport:

- `SerializeRaw` for fixed-width raw values
- `%` operators for value serialization
- `/` operators for packed integer-style serialization
- `Pack` helpers for compact integers and boolean bit packing
- `Magic` for stream signatures

This is a central Core pattern. Many container, value, and time types use `Serialize(Stream&)`.

## Main derived stream types
From `Stream.h` and companion source files:

- `StringStream`: reads from or writes to a `String`
- `MemStream` and `MemReadStream`: wrap caller-owned memory buffers
- `BlockStream`: buffered page-based random-access stream base
- `FileStream`: OS file-handle implementation on top of `BlockStream`
- `FileIn`, `FileOut`, `FileAppend`: convenience wrappers
- `SizeStream`: counts serialized output size without storing data
- `CompareStream`: compares serialized output against another stream
- `OutStream` and `TeeStream`: sink-style output helpers
- `InFilterStream` and `OutFilterStream`: attach transform filters

`CopyStream`, `LoadStream`, and `SaveStream` provide common stream-to-stream and string-to-stream helpers.

## Usage style
Core prefers explicit binary dataflow:

- open a concrete stream
- choose loading vs storing
- call `Serialize` or use primitive stream operators
- copy between streams with `CopyStream`

This is simpler and more predictable than iostream formatting, but it also means the model is mostly synchronous and byte-oriented.

## Limitations
The stream layer is low-level:

- no built-in async stream protocol
- no formatting-rich text stream comparable to C++ iostream manipulators
- text encoding is handled separately through string and charset helpers

That limitation is intentional. The stream layer stays close to transport and serialization semantics.

## See also
- [04-Strings-and-Text.md](04-Strings-and-Text.md)
- [05-Paths-and-Config.md](05-Paths-and-Config.md)
- [09-Containers.md](09-Containers.md)
- [12-Time.md](12-Time.md)
