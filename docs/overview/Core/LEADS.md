# Core / Platform Leads

## What this document is

This is **not** a strict code-backed reference.

This file collects:
- future directions
- extension targets
- architectural leads
- ideas that may be partial, experimental, dirty, or not yet implemented
- places where the current code and the intended platform direction do not yet match

The purpose is to keep important possible directions **visible** instead of letting them disappear into chat history, TODO comments, or memory.

---

## Why this file exists

The verified overview docs should describe what is currently present in code.

This file serves a different role:
- preserve platform vision
- preserve expansion targets
- preserve promising unfinished ideas
- preserve mismatches between current implementation and desired direction

In short:

- `docs/overview/` reference files = what is there now
- `docs/overview/LEADS.md` = where this may go next

---

## Core philosophical leads

### Keep Core as a platform layer, not just a utility package
Core should remain:
- a runtime foundation
- a portability layer
- a semantic alternative to raw STL-centric style
- a place where higher-level systems can attach cleanly

### Preserve explicit semantics
Continue preferring:
- explicit ownership
- explicit copy vs move behavior
- visible tradeoffs
- fewer hidden costs

### Preserve debug/release duality
Keep the current philosophy visible:
- debug should fail early and loudly
- release should keep running where safely possible
- corruption is worse than crashing

### Keep platform abstraction honest
Goal:
- one API surface where practical
- different implementations where platforms differ
- no fake universalism

---

## Platform and target leads

These are not all equally mature. Some are already partially supported, some are only direction markers.

### WebAssembly
Possible direction:
- single-thread Core builds
- single-thread GUI model
- browser-hosted tools
- limited IDE or scripting environments in WASM

Needs:
- better validation of ST mode
- audit of thread assumptions
- filesystem/runtime adaptation
- rendering and event-loop strategy

### FreeDOS / DOS-like targets
Possible direction:
- single-thread runtime
- direct framebuffer / VGA-style UI
- low-level graphical experiments
- retro-compatible tool environments

Needs:
- minimal graphics/output path
- stricter memory/runtime assumptions
- reduced dependency surface
- possible text-mode / framebuffer abstractions

### BSD and wider POSIX
Direction:
- maintain strong POSIX portability
- treat Linux as only one POSIX family member
- avoid Linux-only assumptions where unnecessary

Needs:
- periodic portability audits
- filesystem / dynamic loading / process checks
- socket and thread behavior review

### Android
Direction:
- preserve support as a serious platform target, not just accidental compatibility

Needs:
- regular build verification
- runtime/input/UI review
- packaging and deployment sanity

### Windows ecosystem breadth
Direction:
- preserve support from classic desktop Windows toward newer forms
- retain practical UWP / managed Windows compatibility where useful
- keep portability to Visual Studio workflows possible

Needs:
- reduce project-file friction
- validate runtime-linking and packaging paths

### Jailbroken consoles
Possible direction:
- experimental ports to constrained or unusual gaming hardware
- custom UI/runtime experiments
- niche deployment targets for tools or visual apps

Needs:
- target-by-target feasibility review
- graphics/input/runtime adaptation
- custom build/toolchain work

### Scientific calculators and exotic small systems
Possible direction:
- extreme lightweight builds
- stripped-down Core subsets
- educational or niche embedded tools

Needs:
- tiny-memory profile
- reduced runtime assumptions
- minimal I/O strategy
- likely single-thread-only profile

### Wider CPU targets
Possible direction:
- keep x86/x64 and ARM strong
- expand or preserve support for:
  - PowerPC
  - Altivec
  - additional SIMD families
  - unusual big-endian environments

Needs:
- platform-specific testing
- endian testing
- SIMD abstraction review
- compile/test coverage on rare hardware

### Big-endian validation as a deliberate quality target
This should remain visible as a real testing goal, not trivia.

Reason:
- it exposes hidden assumptions
- it improves serialization, hashing, memory, and protocol quality
- it catches lazy little-endian bias

---

## Core technical extension leads

### Better IDE-integrated profiling
A major lead already identified.

Direction:
- parse profiling output automatically in the IDE
- show timing tables
- show hot paths
- show real-time graphs
- show timelines while the program runs
- possibly show per-thread activity

This is not just a feature. It upgrades the platform’s observability.

### CoWork evolution
Current direction to explore:
- multiple pools instead of one global execution pool
- fairer scheduling between long queues
- better partitioning of work classes
- more deliberate concurrency models

### Networking expansion
Important lead:
- undo the newer simplification where `Socket` effectively means TCP only
- restore clearer naming and separation
- introduce or reintroduce:
  - explicit TCP socket
  - UDP socket
  - ENet-based socket / persistent UDP layer

Reason:
different network semantics matter and should stay visible in the API.

### ENet integration
Direction:
- use ENet-style persistent UDP transport where suitable
- support real-time or lower-latency communication patterns
- allow alternatives to ordinary TCP assumptions

### Daemon / service infrastructure
Direction:
- keep open the possibility of internal service models
- RPC-style services
- local server/client coordination
- MCP-like service integration for AI agents
- debugging-oriented service exposure

Needs:
- security boundaries
- default-off policy
- clear local-only vs network-visible modes

### Runtime linking / plugin architecture
Direction:
- strengthen binary-level extensibility
- load modules/plugins without hard link-time dependency
- support post-deployment extension
- support licensing separation where necessary

