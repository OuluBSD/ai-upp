# Task B2: Eraser Tool Implementation

## Priority: HIGH - Core Editing Functionality

## Overview
Implement the eraser tool for removing tiles from the map. Works like the brush tool but sets tiles to TILE_EMPTY.

## Dependencies
- **Task B1** must be completed (Brush tool - similar implementation)

## Eraser Tool Features

### Basic Functionality
1. Erase tiles by clicking (set to TILE_EMPTY)
2. Erase multiple tiles by dragging
3. Respect active layer selection
4. Variable eraser size (like brush)
5. Show eraser preview

## Implementation

### Use Existing BrushTool with Mode

**Option A**: Extend BrushTool with EraseMode (RECOMMENDED)

Modify `BrushTool.h`:
```cpp
enum BrushMode {
    BRUSH_MODE_PAINT,
    BRUSH_MODE_ERASE
};

class BrushTool {
private:
    BrushMode mode;

public:
    void SetMode(BrushMode m) { mode = m; }
    BrushMode GetMode() const { return mode; }

    // Effective tile to paint (TILE_EMPTY if erasing)
    TileType GetEffectiveTile() const {
        return (mode == BRUSH_MODE_ERASE) ? TILE_EMPTY : paintTile;
    }
};
```

Modify `BrushTool::PaintAt()`:
```cpp
void BrushTool::PaintAt(int col, int row, LayerManager& layerMgr) {
    // ... existing code ...

    TileType tileToPaint = GetEffectiveTile();

    for(const Point& pt : brushTiles) {
        if(grid.IsValid(pt.x, pt.y)) {
            grid.SetTile(pt.x, pt.y, tileToPaint);
        }
    }
}
```

---

**Option B**: Create Separate EraserTool Class

If you prefer separation:

### EraserTool.h
```cpp
class EraserTool {
private:
    BrushSize eraserSize;
    bool isErasing;
    Point lastErasePos;

public:
    EraserTool();

    void SetEraserSize(BrushSize size) { eraserSize = size; }
    BrushSize GetEraserSize() const { return eraserSize; }

    void StartErasing(int col, int row, LayerManager& layerMgr);
    void ContinueErasing(int col, int row, LayerManager& layerMgr);
    void StopErasing();

    void GetEraserTiles(int centerCol, int centerRow, Vector<Point>& outTiles) const;

private:
    void EraseAt(int col, int row, LayerManager& layerMgr);
};
```

Implementation is nearly identical to BrushTool, but always sets TILE_EMPTY.

---

## Integration with MapEditor

### Update MapEditor Tool Selection

In `MapEditor.h`:
```cpp
enum EditTool {
    TOOL_BRUSH,
    TOOL_ERASER,  // Add eraser
    TOOL_FILL,
    TOOL_SELECT
};
```

### Update MapCanvas Mouse Handlers

Add eraser case:
```cpp
void MapCanvas::LeftDown(Point pos, dword flags) {
    // ... get col, row ...

    if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_BRUSH) {
        // ... brush code ...
    }
    else if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_ERASER) {
        BrushTool& brush = parentEditor->GetBrushTool();
        brush.SetMode(BRUSH_MODE_ERASE);
        LayerManager& layerMgr = parentEditor->GetLayerManager();

        brush.StartPainting(col, row, layerMgr);
        Refresh();
    }
}

void MapCanvas::MouseMove(Point pos, dword flags) {
    // ... existing code ...

    if(flags & K_MOUSELEFT) {
        if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_ERASER) {
            BrushTool& brush = parentEditor->GetBrushTool();
            brush.SetMode(BRUSH_MODE_ERASE);
            LayerManager& layerMgr = parentEditor->GetLayerManager();

            brush.ContinuePainting(cursorCol, cursorRow, layerMgr);
        }
    }

    Refresh();
}
```

---

## UI Controls

### Add Eraser Button to Toolbar

In `SetupToolBar()`:
```cpp
Button eraserBtn;
eraserBtn.SetLabel("Eraser").Tip("Eraser Tool (E)");
eraserBtn <<= [=] {
    currentTool = TOOL_ERASER;
};
mainToolBar.Add(eraserBtn);
```

### Keyboard Shortcut

Add to `MapEditor::Key()`:
```cpp
case K_E:  // E for Eraser
    currentTool = TOOL_ERASER;
    brushTool.SetMode(BRUSH_MODE_ERASE);
    return true;
```

---

## Eraser Preview

### Update Paint() for Eraser Preview

