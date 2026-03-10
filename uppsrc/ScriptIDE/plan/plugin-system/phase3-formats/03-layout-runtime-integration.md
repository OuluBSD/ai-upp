# Task: Layout Runtime Integration Plan

## Goal
Define how `.xlay` layouts are loaded at runtime, rendered within the custom plugin view, and manipulated by Python logic via the `.gamestate` execution flow.

## Background / Rationale
Once a `.gamestate` file triggers execution, the IDE must spin up the Python VM, load the specified `.xlay` file into a visual representation within a new tab, and provide the Python script with a way to manipulate the items defined in the layout (e.g., "move card 1 to trick area").

## Scope
- Defining the compatibility boundary between the FormEditor-derived editor and the runtime `.xlay` loader.
- Defining how the runtime layout is attached to the custom `Ctrl` host.
- Defining how Python scripts reference named zones or objects in the `.xlay` layout.

## Non-goals
- Implementing the Python bindings (covered in the VM Bridge phase).

## Dependencies
- `.xlay` schema design.
- `.gamestate` schema design.

## Concrete Investigation Steps
1. Determine how U++ usually loads layout files dynamically (if applicable) and adapt that concept for the custom `.xlay` YAML parser.
2. Outline the steps a plugin must take when "Execute" is pressed on a `.gamestate`: 
   a. Parse `.gamestate`.
   b. Parse and render `.xlay` into the document host.
   c. Start Python script, passing the active layout context.

## Affected Subsystems
- Plugin System / Custom Document Host
- Plugin System / Execute Dispatch

## Implementation Direction
Produce a sequence diagram and a documented workflow detailing the handoff from `.gamestate` execution to `.xlay` rendering, and finally to Python script control.

## Risks
- Desynchronization between what the editor allows and what the runtime can render.

## Acceptance Criteria
- [ ] Documented layout/runtime integration plan between `.xlay`, `.gamestate`, and Python.
- [ ] Defined boundary between editor representation and runtime representation.
