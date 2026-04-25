# Logging

## What this page is for
This page is about Core's logging culture.

Logging is not just output. It is the runtime's memory of what happened, what developers expected to happen, and what kinds of failure deserve to remain legible after the fact.

## Logging as discipline
Core's stance on logging fits its wider debug philosophy: the system should leave evidence.

This makes logging part of engineering discipline rather than an optional convenience. A runtime that values explicitness should also value traces that explain the consequences of that explicitness.

## Debug and release tell different truths
Core is comfortable with different levels of diagnostic expression in different build modes. That is philosophically consistent.

Debug builds are where the framework is allowed to be noisier, stricter, and more accusatory. Release builds should still preserve the traces that matter operationally, but they do not need to carry the full emotional intensity of development-time checking.

## Operational memory matters
A broad framework runtime cannot rely on the debugger alone. Once software is deployed, logging becomes its durable memory.

That is especially true in environments where:

- the UI is not the main story
- services run without direct supervision
- platform issues appear only on certain machines
- future daemon or local-service models become more important

Logging is therefore not a side concern. It is part of Core's long-term credibility.

## Future direction
The interesting future pressure here is integration:

- stronger connection between logs and profiling
- richer developer tooling
- cleaner stories for service processes and headless runtimes

If Core grows into more operationally complex environments, logging will need to become even more central as a narrative layer for the runtime.
