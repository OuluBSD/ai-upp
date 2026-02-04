# Task A3: Enhanced MapCanvas Rendering

## Priority: HIGH - Visual Feedback

## Overview
Update the MapCanvas to render tiles from the actual grid data using colored rectangles. This provides visual feedback for map editing.

## Dependencies
- **Task A1** must be completed (Grid/Tile model)
- **Task A2** should be completed (Layer system) - can render from single grid if layers not ready

## Rendering Strategy

### Color Scheme (from MapPlaytestScreen.java)

Already implemented in `TileTypeToColor()` from Task A1:
- **Canvas** (empty): `RGB(12, 17, 30)` - very dark blue
- **Wall**: `RGB(198, 79, 97)` - reddish pink
- **Background**: `RGB(40, 61, 87)` - dark blue
- **FullBlock**: `RGB(250, 217, 56)` - yellow
- **Grid lines**: `RGB(51, 69, 92)` - medium gray-blue

## Implementation

### Update MapCanvas::Paint() in MapEditor.cpp

Replace the current stub rendering with actual tile rendering:

```cpp
void MapCanvas::Paint(Draw& w) {
    Size sz = GetSize();

    // Get access to layer manager (passed from parent)
    if(!parentEditor) return;
    LayerManager& layerMgr = parentEditor->GetLayerManager();

    // Draw canvas background
    w.DrawRect(sz, Color(12, 17, 30));  // Canvas color

    // Calculate tile size based on zoom
    int tileSize = int(14 * zoom);  // Base tile size is 14px

    // Calculate visible grid range
    int viewCols = sz.cx / tileSize + 2;  // Extra tiles for smooth panning
    int viewRows = sz.cy / tileSize + 2;

    int startCol = max(0, -offset.x / tileSize);
    int startRow = max(0, -offset.y / tileSize);
    int endCol = min(100, startCol + viewCols);  // Assuming 100x100 grid
    int endRow = min(100, startRow + viewRows);

    // Render each visible layer (bottom to top)
    layerMgr.ForEachVisibleLayer([&](const Layer& layer, int layerIndex) {
        const MapGrid& grid = layer.GetGrid();
        int opacity = layer.GetOpacity();

        // Render tiles in this layer
        for(int row = startRow; row < endRow; row++) {
            for(int col = startCol; col < endCol; col++) {
                TileType tile = grid.GetTile(col, row);

                if(tile == TILE_EMPTY) continue;  // Skip empty tiles

                // Calculate screen position
                int screenX = col * tileSize + offset.x;
                int screenY = row * tileSize + offset.y;

                // Get tile color
                Color tileColor = TileTypeToColor(tile);

                // Apply layer opacity
                if(opacity < 100) {
                    tileColor = Blend(tileColor, Color(12, 17, 30), opacity * 255 / 100);
                }

                // Draw tile as filled rectangle
                w.DrawRect(screenX, screenY, tileSize, tileSize, tileColor);
            }
        }
    });

    // Draw grid lines (on top of tiles)
    if(showGrid) {
        Color gridColor = Color(51, 69, 92);  // Grid color

        // Vertical lines
        for(int col = startCol; col <= endCol; col++) {
            int screenX = col * tileSize + offset.x;
            w.DrawLine(screenX, 0, screenX, sz.cy, 1, gridColor);
        }

        // Horizontal lines
        for(int row = startRow; row <= endRow; row++) {
            int screenY = row * tileSize + offset.y;
            w.DrawLine(0, screenY, sz.cx, screenY, 1, gridColor);
        }
    }

    // Draw cursor highlight (current tile under mouse)
    if(cursorCol >= 0 && cursorRow >= 0) {
        int screenX = cursorCol * tileSize + offset.x;
        int screenY = cursorRow * tileSize + offset.y;

        // Draw highlight rectangle (white outline)
        w.DrawRect(screenX, screenY, tileSize, tileSize, Null);
        w.DrawRect(screenX, screenY, tileSize, tileSize, 2, LtGray());
    }
}
```

---

## Additional MapCanvas Features

### 1. Pan Support (Arrow Keys or Middle Mouse Drag)

Add to MapCanvas class:
```cpp
class MapCanvas : public Ctrl {
private:
    Point offset;       // Camera offset in pixels
    double zoom;        // Zoom level (1.0 = 100%)
    bool panning;       // Is currently panning
    Point panStart;     // Pan drag start position
    Point cursorTile;   // Tile under cursor (col, row)
    bool showGrid;      // Show/hide grid lines

    MapEditorApp* parentEditor;  // Access to parent for layer data

public:
    void SetParentEditor(MapEditorApp* parent) { parentEditor = parent; }
    void SetShowGrid(bool show) { showGrid = show; Refresh(); }
    bool GetShowGrid() const { return showGrid; }
```

