# Task: Layout Runtime Integration Plan

## Goal
Define how `.form` layouts are loaded at runtime, rendered by the real U++ `Form`
runtime, and manipulated by Python logic via the `.gamestate` execution flow.

## Background / Rationale
Once a `.gamestate` file triggers execution, the IDE must spin up the Python VM, load
the specified `.form` file into a live visual representation within a new tab, and
provide the Python script with a way to manipulate the items defined in the layout.

Correction:
- The current implementation does **not** use the U++ `Form` runtime.
- It parses `table.form` as custom JSON in `CardGameDocumentHost::SetLayout()`.
- `CardGameLayoutEditor` also serializes a custom JSON scene, even though it embeds
  `FormEdit<ParentCtrl>`.
- This diverged from `bazaar/FormExample`, which uses `FormWindow form; form.Load(file); form.Layout("Default");`.

That divergence is the main reason the Hearts table is not following the same renderer
path as real `.form` content.

## Scope
- Defining the compatibility boundary between the FormEditor-derived editor and the runtime `.form` loader.
- Defining how the runtime layout is attached to the custom `Ctrl` host.
- Defining how Python scripts reference named zones or objects in the `.form` layout.

## Non-goals
- Implementing the Python bindings (covered in the VM Bridge phase).

## Dependencies
- `.form` schema design.
- `.gamestate` schema design.

## Concrete Investigation Steps
1. Determine how U++ usually loads layout files dynamically and use that path directly.
2. Outline the steps a plugin must take when "Execute" is pressed on a `.gamestate`: 
   a. Parse `.gamestate`.
   b. Load and render `.form` through the `Form` package runtime.
   c. Start Python script, passing the active layout context.

## Affected Subsystems
- Plugin System / Custom Document Host
- Plugin System / Execute Dispatch

## Implementation Direction
Produce a sequence diagram and a documented workflow detailing the handoff from
`.gamestate` execution to real `Form` runtime loading, and finally to Python script
control.

## Corrective Direction
The earlier JSON scene path was an expedient side fork introduced by the plugin-system
track. It should be treated as a temporary compatibility layer, not the target
architecture.

Target direction:
1. `.form` means real `Form` XML format.
2. `.gamestate` hosts a live `Form`/`FormWindow`-style view, not a manual zone parser.
3. Card-game-specific overlays and bindings sit on top of the `Form` runtime instead of
   replacing it.

## Risks
- Desynchronization between what the editor allows and what the runtime can render.
- Continuing the JSON fork would preserve editor/runtime mismatch and duplicate layout
  logic indefinitely.

## Acceptance Criteria
- [ ] Documented layout/runtime integration plan between `.form`, `.gamestate`, and Python.
- [ ] Defined boundary between editor representation and runtime representation.
