# Task: FormEditor Adaptation Plan

## Goal
Design the strategy for adapting `uppsrc/FormEditor` into a 2D card-table layout editor for `.form` files within the ScriptIDE plugin system.

## Background / Rationale
ScriptIDE needs a WYSIWYG editor for `.form` files to position card slots, hands, and
scoreboards visually. `uppsrc/FormEditor` is U++'s native UI designer and should stay
paired with the real `Form` runtime.

Correction:
- Earlier planning drifted toward treating `FormEditor` as only an editing canvas while
  runtime would use a separate JSON scene parser.
- That is what led to the current mismatch in `CardGameLayoutEditor` and
  `CardGameDocumentHost`.
- That fork should be reversed. Extending `FormEditor` is still the right direction, but
  its serialized format must remain compatible with the `Form` runtime.

## Scope
- Studying `uppsrc/FormEditor` architecture.
- Identifying reusable subsystems (property grid, selection/manipulation tools, snapping).
- Identifying required forks or rewrites while keeping compatibility with the `Form`
  runtime.
- Defining the compatibility boundary between the adapted editor and the runtime `.form` loader.
- Defining the property editing model for `.form` objects.

## Non-goals
- Building a full 3D editor.
- Integrating animation timelines (focus on static layouts first).

## Dependencies
- `02-form-schema-design.md`
- Access to `uppsrc/FormEditor` source.

## Concrete Investigation Steps
1. Inspect `uppsrc/FormEditor` to understand how it serializes layouts and manages the `Properties` window.
2. Determine if we can inject custom widget types (e.g., `CardPlaceholder`, `PlayerZone`)
   into the standard FormEditor, or if we need a distinct `GameLayoutEditor` fork that
   still serializes real `Form` data.
3. Outline the preview/runtime separation to ensure editor-specific gizmos (like bounding boxes) don't bleed into the actual game rendering.

## Affected Subsystems
- Plugin System / Layout Editor
- `uppsrc/FormEditor` (if extending) vs new `uppsrc/GameEditor` package

## Implementation Direction
Produce a design document detailing whether to extend `FormEditor` via plugins or fork it into a new package, and how the selection/manipulation model will handle the custom `.form` schema.

## Risks
- `FormEditor` might be too tightly coupled to existing U++ widgets.
- Writing a separate canvas is faster short-term, but we now know it causes editor/runtime
  divergence and duplicated layout logic.

## Acceptance Criteria
- [ ] Documented study of `FormEditor` reusability.
- [ ] Defined plan for property editing and scene manipulation.
- [ ] Documented boundary between editor and runtime representation.
