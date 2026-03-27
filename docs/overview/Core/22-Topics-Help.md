# Topics And Help

## What this covers
This file documents Core's packaged help-topic layer: `Topic`, `TopicLink`, `topic://` links, topic registration, and runtime lookup/decompression.

## Main abstractions
[`uppsrc/Core/Topic.h`](../../../uppsrc/Core/Topic.h) defines a compact data model:

- `Topic` with `title`, `text`, `link`, and `label`
- `TopicLink` with `package`, `group`, `topic`, and optional anchor label
- helpers for converting links to and from `topic://...` strings

The link syntax is explicit. `TopicLinkString(...)` renders links like:

- `topic://package/group/topic`
- `topic://package/group/topic#label`

## Runtime storage
[`uppsrc/Core/Topic.cpp`](../../../uppsrc/Core/Topic.cpp) shows that topics are registered into static tables at startup through `RegisterTopic__`.

Topic payloads are not stored as plain text in the registry. `GetTopic(...)` pulls compressed data and expands it with `ZDecompress(...)`.

That ties this subsystem directly to Core's compression layer.

## Language handling
`GetTopicLNG(...)` appends the current language suffix and resolves localized variants when available.

So the packaged help system is language-aware, but it is still based on explicit topic names and registries rather than a generic document database.

## Role in the stack
This is the runtime half of U++'s Topic++ help/documentation mechanism:

- build-time tools package topic content into registries
- Core resolves those entries at runtime
- consumers receive `Topic` objects and `topic://` links

The code in Core is the loader and resolver, not the editor.

## Current vs legacy
This subsystem is current code, but it is not as central as strings, containers, or streams. It is a specialized documentation/help facility that still matters because the wider U++ ecosystem uses Topic++ heavily.

## See also
- [17-Localization.md](17-Localization.md)
- [18-Parsers-and-Serialization.md](18-Parsers-and-Serialization.md)
- [21-Compression.md](21-Compression.md)
