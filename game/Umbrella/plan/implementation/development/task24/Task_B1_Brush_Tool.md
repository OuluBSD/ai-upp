# Task B1: Brush Tool Implementation

## Priority: HIGH - Core Editing Functionality

## Overview
Implement the brush tool for painting tiles on the map canvas. This is the primary editing tool for creating maps.

## Dependencies
- **Task A1** must be completed (Grid/Tile model)
- **Task A2** must be completed (Layer system)
- **Task A3** must be completed (Canvas rendering)

## Brush Tool Features

### Basic Functionality
1. Paint single tiles by clicking
2. Paint multiple tiles by dragging
3. Respect active layer selection
4. Use currently selected tile type
5. Show brush preview (before painting)

### Brush Settings
- Brush size: 1x1, 2x2, 3x3, 5x5
- Current tile type: WALL, BACKGROUND, FULLBLOCK
- Paint mode: Replace or Add

## Files to Create

### 1. BrushTool.h
**Location**: `game/Umbrella/BrushTool.h`

```cpp
enum BrushSize {
    BRUSH_1X1 = 1,
    BRUSH_2X2 = 2,
    BRUSH_3X3 = 3,
    BRUSH_5X5 = 5
};

class BrushTool {
private:
    BrushSize brushSize;
    TileType paintTile;
    bool isPainting;
    Point lastPaintPos;

public:
    BrushTool();

    // Settings
    void SetBrushSize(BrushSize size) { brushSize = size; }
    BrushSize GetBrushSize() const { return brushSize; }

    void SetPaintTile(TileType tile) { paintTile = tile; }
    TileType GetPaintTile() const { return paintTile; }

    // Painting
    void StartPainting(int col, int row, LayerManager& layerMgr);
    void ContinuePainting(int col, int row, LayerManager& layerMgr);
    void StopPainting();

    // Preview
    void GetBrushTiles(int centerCol, int centerRow, Vector<Point>& outTiles) const;

private:
    void PaintAt(int col, int row, LayerManager& layerMgr);
    bool ShouldPaint(int col, int row) const;
};
```

---

### 2. BrushTool.cpp
**Location**: `game/Umbrella/BrushTool.cpp`

```cpp
#include "BrushTool.h"

BrushTool::BrushTool()
    : brushSize(BRUSH_1X1), paintTile(TILE_WALL), isPainting(false),
      lastPaintPos(-1, -1) {
}

void BrushTool::StartPainting(int col, int row, LayerManager& layerMgr) {
    isPainting = true;
    lastPaintPos = Point(col, row);
    PaintAt(col, row, layerMgr);
}

void BrushTool::ContinuePainting(int col, int row, LayerManager& layerMgr) {
    if(!isPainting) return;

    // Check if position changed
    if(col == lastPaintPos.x && row == lastPaintPos.y) return;

    // Paint at new position
    PaintAt(col, row, layerMgr);
    lastPaintPos = Point(col, row);
}

void BrushTool::StopPainting() {
    isPainting = false;
    lastPaintPos = Point(-1, -1);
}

void BrushTool::PaintAt(int col, int row, LayerManager& layerMgr) {
    Layer* activeLayer = layerMgr.GetActiveLayer();
    if(!activeLayer) return;

    if(activeLayer->IsLocked()) return;

    MapGrid& grid = activeLayer->GetGrid();

    // Get brush tiles
    Vector<Point> brushTiles;
    GetBrushTiles(col, row, brushTiles);

    // Paint each tile in brush area
    for(const Point& pt : brushTiles) {
        if(grid.IsValid(pt.x, pt.y)) {
            grid.SetTile(pt.x, pt.y, paintTile);
        }
    }
}

void BrushTool::GetBrushTiles(int centerCol, int centerRow, Vector<Point>& outTiles) const {
    outTiles.Clear();

    int size = (int)brushSize;
    int halfSize = size / 2;

    for(int dy = -halfSize; dy <= halfSize; dy++) {
        for(int dx = -halfSize; dx <= halfSize; dx++) {
            outTiles.Add(Point(centerCol + dx, centerRow + dy));
        }
    }
}

bool BrushTool::ShouldPaint(int col, int row) const {
    // Could add logic here for "paint only if different" mode
    return true;
}
```

---

## Integration with MapEditor

### Update MapEditor.h

