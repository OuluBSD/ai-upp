# Task B3: Fill Tool (Bucket Fill) Implementation

## Priority: MEDIUM - Useful Editing Tool

## Overview
Implement flood fill tool for filling contiguous regions with the same tile type. Useful for filling large areas quickly.

## Dependencies
- **Task A1** (Grid/Tile model)
- **Task A2** (Layer system)
- **Task B1** (Brush tool - for reference)

## Fill Tool Features

### Basic Functionality
1. Click on tile to fill contiguous region
2. Fill spreads to adjacent tiles of same type
3. 4-directional fill (up, down, left, right)
4. Respect layer boundaries
5. Visual feedback during fill

## Files to Create

### 1. FillTool.h
**Location**: `game/Umbrella/FillTool.h`

```cpp
class FillTool {
private:
    TileType fillTile;

public:
    FillTool();

    void SetFillTile(TileType tile) { fillTile = tile; }
    TileType GetFillTile() const { return fillTile; }

    // Perform flood fill starting at (col, row)
    void Fill(int col, int row, LayerManager& layerMgr);

private:
    void FloodFill(int col, int row, TileType targetTile, MapGrid& grid);
    void FloodFillRecursive(int col, int row, TileType targetTile, TileType replaceTile, MapGrid& grid);
};
```

---

### 2. FillTool.cpp
**Location**: `game/Umbrella/FillTool.cpp`

```cpp
#include "FillTool.h"

FillTool::FillTool()
    : fillTile(TILE_WALL) {
}

void FillTool::Fill(int col, int row, LayerManager& layerMgr) {
    Layer* activeLayer = layerMgr.GetActiveLayer();
    if(!activeLayer) return;

    if(activeLayer->IsLocked()) return;

    MapGrid& grid = activeLayer->GetGrid();

    if(!grid.IsValid(col, row)) return;

    TileType targetTile = grid.GetTile(col, row);

    // Don't fill if target is already the fill tile
    if(targetTile == fillTile) return;

    // Perform flood fill
    FloodFill(col, row, targetTile, grid);
}

void FillTool::FloodFill(int col, int row, TileType targetTile, MapGrid& grid) {
    // Use queue-based approach (non-recursive to avoid stack overflow)
    Vector<Point> queue;
    queue.Add(Point(col, row));

    while(queue.GetCount() > 0) {
        Point pt = queue.Pop();

        int c = pt.x;
        int r = pt.y;

        // Check bounds
        if(!grid.IsValid(c, r)) continue;

        // Check if this tile matches target
        if(grid.GetTile(c, r) != targetTile) continue;

        // Fill this tile
        grid.SetTile(c, r, fillTile);

        // Add neighbors to queue (4-directional)
        queue.Add(Point(c + 1, r));     // Right
        queue.Add(Point(c - 1, r));     // Left
        queue.Add(Point(c, r + 1));     // Down
        queue.Add(Point(c, r - 1));     // Up
    }
}
```

---

## Integration with MapEditor

### Update MapEditor.h

Add fill tool member:
```cpp
class MapEditorApp : public TopWindow {
private:
    // ... existing members ...

    FillTool fillTool;

public:
    FillTool& GetFillTool() { return fillTool; }
```

Update tool enum:
```cpp
enum EditTool {
    TOOL_BRUSH,
    TOOL_ERASER,
    TOOL_FILL,    // Already exists
    TOOL_SELECT
};
```

---

### Update MapCanvas Mouse Handler

Add fill tool handling:
```cpp
void MapCanvas::LeftDown(Point pos, dword flags) {
    // ... get col, row ...

    if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_FILL) {
        FillTool& fill = parentEditor->GetFillTool();
        LayerManager& layerMgr = parentEditor->GetLayerManager();

        // Synchronize fill tile with brush tile
        BrushTool& brush = parentEditor->GetBrushTool();
        fill.SetFillTile(brush.GetPaintTile());

        fill.Fill(cursorCol, cursorRow, layerMgr);
        Refresh();
    }
    else if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_BRUSH) {
        // ... existing brush code ...
    }
    // ... other tools ...
}
```

---

## UI Controls

### Add Fill Button to Toolbar

```cpp
Button fillBtn;
fillBtn.SetImage(CtrlImg::bucket()).Tip("Fill Tool (F)");
fillBtn <<= [=] {
    currentTool = TOOL_FILL;
};
mainToolBar.Add(fillBtn);
```

