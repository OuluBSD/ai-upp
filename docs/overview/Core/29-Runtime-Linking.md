# Runtime Linking

## What this page is for
This page is about optionality and late binding.

Runtime linking support in Core matters because it expresses a particular architectural attitude: the framework wants room for optional dependencies, modular boundaries, and platform negotiation without forcing every decision to happen at static link time.

## Optionality as design freedom
Late binding is valuable because it lets a runtime remain adaptable.

It creates room for:

- optional capabilities
- platform-specific integration
- licensing or deployment flexibility
- gradual adoption of external features

This fits Core well. The package tends to value explicit mechanisms for real-world constraints more than clean-but-brittle purity.

## Not the same as a plugin ecosystem
Runtime linking should not be romanticized into a full modular platform story by itself. It is a lower-level capability than that.

Still, keeping it in Core matters because many richer modular designs depend on exactly this kind of underlying optionality. In that sense, runtime linking is one of the small infrastructural seeds from which larger extension models can grow.

## Future direction
This area could become more significant if the framework leans further into:

- optional subsystems
- dynamic service composition
- hot-reload or tool-assisted module workflows
- cleaner separation between baseline runtime and platform extras

Even if it stays modest, it preserves an important architectural possibility.
