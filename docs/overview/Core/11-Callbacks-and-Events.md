# Callbacks And Events

## What this page is for
This page is about callable infrastructure as a sign of historical layering.

Callback systems are rarely glamorous, but they often reveal how a framework transitions between generations of style without breaking itself apart.

## A bridge between eras
Core's callable layers suggest a package that has evolved rather than restarted.

That matters. A runtime that survived multiple idioms usually contains bridges, compatibility stories, and a mixture of old and preferred ways of expressing deferred work. The correct overview should treat that as evidence of continuity, not merely clutter.

## Why this belongs in Core
Callbacks and events sit close to the foundation because the rest of the framework needs a common language for deferred action, signaling, and glue code.

If that language fragments, higher packages become harder to reason about together. Keeping callable infrastructure in Core therefore protects architectural coherence even when the details are historically layered.

## Future direction
The long-term challenge is not to erase history aggressively. It is to make the preferred direction obvious while preserving enough bridge infrastructure that the wider tree remains usable.

Core should continue to distinguish:

- what is modern preference
- what is compatibility support
- what might eventually be retired

That kind of honesty is more useful than pretending the callable story is perfectly unified.
