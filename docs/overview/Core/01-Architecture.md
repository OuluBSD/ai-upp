# Core Architecture

## What this page is for
This page is about how Core behaves as an architectural center, not a file-by-file package map.

Core gathers many concerns that would be separate libraries in a more fragmented ecosystem. That concentration is a deliberate architectural choice. The package wants to be the place where runtime assumptions stay coherent.

## A center of gravity
Core is broad because the framework prefers one strong runtime center of gravity over many loosely related support libraries.

Memory, text, containers, streams, time, threading, diagnostics, parsing, transport, and platform helpers all meet here because they influence one another. The architecture assumes those relationships are important enough to keep under one roof.

This makes Core feel dense, but it also reduces the chance that higher layers drift into conflicting subcultures.

## Architecture by shared semantics
The real architecture is not just the source tree. It is the set of shared semantic expectations that higher packages inherit:

- how ownership is represented
- how data is serialized and moved
- how errors and diagnostics are surfaced
- how much platform difference should remain visible
- how debug and release are expected to differ

Core is the package that fixes those expectations early.

## Not a purity architecture
There is no sign that Core wants a perfectly clean textbook layering. It is more pragmatic than that.

Historical pieces stay if they continue to serve the wider tree. Specialized subsystems remain near foundational ones if the project benefits from a common runtime vocabulary. In that sense, Core is architecturally disciplined without being doctrinaire.

## Historical layering should stay legible
The package contains multiple eras of framework thinking. That is architecturally important, not accidental noise.

Some parts feel foundational and timeless. Some feel like compatibility bridges. Some feel like experiments that proved useful enough to stay. The right overview does not flatten those categories into one tone. It keeps the layering visible so future restructuring can happen intentionally rather than by forgetting why something exists.

## Extension pressure belongs here
Core is also where future runtime directions first become plausible.

If the project explores WebAssembly, Android, stronger single-thread runtime identities, richer daemon models, runtime modularity, alternative transport layers, or more radical CPU-portability work, those ambitions will eventually exert pressure on Core first. The package is where those tensions become architectural questions instead of isolated feature requests.

## The real challenge
The architectural challenge is not reducing Core to a fashionable minimum. It is maintaining a runtime center that stays coherent while still admitting history, specialization, and future direction.

That is a harder task, but it is also more honest.
