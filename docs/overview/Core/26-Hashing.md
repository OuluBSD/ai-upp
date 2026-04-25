# Hashing

## What this page is for
This page is about identity, equivalence, and trust boundaries.

Hashing in Core matters because the runtime needs ways to talk about structure, lookup, signatures, and compact identity without confusing those roles with each other.

## Different kinds of sameness
One of the healthier lessons here is that not all hashes mean the same thing.

Structural hashes, digest-style hashes, and transport-oriented hashes answer different questions. Core is better when it keeps those distinctions visible. That is consistent with its broader refusal to collapse unlike concerns into one comfortable abstraction.

## Why this belongs in Core
Hashing is foundational because containers, caches, value systems, serialization, and networking all eventually depend on some notion of stable derived identity.

Once those notions fragment, the runtime becomes harder to reason about. Keeping hashing close to the center helps preserve conceptual continuity across many subsystems.

## Future direction
The future pressure here includes:

- better clarity around trust and security use cases
- broader architecture validation across endian and CPU variation
- more deliberate coordination with caching and serialization

Hashing may look like a narrow utility topic, but it sits quietly under a great deal of framework behavior.
