# MapEditor Feature Implementation Breakdown

## Overview
Detailed breakdown of MapEditor features to implement, based on the original MapEditorScreen.java from RainbowGame.

## Current Status
✅ **Phase 1 Complete**: GUI Shell Implementation
- MenuBar with File, Edit, View menus
- ToolBar with action buttons
- 3-panel Splitter layout (Tools | Canvas+Tabs | Entities)
- MapCanvas with grid rendering and mouse wheel zoom
- Bottom TabCtrl (Properties, Minimap, Tiles)

## Feature Categories

---

## 1. Core Data Model (PRIORITY: CRITICAL - Foundation)

### 1.1 Grid/Tile System
**Files to create**: `MapGrid.h/cpp`, `Tile.h/cpp`

**Features**:
- [ ] Grid data structure (2D array of tiles)
- [ ] Tile types (wall, background, full_block, empty, etc.)
- [ ] Tile metadata (collision, texture ID, properties)
- [ ] Grid resize functionality
- [ ] Grid coordinate system (row/col indexing)

**Reference**: MapEditorScreen uses GridAnnotationMask for tile storage

---

### 1.2 Layer System
**Files to create**: `Layer.h/cpp`, `LayerManager.h/cpp`

**Layer Types**:
- [ ] TerrainLayer (walls, platforms, blocks)
- [ ] BackgroundLayer (visual background tiles)
- [ ] EntityLayer (enemy spawns, player start, items)
- [ ] AnnotationLayer (editor comments, debug info)
- [ ] ReferenceImageLayer (overlay images for tracing)

**Layer Operations**:
- [ ] Add/remove layers
- [ ] Reorder layers (move up/down)
- [ ] Show/hide layers
- [ ] Lock/unlock layers
- [ ] Layer opacity control (0-100%)
- [ ] Active layer selection

**Reference**: MapEditorScreen has extensive layer management in `refreshLayersLeftPane()`

---

### 1.3 Map Metadata
**Files to create**: `MapMetadata.h/cpp`

**Properties**:
- [ ] Map name/title
- [ ] Map dimensions (width x height in tiles)
- [ ] Tile size (e.g., 32x32 pixels)
- [ ] Background color
- [ ] Water system configuration
- [ ] Spawn points (player, enemies, droplets)
- [ ] Map author/version info

---

## 2. Painting Tools (PRIORITY: HIGH - Core Functionality)

### 2.1 Brush Tool
**File**: `BrushTool.h/cpp`

**Features**:
- [ ] Paint single tiles
- [ ] Brush size (1x1, 2x2, 3x3, etc.)
- [ ] Current tile/texture selection
- [ ] Paint on mouse drag
- [ ] Respect layer selection

**Reference**: MapEditorScreen has PaintTarget enum and brush painting logic

---

### 2.2 Eraser Tool
**File**: `EraserTool.h/cpp`

**Features**:
- [ ] Erase tiles (set to empty)
- [ ] Eraser size (1x1, 2x2, etc.)
- [ ] Erase on mouse drag

---

### 2.3 Fill Tool (Bucket Fill)
**File**: `FillTool.h/cpp`

**Features**:
- [ ] Flood fill algorithm
- [ ] Fill contiguous region with selected tile
- [ ] Respect layer boundaries

---

### 2.4 Selection Tool
**File**: `SelectionTool.h/cpp`

**Features**:
- [ ] Rectangle selection (drag to select area)
- [ ] Show selection outline
- [ ] Copy selected area
- [ ] Paste selected area
- [ ] Move selection
- [ ] Delete selection

---

### 2.5 Eyedropper Tool (Pick Tile)
**Features**:
- [ ] Click to pick tile under cursor
- [ ] Set as current brush tile
- [ ] Show picked tile in UI

---

## 3. Canvas Rendering (PRIORITY: HIGH)

### 3.1 Grid Display
**In**: `MapCanvas::Paint()`

