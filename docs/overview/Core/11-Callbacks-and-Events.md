# Callbacks And Events

## What this covers
This file documents the callable abstractions in Core and how the older callback vocabulary sits on top of the newer function wrapper.

## Current callable core: `Function`
[`uppsrc/Core/Function.h`](../../../uppsrc/Core/Function.h) defines:

- `Function<Signature>`
- `Event<Args...>` as `Function<void(Args...)>`
- `Gate<Args...>` as `Function<bool(Args...)>`
- `MemFn(...)` helper for binding methods

Implementation details visible in code:

- callables are type-erased behind a `WrapperBase`
- wrappers are reference-counted with `Atomic`
- chaining with `operator<<` builds a wrapper that calls the previous function and then the appended function

This is the current lambda-friendly callable mechanism.

## Compatibility layer: `Callback` and `Gate`
[`uppsrc/Core/Callback.h`](../../../uppsrc/Core/Callback.h) starts with:

- `// Backward compatibility; use Function/Event in the new code`

That comment is the clearest status signal in the package.

`CallbackN` and `GateN` are mostly thin wrappers around `Function<void(...)>` and `Function<bool(...)>`. The old macro ecosystem remains:

- `THISBACK`, `THISBACK1`, ...
- `PTEBACK`, `STDBACK`
- generated callback adapter templates in `CallbackR.i`, `CallbackN.i`, and `CallbackNP.i`

## Semantics
- `Function` is reference-counted, nullable, and callable like a modern function object
- `Event` is a naming alias for void-returning callbacks
- `Gate` is a naming alias for bool-returning callbacks
- `Callback` keeps old source compatibility and macro-heavy binding style

The package supports both because much of the codebase still uses the older naming and helper macros.

## Tradeoffs
- `Function` is easier to use with lambdas and generic code
- `Callback` preserves source compatibility and older idioms used across U++
- both are explicit callable objects; Core does not build a separate signal/slot framework at this layer

## Current vs legacy
`Function`/`Event`/`Gate` are current. `Callback` is a compatibility layer that still matters because the rest of the tree uses it heavily.

## See also
- [03-Threading.md](03-Threading.md)
- [06-Streams.md](06-Streams.md)
- [10-Recycling.md](10-Recycling.md)
- [README.md](README.md)
