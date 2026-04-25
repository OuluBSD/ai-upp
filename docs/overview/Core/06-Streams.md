# Streams

## What this page is for
This page is about streams as a mental model.

In Core, streams are not just an I/O feature. They are one of the package's preferred ways of thinking about data movement, persistence, interchange, and object boundaries.

## Streams as the common language of movement
A framework runtime has to decide how values travel. Core's answer is often stream-shaped.

That choice has consequences:

- serialization feels procedural instead of magical
- storage and transport share a common conceptual path
- binary layout remains close enough to the surface to stay discussable

This is less fashionable than ever-more-automatic reflection systems, but it keeps the runtime's dataflow legible.

## A bias toward explicit transport
Core's stream culture reflects the package's suspicion of hidden formatting layers and overly abstract persistence systems.

The stream approach says: data should move through a visible channel, under visible mode, with visible structure. That is philosophically consistent with the rest of Core. It reduces surprise at the cost of asking developers to think more concretely.

## Why this still matters
Streams are one of the reasons Core feels like a full runtime rather than a library bucket. They give the package a default answer to the question "how should things pass through the system?"

That answer influences files, in-memory transformation, serialization, caching, compression, and various tooling stories. Even when newer abstractions are added later, they are often healthiest when they still respect this stream-first culture.

## Future direction
The real future question is not whether streams should be replaced. It is whether Core can extend the stream mindset into richer environments without losing clarity:

- asynchronous or service-oriented boundaries
- browser-hosted or constrained runtimes
- more dynamic reflective pipelines

If it can, streams stay foundational. If not, this is one of the places where the next architectural tension will surface.
