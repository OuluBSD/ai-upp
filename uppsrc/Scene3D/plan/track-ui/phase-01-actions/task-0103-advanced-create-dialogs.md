# Task 0103 - Advanced Create Dialogs

## Status
- Done (2026-02-19)

## Goal
Extend create-action dialogs with additional fields from the spec and persist them to scene object metadata.

## Scope
- Terrain dialog: create trees/grass options and extra props.
- Room mesh dialog: ceiling type, wall height, ceiling texture path (placeholder UI).
- Persist dialog fields into dynamic properties for later use.

## Acceptance
- Advanced fields appear in dialogs with reasonable defaults.
- Values are stored on created objects for downstream systems.

## Notes
- Terrain dialog includes topology, max height, and create-trees/grass options.
- Room mesh dialog includes ceiling type, wall height, and ceiling texture placeholder path.
- Values persist to created object metadata via dynamic properties for downstream use.
