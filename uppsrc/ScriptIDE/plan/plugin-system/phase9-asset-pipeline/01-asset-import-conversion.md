# Task: Asset Import and Conversion Plan

## Goal
Define the pipeline for migrating card images and visual assets from the KDE Hearts tarball into a format usable by the ScriptIDE reference project.

## Background / Rationale
The original KDE Hearts game includes PNG images for all 52 cards plus card backs. These assets need to be extracted, potentially resized or converted to an `.iml` (Image Library) or standard directory structure, and made available to the `.form` layout editor and runtime.

## Scope
- Locating and extracting all necessary `.png` assets from `./tmp/hearts-1.98.tar.bz2`.
- Defining a target folder structure within the reference project (e.g., `examples/Hearts/assets/`).
- Documenting licensing and copyright expectations for the reused assets.
- Converting images into U++ `Image` formats or an `.iml` file.

## Non-goals
- Redrawing or creating new art assets.

## Dependencies
- `02-kde-hearts-source-audit.md`

## Concrete Investigation Steps
1. Review the files in `clients/human/pics/`.
2. Check `COPYING` and `README` in the tarball to confirm the GPL license applies to assets.
3. Determine if U++ `CtrlCore/Image` prefers standalone `.png` files loaded dynamically or baked into an `.iml` at compile time. Since plugins should ideally load dynamic content, standalone files might be preferred.

## Affected Subsystems
- Asset Pipeline
- Layout Engine (`.form` asset references)

## Implementation Direction
Create a shell script or a detailed manual process document that copies the assets, renames them consistently (e.g., `H2.png` to `hearts_2.png`), and creates a manifest for the layout editor. Include a copyright notice file in the target directory referencing the original KDE authors.

## Risks
- Hardcoding paths in the layout editor might break if the project is moved.

## Acceptance Criteria
- [ ] Defined asset extraction and conversion script/process.
- [ ] Documented target directory structure.
- [ ] Documented licensing/copyright handling.
- [ ] Verification plan for visual completeness (all 52 cards present).
