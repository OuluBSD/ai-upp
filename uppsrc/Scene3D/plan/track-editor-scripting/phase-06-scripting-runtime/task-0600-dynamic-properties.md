# Task 0600 - Dynamic Properties

## Goal
Allow ActionScript-style dynamic properties on scene objects.

## Scope
- Script-side property bag per node.
- Serialization for properties.
- Access via dot notation in scripts (e.g., `obj.health = 100`).

## Success Criteria
- Properties persist and can be used in scripts.

## Status
- Done

## Notes
- Added `GeomDynamicProperties` component and serialization.
- `DisplayObject` now stores unknown attributes in a property bag.
