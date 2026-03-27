# Visitor

## What this covers
This file documents `uppsrc/Core/Visitor.h`: what it is for, how it differs from `Serialize` / `Jsonize` / `Xmlize`, and why it should be treated as a non-central or fork-specific Core facility.

## Design intent
`Visitor` is a mode-driven adapter that can route one `Visit(...)` implementation across several backends:

- stream serialization
- JSON
- hashing
- version-control-style persistence
- runtime/access/constraint-style modes

The same object method can therefore feed multiple representations.

## Main structure
`Visitor` stores backend pointers and mode flags directly:

- `JsonIO* json`
- `Stream* stream`
- `VersionControlSystem* vcs`
- `CombineHash hash`
- `mode`, `file_ver`, `skip`, and `storing`

It then exposes helpers such as:

- `Visit`
- `VisitT`
- `VisitVector`
- `VisitMap`
- `VisitMapKV`
- `Ver(...)` and version gating

This is more orchestration-heavy than `Serialize(Stream&)`, `Jsonize(JsonIO&)`, or `Xmlize(XmlIO&)`.

## Semantics
The visitor API assumes user types implement `Visit(Visitor&)` and sometimes type-specific variants. It also knows about repository-specific infrastructure:

- `VersionControlSystem`
- access/menu/value hooks
- runtime-visit and constraint modes

That makes it broader than a generic serialization helper and more coupled to this fork's surrounding systems.

## Current vs fork-specific
`Visitor` is present and functional in this repository, but it is not part of the standard Core story in the same way that `Serialize`, `Jsonize`, `Xmlize`, `Value`, or `Stream` are.

Repository evidence for that assessment:

- it depends on `VersionControlSystem`
- it has UI/access-style hooks in the same type
- it is absent from the older Core high-level docs
- its mode set extends well beyond plain serialization

So this should be treated as fork-specific or at least non-central infrastructure inside this tree.

## Limitations and caveats
- the API is template-heavy and convention-based
- mode handling is explicit and can become hard to reason about
- it overlaps conceptually with `Serialize`, `Jsonize`, and `Xmlize` rather than replacing them cleanly

## See also
- [06-Streams.md](06-Streams.md)
- [13-Value.md](13-Value.md)
- [18-Parsers-and-Serialization.md](18-Parsers-and-Serialization.md)
- [10-Recycling.md](10-Recycling.md)
