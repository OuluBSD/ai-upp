# Task: `.form` Schema Design

## Goal
Design the JSON schema for `.form` files, which define 2D scenes and table layouts for card games.

## Background / Rationale
To separate visual layout from game logic, we need a declarative layout format (`.form`). This format must be understandable by both a visual editor (FormEditor-derived) and a runtime renderer that the Python VM will interact with.

## Scope
- Defining the JSON schema for 2D scene/table layout.
- Designing support for zones (hands, trick area, score area, deck/discard, player seats).
- Designing support for object types, properties, z-order/layering.
- Designing anchors, coordinates, scaling, snapping, and guides.
- Defining references to assets (sprites, card backs/fronts).

## Non-goals
- Implementing the runtime renderer in this task.
- Implementing the FormEditor adaptation in this task.

## Dependencies
- `uppsrc/Core` JSON capabilities.

## Concrete Investigation Steps
1. Review the UI needs of the KDE Hearts reference (table layout, card positions, score box).
2. Look at how U++ `lay` (layout) files are structured internally (though `.form` will be JSON).
3. Draft an initial JSON schema accommodating zones, anchors, and sprites.

## Affected Subsystems
- Plugin System / Layout Engine

## Implementation Direction
Produce a documented JSON schema and an example `.form` file representing a basic 4-player card table.

## Risks
- Making the schema too rigid could limit it to Hearts-only, while making it too generic could complicate the editor implementation.

## Acceptance Criteria
- [ ] Documented JSON schema for `.form`.
- [ ] Schema covers zones, object properties, layering, anchors, and asset references.
- [ ] Example `.form` file provided.
