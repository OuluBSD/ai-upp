# Task 0102 - Scenes and Publish Actions

## Status
- Done (2026-02-19)

## Goal
Bind Scenes and Publish tabs to real scene management and export flows.

## Scope
- Scene selector dropdown with add/delete/settings.
- Publish target dropdown.
- Publishing settings and publish/test actions.

## Acceptance
- Scene operations affect Scene3D document and tree.
- Publish actions trigger export or queue tasks.

## Notes
- Scene selector dropdown is synchronized to project scenes and can switch active scene or create a new scene.
- Scene delete/settings/metrics/post-effects actions now execute concrete behavior.
- Publish target writes to scene dynamic properties, and publishing settings persist export-directory metadata.
- Publish-and-test continues to trigger export execution flow.
