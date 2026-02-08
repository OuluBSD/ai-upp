# Map Editor Gap Analysis

**Date**: 2026-02-08
**Comparison**: Current U++ MapEditor vs UX_MAP_EDITOR.md requirements

---

## Executive Summary

The current U++ MapEditor implementation has basic functionality (tile painting, layers, reference images), but is missing many features from the UX specification. This document identifies gaps and proposes an implementation roadmap.

---

## Feature Comparison Table

| **Feature Category** | **UX Spec** | **Current Implementation** | **Status** | **Priority** |
|---------------------|-------------|---------------------------|------------|--------------|
| **Navigation & Viewport** |
| Arrow key panning | ✓ | ✗ | MISSING | HIGH |
| Pan speed scaling with zoom | ✓ | ✗ | MISSING | MEDIUM |
| Zoom (+/-) | ✓ | ✓ (toolbar buttons) | PARTIAL | HIGH |
| Middle mouse pan | ✓ | ✓ | COMPLETE | - |
| Spacebar + drag pan | ✓ | ✗ | MISSING | MEDIUM |
| Fit reference (F) | ✓ | ✗ | MISSING | LOW |
| 100% zoom (1/Z) | ✓ | ✗ | MISSING | LOW |
| Reset pan/zoom (R) | ✓ | ✓ (toolbar button) | PARTIAL | MEDIUM |
| **Reference Image** |
| Load reference image | ✓ | ✓ | COMPLETE | - |
| Shift+Arrow offset | ✓ | ✗ | MISSING | MEDIUM |
| Reference opacity slider | ✓ | ✓ | COMPLETE | - |
| Ctrl+Shift+Arrow snap | ✓ | ✗ | MISSING | LOW |
| Zero offset (0) | ✓ | ✗ | MISSING | LOW |
| **Grid & Display** |
| Grid toggle (G) | ✓ | ✓ (menu) | PARTIAL | HIGH |
| **Tools & Painting** |
| Tool shortcuts (1-6) | ✓ | ✗ | MISSING | HIGH |
| Draw Wall (1) | ✓ | ✓ (button) | PARTIAL | HIGH |
| Erase Wall (2) | ✓ | ✓ (button) | PARTIAL | HIGH |
| Draw Background (3) | ✓ | ✓ (button) | PARTIAL | HIGH |
| Erase Background (4) | ✓ | ✓ (button) | PARTIAL | HIGH |
| Enemy Add (5) | ✓ | ✗ | MISSING | HIGH |
| Enemy Remove (6) | ✓ | ✗ | MISSING | HIGH |
| Right-click for paired erase | ✓ | ✗ | MISSING | MEDIUM |
| Alt+Click eyedropper | ✓ | ✗ | MISSING | LOW |
| Shift+drag constrain | ✓ | ✗ | MISSING | MEDIUM |
| Double-click flood fill | ✓ | ✓ (Fill tool) | PARTIAL | HIGH |
| Brush size adjust (\[/\]) | ✓ | ✓ (toolbar buttons) | PARTIAL | MEDIUM |
| Line/rectangle modes | ✓ | ✗ | MISSING | LOW |
| Distance overlay | ✓ | ✗ | MISSING | LOW |
| **Entity Placement** |
| Droplet spawn points | ✓ | ✗ | MISSING | HIGH |
| Enemy spawn points | ✓ | ✗ | MISSING | HIGH |
| Ctrl+drag line placement | ✓ | ✗ | MISSING | LOW |
| Toggle Enemy Add/Remove (E) | ✓ | ✗ | MISSING | MEDIUM |
| Quick remove recent (X) | ✓ | ✗ | MISSING | LOW |
| **Save System** |
| Manual save (Ctrl+S) | ✓ | ✓ | COMPLETE | - |
| Autosave | ✓ | ✗ | MISSING | MEDIUM |
| Quick save (F6) | ✓ | ✗ | MISSING | LOW |
| Quick load (F8) | ✓ | ✗ | MISSING | LOW |
| Multi-slot saves | ✓ | ✗ | MISSING | LOW |
| Save browser with thumbnails | ✓ | ✗ | MISSING | LOW |
| **Undo/Redo** |
| Undo (Ctrl+Z) | ✓ | ✓ (toolbar button) | PARTIAL | HIGH |
| Redo (Ctrl+Y) | ✓ | ✓ (toolbar button) | PARTIAL | HIGH |
| Unlimited undo stack | ✓ | ✗ | MISSING | MEDIUM |
| History browser | ✓ | ✗ | MISSING | LOW |
| **Playtest** |
| Launch playtest (F5) | ✓ | ✗ | MISSING | HIGH |
| ESC pause menu | ✓ | ✗ | MISSING | HIGH |
| **Navigation** |
| Ctrl+1/2 cycle worlds | ✓ | ✗ | MISSING | MEDIUM |
| Ctrl+3/4 cycle maps | ✓ | ✗ | MISSING | MEDIUM |
| Home/End jump frames | ✓ | N/A | N/A | - |
| **UI/UX** |
| Status bar messages | ✓ | ✓ | COMPLETE | - |
| Busy indicator | ✓ | ✗ | MISSING | LOW |
| Shortcut overlay (H) | ✓ | ✗ | MISSING | LOW |
| Minimap panel | ✓ | ✗ | MISSING | LOW |
| Theme toggle (dark/light) | ✓ | ✗ | MISSING | LOW |
| Dockable panes | ✓ | ✓ | COMPLETE | - |
| Layout reset | ✓ | ✗ | MISSING | LOW |
| **Comments & Draft** |
| Comment pins | ✓ | ✗ | MISSING | LOW |
| Freehand draft layer | ✓ | ✗ | MISSING | LOW |
| **Brush Presets** |
| Shape presets (round, rect) | ✓ | ✗ | MISSING | MEDIUM |
| Size steps 1-10 | ✓ | ✓ (4 presets) | PARTIAL | MEDIUM |
| Per-tool memory | ✓ | ✗ | MISSING | LOW |
| **Menubar Integration** |
| All tools in menubar | ✓ | ✗ | MISSING | HIGH |
| Dynamic view menu | ✓ | ✗ | MISSING | MEDIUM |
| Keyboard shortcuts via menu | ✓ | ✗ | MISSING | HIGH |