Needs:
- reliable discovery/loading patterns
- hot-reload discipline
- versioning and ABI conventions

### Visitor-based introspection
Direction:
- use visitor/access systems more broadly as a generic reflection substitute
- object graph hashing
- debug tree views
- generic serializers
- script/runtime object bridging

Potential:
this could become a major internal protocol for tooling.

### Serialization unification
Direction:
- bring XML/JSON/visitor-based serialization ideas into a clearer shared model
- reduce duplicated serialization logic
- make inspection/export/import tooling easier to build

### Caching strategy
Direction:
- expand reusable cache patterns
- make value-based cache strategies more visible and reusable
- clarify invalidation and policy patterns

---

## UI / geometry / rendering leads

### Keep basic UI geometry separate from 3D math
This should remain a deliberate rule.

Direction:
- keep `Point`, `Size`, `Rect`, `Volume` style primitives lightweight
- keep UI/screen-coordinate semantics distinct
- do not force GLM-style geometry into basic Core primitives

### VR / volumetric UI direction
The addition of `Volume` suggests a broader lead:
- virtual reality UIs
- spatial editor tools
- 3D-aware but not necessarily fully 3D-renderer-bound interfaces

Needs:
- clearer boundary between Core geometry and 3D math packages
- rendering and input plans outside basic Core

### Theme-aware color semantics
Direction:
- continue making color behavior semantic, not only literal
- support light/dark adaptation cleanly
- allow function-based or dynamic color roles where useful

---

## Localization and language leads

### Treat localization as a platform feature, not a GUI extra
Direction:
- preserve translation support in console and GUI applications
- make it easy to localize tools and runtimes everywhere

### Shared translation libraries
Possible future direction:
- reusable translation sets across multiple programs
- shared language packs
- more deliberate context reuse

Open question:
how to keep this practical without making prototyping painful.

---

## Tooling and developer-experience leads

### Preserve log-first diagnostics
Direction:
- keep log files and IDE log integration as first-class diagnostics
- avoid falling back to stdout-centric habits in GUI tools

### Better AI-agent ergonomics
Direction:
- make agent-friendly logging and instrumentation easier
- reduce friction between Core conventions and external automation/tools
- possibly expose more structured diagnostics

### Keep examples meaningful
Lead:
- examples should increasingly show real best practices, not only tiny demos
- especially around:
  - UTC vs local time
  - serialization
  - logging
  - threading
  - platform differences

---

## Safety and systems leads

### Keep safer pointer semantics available
Direction:
- continue preferring tracked pointer patterns over raw long-lived pointers
- make object-lifetime failures visible early

### Preserve and improve internal memory diagnostics
Direction:
- keep deterministic allocation-tracking workflows
- continue making memory issues easy to isolate before resorting to heavier external tools

### Single-thread mode as a real systems profile
This should remain visible as more than a relic.

Potential uses:
- WASM
- small systems
- DOS-like targets
- deterministic environments
- simplified GUI experiments

Needs:
- real maintenance
- audits for hidden thread assumptions

---

## Documentation leads

### Keep two documentation layers
This should be explicit project policy.

1. **Verified overview/reference layer**
   - what current code clearly does

2. **Vision / leads layer**
   - where the platform may grow
   - what is intended
   - what is unfinished
   - what mismatches current code

Without the second layer, important directions disappear.

### Track mismatches explicitly
Examples:
- places where upstream changed a semantic choice and the fork may reverse it
- places where current code only partially supports the intended model
- places where the design direction is clear but implementation is dirty

---

## Policy hints: what is acceptable to build here

Generally acceptable:
- new platform experiments
- explicit-performance work
- infrastructure for scripting/tooling
- plugin/dynamic-loading systems
- better introspection and profiling
- practical low-level experiments
- niche target ports if they teach the platform something useful

Needs caution:
- network-exposed services
- insecure-by-default protocols
- ABI-sensitive plugin work
- platform additions without ongoing test strategy

Should not be hidden:
- rough ideas with real leverage
- ugly but important experiments
- targets that are not production-ready yet
- reversals of upstream design decisions

---

## Open mismatch list

This section is intentionally rough.

### Socket naming and protocol coverage
Current code/documentation may reflect a simplified TCP-centric model.
Desired direction may require:
- TCP named explicitly
- UDP added explicitly
- ENet/persistent UDP added explicitly

### Single-thread support
Still conceptually valuable, but may not be maintained evenly everywhere.

### Profiling UX
Instrumentation exists, but IDE-level visibility still lags behind platform ambition.

### Shared translation infrastructure
Translation support exists, but reusable cross-program translation libraries are not yet a clear system.

### Multi-pool concurrency
CoWork exists, but richer concurrency orchestration remains open.

### Dynamic service layer
Daemon/service ideas exist, but there is not yet a single clean, modern, safe service architecture.

### Exotic platform targets
Interesting and aligned with the platform spirit, but mostly not consolidated into a maintained port matrix.

---

## Suggested maintenance rule for this file

When a new idea appears that affects:
- portability
- architecture
- extension points
- runtime behavior
- major tooling
- future platform targets

it should be added here briefly, even if immature.

The point is not to guarantee delivery.

The point is to prevent strategic ideas from vanishing.

---

## Final note

A platform like this does not live by cleanliness alone.

It lives by:
- strong foundations
- visible tradeoffs
- memory of where it still wants to go

This file exists so the future does not get silently optimized away.