### Keyboard Shortcut

```cpp
case K_F:  // F for Fill
    currentTool = TOOL_FILL;
    return true;
```

---

## Fill Preview

### Show Fill Preview on Hover

In `MapCanvas::Paint()`:
```cpp
// If fill tool is active, show preview
if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_FILL && cursorCol >= 0) {
    // Highlight the region that would be filled
    Layer* activeLayer = parentEditor->GetLayerManager().GetActiveLayer();
    if(activeLayer) {
        const MapGrid& grid = activeLayer->GetGrid();
        TileType targetTile = grid.GetTile(cursorCol, cursorRow);

        // Show different cursor for fill
        int tileSize = int(14 * zoom);
        int screenX = cursorCol * tileSize + offset.x;
        int screenY = cursorRow * tileSize + offset.y;

        // Draw bucket icon or highlight
        w.DrawRect(screenX, screenY, tileSize, tileSize, 2, Yellow());

        // Optional: Show fill color preview
        Color fillColor = TileTypeToColor(fillTool.GetFillTile());
        fillColor = Blend(fillColor, Color(255, 255, 255), 200);
        w.DrawRect(screenX + 2, screenY + 2, tileSize - 4, tileSize - 4, fillColor);
    }
}
```

---

## Performance Optimization

### Limit Fill Area

For very large fills, add a limit:
```cpp
void FillTool::FloodFill(int col, int row, TileType targetTile, MapGrid& grid) {
    Vector<Point> queue;
    queue.Add(Point(col, row));

    int fillCount = 0;
    const int MAX_FILL = 10000;  // Prevent filling entire 100x100 grid

    while(queue.GetCount() > 0 && fillCount < MAX_FILL) {
        Point pt = queue.Pop();

        // ... existing fill logic ...

        fillCount++;
    }

    if(fillCount >= MAX_FILL) {
        Exclamation("Fill stopped: too many tiles (limit: 10000)");
    }
}
```

---

## Testing Requirements

### Manual Tests

1. **Simple Fill**:
   - Create a small enclosed area (e.g., 5x5 box)
   - Click inside with fill tool
   - Verify entire area fills with selected tile type

2. **Boundary Respect**:
   - Create area with walls
   - Fill inside
   - Verify fill doesn't leak through walls

3. **Large Area Fill**:
   - Clear entire grid
   - Fill entire grid
   - Verify performance acceptable
   - Check for hangs/crashes

4. **Mixed Tiles**:
   - Paint checkerboard pattern (alternating tiles)
   - Fill one color
   - Verify only matching tiles fill

5. **Layer Respect**:
   - Fill on Background layer
   - Verify Terrain layer unaffected
   - Switch to Terrain, fill
   - Verify Background layer unaffected

6. **Locked Layer**:
   - Lock active layer
   - Try to fill
   - Verify no fill occurs

7. **Same Tile Fill**:
   - Fill area with WALL
   - Try to fill same area with WALL again
   - Verify nothing happens (optimization)

8. **Keyboard Shortcut**:
   - Press F to select fill tool
   - Verify tool switches
   - Press B to go back to brush

---

## Edge Cases

### 1. Fill Entire Grid
- Should work but may be slow
- Consider progress indicator or limit

### 2. Fill on Edge
- Start fill at grid edge (row 0 or col 0)
- Verify no out-of-bounds errors

### 3. Fill Empty Grid
- Fill on completely empty grid
- Verify fills entire grid

---

## Success Criteria

- [x] FillTool.h/cpp created
- [x] Flood fill algorithm works (4-directional)
- [x] Respects tile boundaries
- [x] Performance acceptable (10,000 tile limit)
- [x] Layer selection respected
- [x] Locked layers prevent fill
- [x] Fill preview visible
- [x] Keyboard shortcut (F) works
- [x] UI button added to toolbar
- [x] No crashes on large fills
- [x] Project builds and runs

---

## Future Enhancements (Not Part of Task B3)

- 8-directional fill (include diagonals)
- Fill with pattern instead of solid color
- Magic wand (fill similar colors, not just exact match)
- Fill preview highlighting entire region

---

## Next Tasks After Completion

- **Task B4**: Selection Tool
- **Task B7**: Undo/Redo (undo large fills)

---

## Estimated Effort

**Time**: 2-3 hours

**Complexity**: Medium (flood fill algorithm)

**Dependencies**: Task A1, A2, B1