---

## Critical Missing Features (HIGH Priority)

### 1. Keyboard Shortcuts for Tools
**UX Requirement**: Number keys 1-6 select tools directly
**Current**: Tools only accessible via toolbar buttons
**Impact**: Slow workflow, no quick switching
**Implementation**:
- Add `Key()` handler in MapEditorApp
- Map 1→Wall, 2→Erase Wall, 3→Background, 4→Erase BG, 5→Enemy Add, 6→Enemy Remove
- Update toolbar to show shortcuts in tooltips

### 2. Entity/Spawn Point Placement Tools
**UX Requirement**: Tools 5 and 6 for enemy placement, plus droplet spawn editing
**Current**: No entity placement tools
**Impact**: Cannot create levels with enemies or droplets in editor
**Implementation**:
- Add `EnemyPlacementTool` class (similar to BrushTool)
- Add `DropletSpawnTool` class
- Store spawn points in separate annotation layer
- Serialize spawn points with map data (already supported in MapSerializer)
- Add visual indicators (icons/markers) for spawn points in canvas

### 3. Arrow Key Panning
**UX Requirement**: Arrow keys pan viewport, speed scales with zoom
**Current**: No arrow key panning
**Impact**: Requires mouse for all navigation
**Implementation**:
- Handle arrow keys in MapCanvas::Key()
- Pan offset by (PAN_SPEED / zoom) pixels per key press
- Support hold-to-continue-panning

### 4. Grid Toggle Shortcut
**UX Requirement**: Press 'G' to toggle grid
**Current**: Grid toggle only in menu
**Impact**: Requires menu navigation to toggle frequently-used feature
**Implementation**:
- Add 'G' key handler
- Toggle MapCanvas::showGrid flag

### 5. Playtest Launch (F5)
**UX Requirement**: F5 launches current map in playtest mode
**Current**: No playtest integration
**Impact**: Must manually run game to test levels
**Implementation**:
- Add F5 key handler
- Save current map to temp file
- Launch GameScreen with temp file path
- Return to editor when playtest ends

### 6. Menubar Integration for Tools
**UX Requirement**: All tools accessible via menubar with keyboard shortcuts
**Current**: Tools only in toolbar, no keyboard shortcuts
**Impact**: Inconsistent UX, hard to discover shortcuts
**Implementation**:
- Add "Tools" menu with all tool options
- Assign shortcuts (1-6, G, etc.)
- Update toolbar buttons to trigger menu actions

---

## Important Missing Features (MEDIUM Priority)

### 7. Shift+Arrow Reference Image Offset
**UX Requirement**: Hold Shift+Arrow to move reference image independently
**Current**: No reference image offset control beyond initial load
**Implementation**:
- Detect Shift modifier in Key() handler
- Update MapCanvas::referenceImageOffset
- Redraw canvas

