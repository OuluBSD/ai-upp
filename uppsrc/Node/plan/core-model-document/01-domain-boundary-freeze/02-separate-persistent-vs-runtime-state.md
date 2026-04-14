# Separate Persistent vs Runtime State

## Purpose
Define which fields belong to persisted document data versus runtime scene/editor state so the new model avoids GraphLib-style coupling.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Classify existing GraphLib Node/Edge/Group fields into document or runtime buckets
- Define runtime state containers for selection and transient animation flags
- Define explicit migration notes for deprecated mixed fields

## Out of Scope
- Implementing command stack behavior
- Building Ctrl adapters
- Final schema encoding details

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/01-domain-boundary-freeze/01-define-stable-id-model.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Graph.h`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Use a table-driven classification to avoid hidden fields
- Call out fields that must be dropped or synthesized at load time
- Preserve backward compatibility through adapter layer instead of polluting core model
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Persistent/runtime split is documented for every migrated GraphLib field
- [ ] Runtime-only state has no persistence requirement
- [ ] Design explicitly preserves Core/Ctrl separation

## Suggested Validation
- design review checklist
- compile checks for Node/Core