In `MapCanvas::Paint()`:
```cpp
// Draw tool preview
if(cursorCol >= 0 && cursorRow >= 0) {
    BrushTool& brush = parentEditor->GetBrushTool();

    Vector<Point> previewTiles;
    brush.GetBrushTiles(cursorCol, cursorRow, previewTiles);

    int tileSize = int(14 * zoom);

    Color previewColor;
    if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_ERASER) {
        // Eraser preview: Show as red outline (delete indicator)
        previewColor = LtRed();
    }
    else {
        // Brush preview: Show tile color
        previewColor = TileTypeToColor(brush.GetPaintTile());
        previewColor = Blend(previewColor, Color(255, 255, 255), 128);
    }

    for(const Point& pt : previewTiles) {
        int screenX = pt.x * tileSize + offset.x;
        int screenY = pt.y * tileSize + offset.y;

        if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_ERASER) {
            // Draw red X for eraser
            w.DrawLine(screenX, screenY, screenX + tileSize, screenY + tileSize, 2, LtRed());
            w.DrawLine(screenX + tileSize, screenY, screenX, screenY + tileSize, 2, LtRed());

            // Red outline
            w.DrawRect(screenX, screenY, tileSize, tileSize, 2, LtRed());
        }
        else {
            // Normal brush preview
            w.DrawRect(screenX, screenY, tileSize, tileSize, previewColor);
            w.DrawRect(screenX, screenY, tileSize, tileSize, 1, White());
        }
    }
}
```

---

## Testing Requirements

### Manual Tests

1. **Single Tile Erase**:
   - Paint some tiles with brush
   - Switch to eraser (press E)
   - Click on painted tile
   - Verify tile becomes empty (canvas color)

2. **Drag Erasing**:
   - Paint a line of tiles
   - Select eraser
   - Drag across painted line
   - Verify all tiles erased

3. **Eraser Sizes**:
   - Paint a large area
   - Test 1x1 eraser: Single tile removal
   - Test 2x2 eraser: 4 tiles at once
   - Test 3x3, 5x5: Larger areas

4. **Layer Respect**:
   - Paint on Terrain layer
   - Paint on Background layer
   - Switch to Terrain layer, erase
   - Verify only Terrain tiles erased
   - Background tiles remain

5. **Eraser Preview**:
   - Select eraser
   - Move mouse over canvas
   - Verify red X preview visible
   - Change eraser size
   - Verify preview size updates

6. **Tool Switching**:
   - Press B (brush), paint tiles
   - Press E (eraser), erase tiles
   - Press B (brush), paint again
   - Verify smooth switching

7. **Keyboard Shortcuts**:
   - Press E to select eraser
   - Press 1, 2, 3, 5 to change size
   - Verify eraser size changes

---

## Alternative: Right-Click to Erase

Add right-click erasing for faster workflow:

```cpp
void MapCanvas::RightDown(Point pos, dword flags) {
    // Treat right-click as temporary eraser
    int tileSize = int(14 * zoom);
    int col = (pos.x - offset.x) / tileSize;
    int row = (pos.y - offset.y) / tileSize;

    BrushTool& brush = parentEditor->GetBrushTool();
    brush.SetMode(BRUSH_MODE_ERASE);
    LayerManager& layerMgr = parentEditor->GetLayerManager();

    brush.StartPainting(col, row, layerMgr);
    Refresh();
}

void MapCanvas::RightUp(Point pos, dword flags) {
    BrushTool& brush = parentEditor->GetBrushTool();
    brush.StopPainting();
    brush.SetMode(BRUSH_MODE_PAINT);  // Restore paint mode
}
```

---

## Success Criteria

- [x] Eraser tool functional (sets tiles to TILE_EMPTY)
- [x] Eraser sizes work (1x1, 2x2, 3x3, 5x5)
- [x] Drag erasing works
- [x] Eraser preview visible (red X)
- [x] Layer selection respected
- [x] Locked layers prevent erasing
- [x] Keyboard shortcut (E) works
- [x] Tool switching (B ↔ E) works
- [x] Optional: Right-click erasing works
- [x] Project builds and runs

---

## Next Tasks After Completion

- **Task B3**: Fill Tool (bucket fill)
- **Task B4**: Selection Tool (select/copy/paste)
- **Task B7**: Undo/Redo (undo erase operations)

---

## Estimated Effort

**Time**: 1-2 hours (reuses BrushTool code)

**Complexity**: Low (similar to Brush)

**Dependencies**: Task B1 (Brush tool)
