# Compression

## What this covers
This file documents Core's built-in compression helpers: zlib/deflate, gzip wrappers, stream adapters, CRC32, and the separate LZ4-based fast-compression path.

## `Zlib`
[`uppsrc/Core/z.h`](../../../uppsrc/Core/z.h) exposes `Zlib` as the low-level incremental compressor/decompressor.

Visible features include:

- compression and decompression mode
- optional gzip framing with `GZip(bool)`
- header and CRC control
- configurable chunk size and compression level
- incremental `Put`, `End`, and `Get` flow

On decompression, the implementation can also report gzip metadata such as original filename and comment.

## Stream wrappers
Core adds stream adapters on top of that low-level engine:

- `ZCompressStream`
- `ZDecompressStream`

These are built on `OutFilterStream` and `InFilterStream`, so compression fits naturally into the package's general stream model.

## Convenience functions
The same header provides one-shot helpers:

- `ZCompress` / `ZDecompress`
- `GZCompress` / `GZDecompress`
- `GZCompressFile` / `GZDecompressFile`
- `CRC32`

This is the simplest path when the caller does not need incremental streaming.

## Fast path: `FastCompress`
[`uppsrc/Core/z.cpp`](../../../uppsrc/Core/z.cpp) also contains a separate fast-compression path:

- `FastCompress`
- `FastDecompress`

This path uses LZ4, not zlib. It prepends the original uncompressed size as a 4-byte integer and then stores the LZ4 payload.

That means it is a Core-specific transport format, not interchangeable with `.gz` or raw deflate data.

## Tradeoffs
- zlib/gzip path is interoperable and stream-friendly
- gzip mode adds common file-format behavior and metadata support
- `FastCompress` favors speed and simplicity over standard interchange
- CRC helpers are available directly, which is useful outside full gzip workflows

## Current vs legacy
This area is current. Compression is used both directly and indirectly, including by the topic/help subsystem for packaged documentation payloads.

## See also
- [06-Streams.md](06-Streams.md)
- [22-Topics-Help.md](22-Topics-Help.md)
