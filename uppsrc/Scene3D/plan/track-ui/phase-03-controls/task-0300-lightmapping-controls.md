# Task 0300 - Lightmapping Control Wiring

## Status
- Done (2026-02-19)

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
- Calculate now triggers a concrete bake path:
  - Generates/updates `TEXTYPE_LIGHTMAP` textures for model materials in the active scene.
  - Applies scene bake settings (mode/size/subsampling/resolution/shadow/toggles) into generated output.
  - Updates scene metadata (`lightmap_last_bake_*`) and refreshes renderers.
