# Design Scene Descriptor Types

## Purpose
Define the scene descriptor model that represents paintable graph visuals independently from Ctrl controls.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define descriptor types for node glyphs pin glyphs edge visuals labels and groups
- Define style references and resolved style payload fields
- Define scene-level metadata for culling and hit testing

## Out of Scope
- Ctrl paint function code
- Layout algorithm implementation
- Persistence schema changes

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/01-coordinate-system-spec/02-define-viewport-transform-api.md`
- `./uppsrc/Node/plan/core-model-document/03-core-model-implementation/01-implement-core-document-entities.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Renderer.cpp`

## Implementation Notes
- Scene descriptors should be immutable snapshots for paint frame generation
- Avoid embedding Ctrl objects or callbacks in descriptors
- Plan descriptor evolution for future widget slots overlays and annotations
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Descriptor model covers all visuals needed by migrated GraphLib tutorials
- [ ] Descriptors are serializable for debug dumps if needed
- [ ] Core/Ctrl boundary remains explicit

## Suggested Validation
- unit tests for descriptor construction
- compile checks