### 8. Right-Click Paired Erase Tool
**UX Requirement**: Right-click temporarily switches to erase mode
**Current**: Must manually switch to eraser tool
**Implementation**:
- Track right mouse button state
- Temporarily override currentTool when right-held
- Restore previous tool on release

### 9. Constrained Painting (Shift+Drag)
**UX Requirement**: Hold Shift while dragging to constrain to rows/columns
**Current**: Free-form painting only
**Implementation**:
- Track Shift state during drag
- If Shift held, constrain to horizontal or vertical line from drag start
- Lock to axis with greater movement delta

### 10. Autosave
**UX Requirement**: Autosave after brief idle period
**Current**: Manual save only
**Implementation**:
- Add idle timer (reset on any edit action)
- After 30 seconds idle, auto-save to temp slot
- Show autosave indicator in status bar

### 11. World/Map Navigation (Ctrl+1/2/3/4)
**UX Requirement**: Keyboard shortcuts to cycle through worlds and maps
**Current**: Must use File→Open dialog
**Implementation**:
- Parse level directory structure
- Build world/map index
- Add Ctrl+1/2/3/4 handlers to navigate

### 12. Brush Shape Presets
**UX Requirement**: Shape presets (round, vertical, horizontal, rectangular)
**Current**: Only square brushes
**Implementation**:
- Add shape enum to BrushTool
- Implement different fill patterns per shape
- Add shape selector buttons to tools panel

---

## Nice-to-Have Features (LOW Priority)

### 13. Alt+Click Eyedropper
- Sample tile type at cursor position
- Set active tool and tile type

### 14. Line/Rectangle Drawing Modes
- Drag to draw lines or rectangles
- Show distance overlay during drag

### 15. Quick Save/Load (F6/F8)
- Dedicated quick-save slot
- One-key save/restore

### 16. History Browser
- Visual timeline of undo states
- Jump to any previous state

### 17. Minimap Panel
- Bird's-eye view of full map
- Click to recenter viewport

### 18. Comment System
- Drop comment pins on map
- Store with map data

### 19. Draft Layer
- Freehand sketching layer
- Non-gameplay visual aid

### 20. Theme Toggle
- Dark/light mode support

### 21. Layout Reset
- Restore default panel arrangement

### 22. Shortcut Overlay (H)
- Show active shortcuts and tool

---

## Implementation Roadmap

### Phase 1: Essential Shortcuts (1-2 days)
**Goal**: Make editor usable with keyboard
- [ ] Implement tool shortcuts (1-6)
- [ ] Implement grid toggle (G)
- [ ] Implement arrow key panning
- [ ] Integrate shortcuts into menubar

### Phase 2: Entity Placement (3-5 days)
**Goal**: Enable full level design in editor
- [ ] Create EnemyPlacementTool
- [ ] Create DropletSpawnTool
- [ ] Visual spawn point indicators
- [ ] Serialize spawn points
- [ ] Update GameScreen to read spawn points from editor

### Phase 3: Playtest Integration (2-3 days)
**Goal**: Rapid iteration cycle
- [ ] Implement F5 playtest launch
- [ ] Temp file save/load
- [ ] Return to editor after playtest

### Phase 4: Workflow Improvements (3-4 days)
**Goal**: Faster editing
- [ ] Right-click paired erase
- [ ] Shift+Arrow reference offset
- [ ] Shift+drag constrain painting
- [ ] Autosave
- [ ] World/map navigation shortcuts

### Phase 5: Advanced Tools (4-6 days)
**Goal**: Power user features
- [ ] Brush shape presets
- [ ] Alt+click eyedropper
- [ ] Line/rectangle drawing
- [ ] Distance overlay

### Phase 6: Polish (2-3 days)
**Goal**: UX refinement
- [ ] Quick save/load
- [ ] History browser
- [ ] Minimap
- [ ] Shortcut overlay
- [ ] Theme toggle

---

## Effort Estimate

| Phase | Effort | Priority |
|-------|--------|----------|
| Phase 1 | 1-2 days | HIGH |
| Phase 2 | 3-5 days | HIGH |
| Phase 3 | 2-3 days | HIGH |
| Phase 4 | 3-4 days | MEDIUM |
| Phase 5 | 4-6 days | LOW |
| Phase 6 | 2-3 days | LOW |
| **Total** | **15-23 days** | - |

**Immediate Priority**: Phase 1-3 (6-10 days) to reach feature parity with basic editor requirements.

---

## Next Steps

1. Discuss priorities with stakeholders
2. Begin Phase 1 implementation (keyboard shortcuts)
3. Implement entity placement tools (Phase 2)
4. Test full level design workflow
5. Iterate based on user feedback