**Features**:
- [x] ✅ Grid lines (already implemented)
- [ ] Configurable grid size
- [ ] Grid color/opacity
- [ ] Toggle grid on/off
- [ ] Snap to grid

---

### 3.2 Tile Rendering
**Features**:
- [ ] Render tiles from all visible layers
- [ ] Render tile textures (when textures loaded)
- [ ] Render colored rectangles (placeholder mode)
- [ ] Layer blending (opacity support)
- [ ] Render entity icons/sprites
- [ ] Render collision boxes

**Color scheme** (from MapPlaytestScreen):
- Wall: RGB(0.78, 0.31, 0.38) - reddish
- Background: RGB(0.16, 0.24, 0.34) - dark blue
- FullBlock: RGB(0.98, 0.85, 0.22) - yellow
- Canvas: RGB(0.05, 0.07, 0.12) - very dark

---

### 3.3 Camera/View Control
**Features**:
- [x] ✅ Zoom (mouse wheel - already implemented)
- [ ] Pan (middle mouse drag or arrow keys)
- [ ] Zoom to fit map
- [ ] Center on selection
- [ ] Minimap navigation

---

### 3.4 Visual Feedback
**Features**:
- [ ] Highlight current tile under cursor
- [ ] Show brush preview (outline of tiles to be painted)
- [ ] Show selection box
- [ ] Active layer indicator overlay
- [ ] Show entity spawn points

---

## 4. Undo/Redo System (PRIORITY: HIGH)

### 4.1 Command Pattern
**Files**: `EditorCommand.h/cpp`, `UndoStack.h/cpp`

**Commands to support**:
- [ ] PaintCommand (single or batch tile paint)
- [ ] EraseCommand
- [ ] FillCommand
- [ ] LayerAddCommand
- [ ] LayerRemoveCommand
- [ ] LayerReorderCommand
- [ ] GridResizeCommand

**Features**:
- [ ] Undo stack (Ctrl+Z)
- [ ] Redo stack (Ctrl+Y)
- [ ] Command history size limit (e.g., 100 commands)
- [ ] Clear history on new map

**Reference**: MapEditorScreen mentions undo/redo in editor tools

---

## 5. File I/O (PRIORITY: HIGH)

### 5.1 Map File Format
**File**: `MapSerializer.h/cpp`

**Format Options**:
- Option A: JSON format (human-readable, easier debugging)
- Option B: Binary format (smaller, faster)
- Option C: Custom text format (RainbowGame uses custom format)

**Data to serialize**:
- [ ] Map metadata (dimensions, name, etc.)
- [ ] All layer data (type, visibility, tiles)
- [ ] Entity placements
- [ ] Water configuration
- [ ] Spawn points

---

### 5.2 File Operations
**Features**:
- [x] ✅ New map (stub implemented)
- [ ] New map dialog (choose dimensions)
- [x] ✅ Open map file (dialog implemented)
- [ ] Load map from file
- [x] ✅ Save map file (dialog implemented)
- [ ] Save map to file
- [ ] Save As functionality
- [ ] Auto-save every N minutes
- [ ] Dirty flag (unsaved changes warning)

**Reference**: MapEditorScreen uses EditorSaveManager for save/load

---

## 6. Entity/Spawn Point System (PRIORITY: MEDIUM)

### 6.1 Entity Placement
**File**: `EntityPlacer.h/cpp`

**Entity Types** (from RainbowGame):
- [ ] Player spawn point
- [ ] Enemy spawn points (with type selection)
- [ ] Droplet spawn points (water droplets)
- [ ] Item pickups
- [ ] Trigger zones

**Features**:
- [ ] Place entity by clicking
- [ ] Select entity type from palette
- [ ] Move existing entities (drag and drop)
- [ ] Delete entities
- [ ] Entity properties editor

**Reference**: MapEditorScreen has EnemySpawnMap, DropletSpawnMap

---

### 6.2 Entity Properties
**Panel**: Properties tab

