# Task: Audit ByteVM JavaScript + Browser Path

## Goal

Establish whether the requested feature should be built as:
- a true JavaScript frontend that compiles into existing `ByteVM` execution structures, or
- a sidecar JavaScript runtime that only reuses ScriptIDE/plugin infrastructure.

The preferred direction is the first one, but the decision must be explicit because `ByteVM` is Python-only today.

## Investigated Current State

- `uppsrc/ByteVM/PyVM.*` and related compiler files implement a Python pipeline only.
- `ScriptCommon/CardGamePlugin` executes `.gamestate` by loading Python modules and calling a Python `entry_function`.
- `reference/Hearts/game.gamestate` is the concrete reference project today.
- `reference/Solitaire` is missing, so Klondike must be planned as a net-new reference project.
- `uppsrc/Skylark` already provides a server framework and the repo has concrete references in `reference/Skylark01` through `reference/Skylark12` plus `reference/SkylarkUpload`.
- The browser host should therefore follow `Skylark` patterns instead of inventing a custom HTTP loop inside ScriptIDE.

## Scope

- Audit the current runtime boundary between parser/compiler/IR/VM.
- Decide what “JavaScript frontend to ByteVM” means in code terms.
- Define the browser-hosted execution model for `.gamestate`.
- Define the process boundary between ScriptIDE and the separate web host program.

## Non-goals

- Implementing the JavaScript parser
- Implementing the separate web host
- Porting Hearts to JavaScript

## Key Decision To Make

The track must commit to one of these:

1. `ByteVM` multi-frontend model:
   - Add JavaScript tokenizer/parser/compiler stages.
   - Lower JavaScript into the same or extended IR used by `PyVM`.
   - Keep execution, bindings, and scheduling inside one VM family.
2. Split-runtime model:
   - Keep `ByteVM` Python-only.
   - Add a separate JavaScript engine path.
   - Reuse only plugin/document/web-host infrastructure.

Recommendation from current codebase shape: prefer the multi-frontend model, otherwise the request is not really “JavaScript frontend to ByteVM”.

## Risks

- JavaScript semantics differ sharply from Python semantics around objects, prototypes, truthiness, `this`, closures, and module loading.
- If the plan overpromises source compatibility with browser JavaScript too early, implementation cost will explode.
- `.gamestate` currently assumes Python entrypoints; that contract must be generalized before any browser host can be clean.
- Embedding the server into ScriptIDE would couple process lifetime and GUI responsiveness unnecessarily.

## Acceptance Criteria

- [ ] Documented decision on ByteVM multi-frontend vs split-runtime
- [ ] Clear statement of which JavaScript subset is supported in phase 1
- [ ] Concrete list of runtime contracts that must become language-neutral
- [ ] Documented decision that the browser host is a separate Skylark-based program
