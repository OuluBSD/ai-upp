# Task: Design JavaScript Frontend for ByteVM

## Goal

Define the first JavaScript frontend that targets `ByteVM` without destabilizing the existing Python flow.

## Background / Rationale

The repository already has a working VM, plugin binding path, and `.gamestate` execution model. The missing piece is a JavaScript language frontend that can feed the runtime with equivalent callable modules and predictable state.

## Scope

- Tokenizer/parser plan for a constrained JavaScript subset
- AST-to-IR lowering strategy
- Runtime object model deltas needed in `ByteVM`
- Language-neutral module and entrypoint contract for `.gamestate`

## Proposed Phase-1 JavaScript Surface

Support only the subset needed by Klondike and the browser form host:
- `let` / `const`
- functions and closures
- arrays and plain objects
- `if`, `while`, `for`, `return`
- property access
- imports for local game modules
- host calls into a bound module such as `cardgame_view`

Explicitly defer:
- classes
- prototypes beyond basic object lookup
- `async` / `await`
- DOM APIs inside the VM
- npm/package ecosystem compatibility

## Required Runtime Work

1. Add language-neutral module loading:
   - `.gamestate` cannot remain hard-coded to Python `entry_script`.
   - Add fields such as `entry_language`, `entry_module`, and a normalized callable entry name.
2. Introduce a frontend interface:
   - parse
   - compile
   - load module
   - expose callable exports
3. Audit `PyValue` naming and ownership:
   - either generalize the value layer or isolate JavaScript-specific wrappers carefully.
4. Preserve existing Python behavior:
   - no regression for ScriptIDE, ScriptCLI, or Hearts.

## File Targets

- `uppsrc/ByteVM/`
- `uppsrc/ScriptCommon/CardGamePlugin.*`
- `uppsrc/ScriptCommon/RunManager.*`
- `.gamestate` schema docs and loaders

## Risks

- Reusing `PyValue` and `PyVM` naming may become misleading if JavaScript support grows.
- Directly emulating browser JavaScript is the wrong target; the VM should support game/frontend code, not the full web platform.

## Acceptance Criteria

- [ ] Defined JavaScript subset and exclusions
- [ ] Defined compiler/lowering entrypoints
- [ ] Defined language-neutral `.gamestate` execution contract
- [ ] Defined compatibility rule for existing Python `.gamestate` projects
