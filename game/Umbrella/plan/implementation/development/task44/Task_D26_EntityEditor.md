# Task D26: Entity Editor — Visual Sprite/Animation Editing

## Priority: MEDIUM — Tier 2 (content authoring)

## Overview

Separate editor screen for creating and editing entity definitions (sprites,
animations, hitboxes). Allows designers to import sprite sheets, define animation
frames, set hitboxes visually, preview animations, and export entity JSON.

## Java Reference
- `editor/entity/EntityEditorScreen.java`
- `editor/entity/PreviewList.java`
- `editor/TextureListPanel.java`

## Design

### Editor Layout
```
+--------------------------------------------------+
| [Menu Bar]                                       |
+----------+---------------------------+-----------+
| Entity   | Sprite Sheet View         | Animation |
| List     | (zoomed, with grid)       | Preview   |
|          | Click to select frames    |           |
| [New]    | Frame selection highlight  | [Play]    |
| [Delete] |                           | [Stop]    |
|          |                           | FPS: [30] |
+----------+---------------------------+-----------+
| Animation Timeline                               |
| [idle] [walk] [jump] [attack]  + Add Animation   |
| Frames: [0][1][2][3]  Duration: [0.1]  Loop: [x] |
+--------------------------------------------------+
| Hitbox Editor (overlay on sprite)                 |
| Drag to define hitbox rectangle                   |
+--------------------------------------------------+
```

### Features
1. Import sprite sheet PNG
2. Define frame grid (width/height)
3. Click frames to add to animation sequence
4. Name animations (idle, walk, jump, etc.)
5. Set frame duration and loop flag
6. Preview animation playback
7. Define hitbox rectangle visually
8. Export to entity definition JSON

## Implementation Steps

1. **EntityEditorApp** — `EntityEditor.h/.cpp`:
   - DockWindow-based layout
   - Sprite sheet viewer with zoom and grid overlay
   - Frame selection (click to select, drag to multi-select)

2. **Animation timeline panel**:
   - List of named animations
   - Frame sequence editor (reorder, add, remove)
   - Duration and loop controls

3. **Animation preview**:
   - Real-time playback of selected animation
   - Play/stop/step controls
   - FPS control

4. **Hitbox editor**:
   - Overlay on sprite view
   - Drag to define rectangle
   - Per-frame or per-entity hitbox

5. **JSON export/import**:
   - Save to EntityDefinition JSON format
   - Load existing definitions for editing

## Files to Create
- `game/Umbrella/EntityEditor.h` / `EntityEditor.cpp`

## Files to Modify
- `game/Umbrella/MapEditor.cpp` — menu item to open entity editor
- `game/Umbrella/Umbrella.upp` — add new files

## Dependencies
- D20 (Sprite/Animation System) — entity definition format must exist
- D21 (Texture Catalog) — texture loading infrastructure

## Acceptance Criteria
- [ ] Can import PNG sprite sheet
- [ ] Grid overlay shows frame boundaries
- [ ] Can define named animations with frame sequences
- [ ] Animation preview plays back correctly
- [ ] Can define hitbox rectangle
- [ ] Exports valid entity definition JSON
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