**Properties per entity**:
- [ ] Entity type/ID
- [ ] Position (X, Y)
- [ ] Rotation/facing direction
- [ ] Custom parameters (varies by entity)
- [ ] Enable/disable

---

## 7. Texture/Tile Palette (PRIORITY: MEDIUM)

### 7.1 Tiles Tab
**Panel**: Tiles tab (currently placeholder)

**Features**:
- [ ] Display available tile textures
- [ ] Grid view of tiles
- [ ] Click to select tile for brush
- [ ] Categorize tiles (walls, platforms, decorations)
- [ ] Search/filter tiles
- [ ] Load texture catalog from file

**Reference**: MapEditorScreen has TextureListPanel, TextureCatalog

---

### 7.2 Texture Loading
**File**: `TextureManager.h/cpp`

**Features**:
- [ ] Load textures from asset directory
- [ ] Texture atlas support
- [ ] Placeholder colored rectangles (when no textures)
- [ ] Texture metadata (name, size, category)

**Reference**: Uses uppsrc/Draw for image loading

---

## 8. Minimap (PRIORITY: MEDIUM)

### 8.1 Minimap Display
**Panel**: Minimap tab

**Features**:
- [ ] Render entire map at small scale
- [ ] Show visible viewport indicator (rectangle)
- [ ] Click to navigate (jump viewport)
- [ ] Update in real-time as map changes
- [ ] Show entity positions
- [ ] Color-coded layers

---

## 9. Properties Panel (PRIORITY: MEDIUM)

### 9.1 Contextual Properties
**Panel**: Properties tab

**Display properties for**:
- [ ] Selected tile (position, type, texture)
- [ ] Selected entity (type, position, custom properties)
- [ ] Selected layer (name, visibility, opacity)
- [ ] Map settings (when nothing selected)

**Features**:
- [ ] Edit properties inline
- [ ] Apply changes immediately
- [ ] Property validation

---

## 10. Map Playtest Integration (PRIORITY: MEDIUM)

### 10.1 Playtest Mode
**File**: `MapPlaytestScreen.h/cpp` (new screen)

**Features**:
- [ ] Launch playtest from editor (menu option)
- [ ] Load current editor map into playtest
- [ ] Simple physics simulation (gravity, collision)
- [ ] Colored rectangle rendering (placeholder for sprites)
- [ ] Camera following player
- [ ] Return to editor (Escape or button)
- [ ] Show debug overlay (FPS, position, etc.)

**Reference**: MapPlaytestScreen is a lightweight test runner with colored rectangles

**Key differences from full game**:
- No final art (colored rectangles only)
- Simplified physics
- Quick iteration (no asset loading delay)

---

### 10.2 Playtest Features
**Features from MapPlaytestScreen**:
- [ ] Player movement (arrow keys or WASD)
- [ ] Wall collision detection
- [ ] Jump physics
- [ ] Camera bounds (keep player visible)
- [ ] Enemy spawn visualization
- [ ] Water droplet spawns
- [ ] Pause menu (return to editor, restart)
- [ ] Debug info overlay

**Colors** (from MapPlaytestScreen):
- Canvas: RGB(0.05, 0.07, 0.12)
- Wall: RGB(0.78, 0.31, 0.38)
- Background: RGB(0.16, 0.24, 0.34)
- FullBlock: RGB(0.98, 0.85, 0.22)
- Grid: RGB(0.20, 0.27, 0.36)

---

## 11. Advanced Features (PRIORITY: LOW)

### 11.1 Annotations
- [ ] Add text comments to map
- [ ] Visual markers/pins
- [ ] Debug annotations

### 11.2 Reference Image Overlay
- [ ] Load reference image (PNG/JPG)
- [ ] Overlay on canvas for tracing
- [ ] Adjust opacity
- [ ] Lock/unlock reference layer

### 11.3 Map Import/Export
- [ ] Import from external format
- [ ] Export to game format
- [ ] Batch processing

