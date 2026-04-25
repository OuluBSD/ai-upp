# Caching

## What this page is for
This page is about reuse of results, not just reuse of objects.

Caching in Core is interesting because it suggests the runtime wants a native opinion about when repeated work should be remembered rather than recomputed.

## Caching as policy
Caches are never neutral. They encode beliefs about memory pressure, recomputation cost, object identity, and what kinds of reuse are safe.

The current Core story appears relatively narrow, but that narrowness is itself informative. It suggests the package recognizes the need for caching while still being cautious about turning it into a giant universal subsystem.

## A topic worth keeping separate
Caching deserves its own page because it reflects a different kind of runtime thinking from recycling, containers, or serialization.

It is about memory and time, but it is really about policy: what should the runtime remember, for whom, and under what pressure. That is a distinct architectural question.

## Future direction
This is one of the places where Core could eventually become more ambitious:

- stronger cache policy stories
- better visibility into memory tradeoffs
- clearer coordination with value, serialization, and service layers

Even as a micro-topic, it preserves a useful line of thought.
