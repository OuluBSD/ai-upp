# Parsers And Serialization

## What this covers
This file documents Core's text parsers and data-serialization helpers: `CParser`, XML, JSON, `Xmlize`, `Jsonize`, and the bridge utilities built on top of them.

## `CParser`
[`uppsrc/Core/Parser.h`](../../../uppsrc/Core/Parser.h) provides the foundational text parser.

It supports:

- identifier, integer, float, and string scanning
- optional space/comment skipping
- nested comments
- Unicode escape handling
- source-position tracking with file, line, and column

This is the common low-level parser that higher text formats build on.

## XML
Core's XML support spans:

- `XmlTag` for building tags
- `XmlParser` for streaming parse
- `XmlNode` for tree representation
- `ParseXML(...)` overloads in [`uppsrc/Core/XML.h`](../../../uppsrc/Core/XML.h)

The XML layer supports relaxed parsing, raw mode, whitespace-preservation options, entity registration, and parse filters.

## JSON
Core's JSON support is centered on [`uppsrc/Core/JSON.h`](../../../uppsrc/Core/JSON.h):

- `ParseJSON(...)` returns `Value`
- `AsJSON(...)` renders primitives and `Value`
- `Json` and `JsonArray` are small builders
- `JsonIO` is the object-style bridge used by `Jsonize(...)`

The JSON layer uses `Value`, `ValueArray`, and `ValueMap` as its common in-memory representation.

## `Xmlize` and `Jsonize`
Core provides two parallel object-mapping styles:

- `Xmlize(XmlIO&, T&)`
- `Jsonize(JsonIO&, T&)`

`XmlIO` and `JsonIO` are intentionally similar in shape:

- loading vs storing mode
- tag/key access
- array/map helpers
- user-defined `Xmlize` / `Jsonize` hooks

On top of that, `Xmlize.h` adds convenience wrappers:

- `StoreAsXML`
- `LoadFromXML`
- file variants
- `XmlizeBySerialize`
- `XmlizeByJsonize`

JSON has equivalent `StoreAsJsonValue`, `LoadFromJsonValue`, and text/file helpers in `JSON.h`.

## Tradeoffs
- XML and JSON object mapping are explicit, not reflection-based
- the common transport type is often `Value`, which keeps the layers interoperable
- XML is richer in structure and attribute handling; JSON is simpler and more directly tied to `ValueMap` / `ValueArray`

## Current vs legacy
This whole area is current and central. It is one of the main reasons `Value` exists in Core.

## See also
- [06-Streams.md](06-Streams.md)
- [13-Value.md](13-Value.md)
- [17-Localization.md](17-Localization.md)
- [19-Visitor.md](19-Visitor.md)