Add members:
```cpp
class MapEditorApp : public TopWindow {
private:
    // ... existing members ...

    BrushTool brushTool;
    bool toolActive;  // Is tool being used

    // Tool selection
    enum EditTool {
        TOOL_BRUSH,
        TOOL_ERASER,
        TOOL_FILL,
        TOOL_SELECT
    };

    EditTool currentTool;

public:
    BrushTool& GetBrushTool() { return brushTool; }
    EditTool GetCurrentTool() const { return currentTool; }
    void SetCurrentTool(EditTool tool) { currentTool = tool; }
```

---

### Update MapCanvas

Add brush tool integration to mouse handlers:

```cpp
void MapCanvas::LeftDown(Point pos, dword flags) {
    if(!parentEditor) return;

    // Get tile coordinates
    int tileSize = int(14 * zoom);
    int col = (pos.x - offset.x) / tileSize;
    int row = (pos.y - offset.y) / tileSize;

    // Check current tool
    if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_BRUSH) {
        BrushTool& brush = parentEditor->GetBrushTool();
        LayerManager& layerMgr = parentEditor->GetLayerManager();

        brush.StartPainting(col, row, layerMgr);
        Refresh();
    }
}

void MapCanvas::MouseMove(Point pos, dword flags) {
    // ... existing pan code ...

    // Update cursor tile
    int tileSize = int(14 * zoom);
    cursorCol = (pos.x - offset.x) / tileSize;
    cursorRow = (pos.y - offset.y) / tileSize;

    // If painting, continue painting
    if(flags & K_MOUSELEFT) {
        if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_BRUSH) {
            BrushTool& brush = parentEditor->GetBrushTool();
            LayerManager& layerMgr = parentEditor->GetLayerManager();

            brush.ContinuePainting(cursorCol, cursorRow, layerMgr);
        }
    }

    Refresh();
}

void MapCanvas::LeftUp(Point pos, dword flags) {
    if(!parentEditor) return;

    if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_BRUSH) {
        BrushTool& brush = parentEditor->GetBrushTool();
        brush.StopPainting();
    }
}
```

---

### Add Brush Preview to Paint()

Show brush outline before painting:

```cpp
void MapCanvas::Paint(Draw& w) {
    // ... existing tile rendering ...

    // Draw brush preview
    if(cursorCol >= 0 && cursorRow >= 0) {
        BrushTool& brush = parentEditor->GetBrushTool();

        Vector<Point> brushTiles;
        brush.GetBrushTiles(cursorCol, cursorRow, brushTiles);

        int tileSize = int(14 * zoom);

        // Get preview color (semi-transparent version of paint tile)
        Color previewColor = TileTypeToColor(brush.GetPaintTile());
        previewColor = Blend(previewColor, Color(255, 255, 255), 128);

        // Draw brush tiles preview
        for(const Point& pt : brushTiles) {
            int screenX = pt.x * tileSize + offset.x;
            int screenY = pt.y * tileSize + offset.y;

            // Draw semi-transparent preview
            w.DrawRect(screenX, screenY, tileSize, tileSize, previewColor);

            // Draw outline
            w.DrawRect(screenX, screenY, tileSize, tileSize, 1, White());
        }
    }
}
```

---

## UI Controls

### Brush Size Buttons (Tools Panel)

Add to tools panel:
```cpp
// In SetupUI(), tools panel
Label brushSizeLabel;
brushSizeLabel.SetText("Brush Size:");
toolsPanel.Add(brushSizeLabel.HSizePos(5, 5).TopPos(30, 20));

Button brush1x1, brush2x2, brush3x3, brush5x5;

brush1x1.SetLabel("1x1");
brush1x1 <<= [=] {
    brushTool.SetBrushSize(BRUSH_1X1);
};
toolsPanel.Add(brush1x1.LeftPos(5, 40).TopPos(55, 25));

brush2x2.SetLabel("2x2");
brush2x2 <<= [=] {
    brushTool.SetBrushSize(BRUSH_2X2);
};
toolsPanel.Add(brush2x2.LeftPos(50, 40).TopPos(55, 25));

brush3x3.SetLabel("3x3");
brush3x3 <<= [=] {
    brushTool.SetBrushSize(BRUSH_3X3);
};
toolsPanel.Add(brush3x3.LeftPos(95, 40).TopPos(55, 25));

brush5x5.SetLabel("5x5");
brush5x5 <<= [=] {
    brushTool.SetBrushSize(BRUSH_5X5);
};
toolsPanel.Add(brush5x5.LeftPos(140, 40).TopPos(55, 25));
```

---

### Tile Type Selector

