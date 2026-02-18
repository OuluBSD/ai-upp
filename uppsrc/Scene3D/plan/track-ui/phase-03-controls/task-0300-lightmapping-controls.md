# Task 0300 - Lightmapping Control Wiring

## Status
- Partial / Open (updated 2026-02-17)

## Goal
Hook lightmapping form controls to bake settings and actions.

## Scope
- Bind lightmap mode, size, resolution, subsampling, and toggles.
- Wire Calculate button to baking pipeline.
- Persist settings per project.

## Acceptance
- Lightmapping tab reflects current bake settings.
- Calculate button triggers bake with selected options.

## Current State
- Ribbon lightmapping controls are wired to scene dynamic properties and synchronized to/from the active scene.
- Field edits persist immediately in-scene and Calculate stores the selected settings.

## Remaining
- Connect `calculate_lightmap` to the real bake pipeline (currently logs stub request).
