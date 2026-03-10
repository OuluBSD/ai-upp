# Task: FormEditor Adaptation Plan

## Goal
Design the strategy for adapting `uppsrc/FormEditor` into a 2D card-table layout editor for `.xlay` files within the ScriptIDE plugin system.

## Background / Rationale
ScriptIDE needs a WYSIWYG editor for `.xlay` files to position card slots, hands, and scoreboards visually. `uppsrc/FormEditor` is U++'s native UI designer, which is robust but tailored for standard GUI dialogs (buttons, text fields). It must be adapted or forked to handle 2D game scenes with custom object types and layers.

## Scope
- Studying `uppsrc/FormEditor` architecture.
- Identifying reusable subsystems (property grid, selection/manipulation tools, snapping).
- Identifying required forks or rewrites (switching from `Ctrl` layout to a custom scene graph or modified `Ctrl` hierarchy for sprites).
- Defining the compatibility boundary between the adapted editor and the runtime `.xlay` loader.
- Defining the property editing model for `.xlay` objects.

## Non-goals
- Building a full 3D editor.
- Integrating animation timelines (focus on static layouts first).

## Dependencies
- `02-xlay-schema-design.md`
- Access to `uppsrc/FormEditor` source.

## Concrete Investigation Steps
1. Inspect `uppsrc/FormEditor` to understand how it serializes layouts and manages the `Properties` window.
2. Determine if we can inject custom widget types (e.g., `CardPlaceholder`, `PlayerZone`) into the standard FormEditor, or if we need a distinct `GameLayoutEditor` fork.
3. Outline the preview/runtime separation to ensure editor-specific gizmos (like bounding boxes) don't bleed into the actual game rendering.

## Affected Subsystems
- Plugin System / Layout Editor
- `uppsrc/FormEditor` (if extending) vs new `uppsrc/GameEditor` package

## Implementation Direction
Produce a design document detailing whether to extend `FormEditor` via plugins or fork it into a new package, and how the selection/manipulation model will handle the custom `.xlay` schema.

## Risks
- `FormEditor` might be too tightly coupled to U++ `lay` files, making adaptation excessively difficult compared to writing a simple custom canvas.

## Acceptance Criteria
- [ ] Documented study of `FormEditor` reusability.
- [ ] Defined plan for property editing and scene manipulation.
- [ ] Documented boundary between editor and runtime representation.