Add tile type selection buttons:
```cpp
Label tileTypeLabel;
tileTypeLabel.SetText("Paint Tile:");
toolsPanel.Add(tileTypeLabel.HSizePos(5, 5).TopPos(90, 20));

Button wallBtn, bgBtn, blockBtn;

wallBtn.SetLabel("Wall");
wallBtn <<= [=] {
    brushTool.SetPaintTile(TILE_WALL);
};
toolsPanel.Add(wallBtn.LeftPos(5, 60).TopPos(115, 25));

bgBtn.SetLabel("Background");
bgBtn <<= [=] {
    brushTool.SetPaintTile(TILE_BACKGROUND);
};
toolsPanel.Add(bgBtn.LeftPos(70, 80).TopPos(115, 25));

blockBtn.SetLabel("FullBlock");
blockBtn <<= [=] {
    brushTool.SetPaintTile(TILE_FULLBLOCK);
};
toolsPanel.Add(blockBtn.LeftPos(155, 80).TopPos(115, 25));
```

---

### Keyboard Shortcuts

Add to MapEditor::Key():
```cpp
bool MapEditorApp::Key(dword key, int) {
    switch(key) {
        case K_B:  // B for Brush
            currentTool = TOOL_BRUSH;
            return true;

        case K_1:  // Number keys for brush size
            brushTool.SetBrushSize(BRUSH_1X1);
            return true;
        case K_2:
            brushTool.SetBrushSize(BRUSH_2X2);
            return true;
        case K_3:
            brushTool.SetBrushSize(BRUSH_3X3);
            return true;
        case K_5:
            brushTool.SetBrushSize(BRUSH_5X5);
            return true;

        case K_W:  // W for Wall
            brushTool.SetPaintTile(TILE_WALL);
            return true;
        case K_G:  // G for backGround
            brushTool.SetPaintTile(TILE_BACKGROUND);
            return true;
        case K_F:  // F for Fullblock
            brushTool.SetPaintTile(TILE_FULLBLOCK);
            return true;

        // ... existing shortcuts ...
    }
    return false;
}
```

---

## Testing Requirements

### Manual Tests

1. **Single Tile Painting**:
   - Select 1x1 brush
   - Click on canvas
   - Verify single tile painted
   - Verify correct color

2. **Brush Sizes**:
   - Test 2x2: Click, verify 4 tiles painted
   - Test 3x3: Click, verify 9 tiles painted
   - Test 5x5: Click, verify 25 tiles painted

3. **Drag Painting**:
   - Select brush
   - Click and drag across canvas
   - Verify continuous painting
   - Verify no gaps in painted trail

4. **Tile Types**:
   - Paint with WALL (red-pink)
   - Paint with BACKGROUND (dark blue)
   - Paint with FULLBLOCK (yellow)
   - Verify colors match

5. **Layer Respect**:
   - Paint on Terrain layer
   - Switch to Background layer
   - Paint different tiles
   - Verify each layer has correct tiles

6. **Locked Layer**:
   - Lock active layer
   - Try to paint
   - Verify no painting occurs
   - Unlock layer, verify painting works

7. **Brush Preview**:
   - Move mouse over canvas
   - Verify brush outline visible
   - Change brush size
   - Verify preview size updates

8. **Keyboard Shortcuts**:
   - Press 1, 2, 3, 5 to change brush size
   - Press W, G, F to change tile type
   - Press B to select brush tool

---

## Future Enhancements (Not Part of Task B1)

- Brush shapes (circle, line)
- Brush patterns (checkerboard, noise)
- Pressure sensitivity (for tablets)
- Smart brush (auto-connect tiles)

---

## Success Criteria

- [x] BrushTool.h/cpp created
- [x] Basic painting works (click to paint)
- [x] Drag painting works (continuous)
- [x] Brush sizes work (1x1, 2x2, 3x3, 5x5)
- [x] Tile types work (WALL, BACKGROUND, FULLBLOCK)
- [x] Brush preview visible
- [x] Layer selection respected
- [x] Locked layers prevent painting
- [x] Keyboard shortcuts work
- [x] UI controls added to tools panel
- [x] Project builds and runs

---

## Next Tasks After Completion

- **Task B2**: Eraser Tool
- **Task B3**: Fill Tool (bucket fill)
- **Task B7**: Undo/Redo system (to undo brush strokes)

---

## Estimated Effort

**Time**: 3-4 hours

**Complexity**: Medium (mouse interaction, preview rendering)

**Dependencies**: Task A1, A2, A3