Update mouse handlers:
```cpp
void MapCanvas::MiddleDown(Point pos, dword flags) {
    panning = true;
    panStart = pos;
    SetCapture();
}

void MapCanvas::MiddleUp(Point pos, dword flags) {
    panning = false;
    ReleaseCapture();
}

void MapCanvas::MouseMove(Point pos, dword flags) {
    if(panning) {
        // Update camera offset
        Point delta = pos - panStart;
        offset += delta;
        panStart = pos;
        Refresh();
    }

    // Update cursor tile position
    int tileSize = int(14 * zoom);
    cursorCol = (pos.x - offset.x) / tileSize;
    cursorRow = (pos.y - offset.y) / tileSize;

    Refresh();
}
```

---

### 2. Zoom to Fit

Add method:
```cpp
void MapCanvas::ZoomToFit() {
    // Get visible map area (e.g., 32x24 tiles)
    if(!parentEditor) return;

    Layer* layer = parentEditor->GetLayerManager().GetActiveLayer();
    if(!layer) return;

    const MapGrid& grid = layer->GetGrid();
    int mapCols = grid.GetMapCols();
    int mapRows = grid.GetMapRows();
    int tileSize = grid.GetGridSize();

    // Calculate zoom to fit map in canvas
    Size canvasSize = GetSize();
    double zoomX = double(canvasSize.cx) / (mapCols * tileSize);
    double zoomY = double(canvasSize.cy) / (mapRows * tileSize);

    zoom = min(zoomX, zoomY);
    offset = Point(0, 0);  // Reset offset

    Refresh();
}
```

---

### 3. Grid Toggle

Add menu/toolbar button:
```cpp
// In SetupViewMenu()
bar.Add("Show Grid", [=] {
    mapCanvas.SetShowGrid(!mapCanvas.GetShowGrid());
})
.Check(mapCanvas.GetShowGrid())
.Key(K_G);
```

---

## Update MapEditor.h

Add method to access layer manager:
```cpp
class MapEditorApp : public TopWindow {
public:
    LayerManager& GetLayerManager() { return layerManager; }
```

In SetupUI(), connect canvas to parent:
```cpp
void MapEditorApp::SetupUI() {
    // ... existing code ...

    mapCanvas.SetParentEditor(this);
}
```

---

## Testing Requirements

### Visual Tests

1. **Empty Grid**:
   - Start editor
   - Verify grid lines visible
   - Verify canvas background is dark blue

2. **Paint Some Tiles**:
   - Manually set some tiles in code (for testing):
     ```cpp
     Layer* terrain = layerManager.FindLayerByType(LAYER_TERRAIN);
     terrain->GetGrid().SetTile(5, 5, TILE_WALL);
     terrain->GetGrid().SetTile(6, 5, TILE_WALL);
     terrain->GetGrid().SetTile(7, 5, TILE_FULLBLOCK);
     ```
   - Verify tiles render in correct colors
   - Verify tiles appear at correct positions

3. **Zoom**:
   - Use mouse wheel to zoom in/out
   - Verify tiles scale correctly
   - Verify grid lines scale correctly

4. **Pan**:
   - Drag with middle mouse button
   - Verify canvas scrolls
   - Verify tiles move correctly

5. **Layer Visibility**:
   - Hide background layer
   - Verify background tiles don't render
   - Show background layer
   - Verify background tiles render again

6. **Layer Opacity**:
   - Set terrain layer opacity to 50%
   - Verify tiles are semi-transparent
   - Set opacity to 100%
   - Verify tiles are opaque

7. **Cursor Highlight**:
   - Move mouse over canvas
   - Verify white outline follows cursor tile

---

## Performance Considerations

- Only render tiles in visible viewport (not entire 100x100 grid)
- Skip empty tiles (don't draw canvas color for each tile)
- Consider caching tile render if performance becomes an issue

---

## Success Criteria

- [x] MapCanvas::Paint() renders tiles from layers
- [x] Colored rectangles match tile types
- [x] Grid lines visible and toggle-able
- [x] Zoom affects tile size
- [x] Pan moves camera
- [x] Cursor highlight works
- [x] Layer visibility affects rendering
- [x] Layer opacity works
- [x] Project builds and runs

---

## Next Tasks After Completion

- **Task B1**: Implement Brush tool (paint tiles on canvas)
- **Task B2**: Implement Eraser tool
- **Task A4**: Load existing level files

---

## Estimated Effort

**Time**: 3-4 hours

**Complexity**: Medium (rendering logic, coordinate math)

**Dependencies**: Task A1, ideally Task A2