### 11.4 Water System Config
- [ ] Configure water physics parameters
- [ ] Droplet spawn configuration
- [ ] Water flow visualization

---

## Implementation Phases

### Phase A: Core Foundation (Week 1-2)
1. ✅ GUI shell (completed)
2. Grid/Tile data model
3. Layer system
4. Basic tile rendering (colored rectangles)

### Phase B: Basic Editing (Week 3-4)
5. Brush tool implementation
6. Eraser tool
7. File I/O (save/load maps)
8. Undo/redo system

### Phase C: Enhanced Features (Week 5-6)
9. Entity placement system
10. Tile palette/texture support
11. Selection and copy/paste tools
12. Minimap display

### Phase D: Playtest Integration (Week 7-8)
13. MapPlaytestScreen implementation
14. Simple physics simulation
15. Player movement and collision
16. Debug overlay

### Phase E: Polish (Week 9+)
17. Properties panel functionality
18. Advanced tools (fill, reference images)
19. Auto-save and preferences
20. Performance optimization

---

## Dependencies

### External Systems Needed:
- **LevelLoader** - For loading test levels (basic stub exists)
- **TextureCatalog** - For managing tile textures
- **EntityDefinitions** - For entity types and properties
- **WaterConfig** - For water system parameters

### U++ Systems Used:
- **Draw** - For rendering tiles and grid
- **Image** - For texture loading
- **FileIn/FileOut** - For file I/O
- **Json** - For map file format (optional)

---

## Testing Strategy

### Unit Tests:
- Grid operations (set/get tiles, resize)
- Layer management (add, remove, reorder)
- Undo/redo stack
- File serialization/deserialization

### Integration Tests:
- Paint tool on canvas
- Entity placement and movement
- Map save and load roundtrip
- Playtest mode launch and return

### Manual Tests:
- Create a small test map
- Paint tiles, place entities
- Save map, close editor, reload
- Launch playtest, verify collision
- Undo/redo multiple operations

---

## Success Metrics

- [ ] Can create a 50x30 tile map
- [ ] Can paint walls and platforms
- [ ] Can place player and enemy spawns
- [ ] Can save and reload map successfully
- [ ] Can playtest map with colored rectangles
- [ ] Can undo/redo 50+ operations without lag
- [ ] Can zoom/pan smoothly on large maps (100x100+)

---

## Reference Files in RainbowGame

### Core Editor:
- `MapEditorScreen.java` - Main editor screen (~4500 lines)
- `MapPlaytestScreen.java` - Playtest runner (~1600 lines)

### Entity System:
- `editor/entity/EntityDefinition.java`
- `editor/entity/TextureCatalog.java`
- `editor/entity/TextureDefinition.java`

### Annotation System:
- `editor/annotation/GridAnnotationMask.java` - Tile storage
- `editor/annotation/EnemySpawnMap.java` - Enemy spawns
- `editor/annotation/DropletSpawnMap.java` - Water droplets

### Save/Load:
- `editor/save/EditorSaveManager.java` - Save/load system

### Configuration:
- `editor/options/EditorPreferences.java`
- `gameplay/water/LevelWaterConfig.java`

---

## Next Immediate Tasks

**Recommended starting point**: Phase A - Core Foundation

1. **Task A1**: Implement Grid/Tile data model
   - Create Grid class with 2D tile array
   - Create Tile struct with type and properties
   - Add resize functionality

2. **Task A2**: Implement Layer system
   - Create Layer base class
   - Create TerrainLayer, EntityLayer
   - Add layer visibility/opacity

3. **Task A3**: Enhance MapCanvas rendering
   - Render tiles from grid data
   - Use colored rectangles for tile types
   - Support multiple layer rendering

4. **Task A4**: Basic file I/O
   - Design simple map file format (JSON)
   - Implement save to file
   - Implement load from file

---

This breakdown provides a complete roadmap for implementing all MapEditor features, from core data structures to advanced playtest integration.
