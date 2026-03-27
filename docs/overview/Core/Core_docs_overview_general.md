# Core Package Overview

## What this document is
This document is not a strict technical specification.

It is a higher-level overview of the Core package's design intent, direction, and longer-term extension themes, including ideas that go beyond the current implementation in `uppsrc/Core/`.

For code-verified behavior, use the numbered overview files in this directory, especially:

- `00-Core-Philosophy.md`
- `01-Architecture.md`
- `03-Threading.md`
- `21-Compression.md`
- `23-CoWork.md`
- `26-Hashing.md`
- `27-Networking.md`
- `28-Daemon.md`
- `29-Runtime-Linking.md`
- `30-Windows-Specific.md`

## Philosophy and mindset
Core is not framed as a minimal academic runtime. Its general direction is pragmatic, performance-aware, and explicit about tradeoffs.

Recurring themes across the package and surrounding codebase include:

- explicit semantics over convenience
- direct control over ownership, memory, and execution behavior
- debug-time strictness with release-time survivability
- platform abstraction without pretending all platforms behave the same

## Relation to STL and system libraries
Core does not try to mirror STL design directly.

The broader direction is to:

- provide framework-specific semantics where those semantics matter
- keep system APIs available underneath the abstraction layer
- prefer predictable behavior over maximum generality

That does not mean "reject STL everywhere." It means Core keeps its own runtime model where the project considers that model important.

## Platform direction
In current practice, Core primarily targets mainstream desktop and systems environments, especially Windows and POSIX-class platforms.

At the level of design direction, the package also leaves room for:

- reduced or compatibility-oriented platform subsets
- single-threaded or constrained environments
- experimental targets where only part of the full runtime is practical

These should be read as extension directions, not guarantees that every target is currently complete or equally supported.

## Portability approach
Core generally follows this portability model:

- one public API surface where practical
- platform-specific implementation branches where necessary
- explicit fallback paths when full support is unavailable

The intention is shared usage patterns, not fake behavioral uniformity.

## What Core is meant to enable
As a design layer, Core exists to support higher packages that need a coherent non-GUI runtime foundation.

That includes, in broad terms:

- application infrastructure
- tooling and IDE-style programs
- serialization and configuration pipelines
- networking and service layers
- experimental framework extensions built on the same runtime assumptions

## Accepted tradeoffs
The general direction of the package accepts:

- complexity when it makes ownership or execution behavior clearer
- custom abstractions when standard ones do not express the intended semantics well
- compatibility layers that remain in the tree because the wider codebase still depends on them

It generally pushes back against:

- hidden behavior
- unnecessary abstraction layers
- portability claims that hide real platform differences

## Known rough edges
Because Core is long-lived and broad in scope, the package naturally carries some unevenness:

- older and newer subsystems coexist
- some areas are central while others are specialized or compatibility-oriented
- platform support depth varies by subsystem
- extension-oriented code can sit beside foundational runtime code

This is part of the package's actual character and should be understood as such.

## Direction and potential extensions
The following themes are better read as possible directions than as statements of finished functionality:

### Profiling and diagnostics
- richer tooling around timing and runtime diagnostics
- tighter integration with developer-facing analysis tools

### Networking
- clearer transport specialization where needed
- possible future expansion beyond the current TCP-centered Core networking layer

### Scheduling and work distribution
- more flexible work scheduling than the current single-pool `CoWork` model

### Localization and data interchange
- broader reuse of translation assets
- tighter alignment between serialization, conversion, and visitor-style flows

### Runtime modularity
- smoother workflows around runtime linking and optional components

### Platform evolution
- refinement of constrained-platform support where it is valuable to the project

## Final note
This file describes intent and direction.

It is useful for understanding how the package is meant to evolve, but it should not be treated as the authoritative source for concrete behavior. The numbered Core overview documents are the authoritative technical companion to this broader overview.
