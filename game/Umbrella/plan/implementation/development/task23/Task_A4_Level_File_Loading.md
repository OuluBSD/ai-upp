# Task A4: Level File Loading (JSON Format)

## Priority: HIGH - Integration with Existing Content

## Overview
Implement loading of existing level files from `share/mods/umbrella/levels/`. This allows the MapEditor to open and edit existing game levels.

## Dependencies
- **Task A1** must be completed (Grid/Tile model)
- **Task A2** should be completed (Layer system)

## Existing Level Format

### JSON Structure
```json
{
  "columns": 100,          // Total grid dimensions
  "rows": 100,
  "gridSize": 14,          // Tile size in pixels
  "mapCols": 32,           // Visible map area
  "mapRows": 24,
  "walls": [0, 1, 2, ...], // Linear indices of wall tiles
  "background": [...],      // Linear indices of background tiles
  "fullBlocks": [...],      // Linear indices of full block tiles
  "spawns": [               // Optional: entity spawn points
    {"col": 27, "row": 18},
    {"col": 5, "row": 18}
  ]
}
```

### File Locations
- Base path: `share/mods/umbrella/levels/`
- Manifest: `manifest.json` (lists all worlds and levels)
- Levels: `world1-stage1.json`, `world2-stage3.json`, etc.

## Files to Create

### 1. MapSerializer.h
**Location**: `game/Umbrella/MapSerializer.h`

```cpp
class MapSerializer {
public:
    // Load map from JSON file
    static bool LoadFromFile(const String& filePath, LayerManager& layerMgr);

    // Save map to JSON file (future)
    static bool SaveToFile(const String& filePath, const LayerManager& layerMgr);

private:
    // Helper: Load tile indices from JSON array
    static Vector<int> LoadTileIndices(const Value& jsonArray);

    // Helper: Convert linear index to (col, row)
    static Point IndexToColRow(int index, int columns);
};
```

---

### 2. MapSerializer.cpp
**Location**: `game/Umbrella/MapSerializer.cpp`

```cpp
#include "MapSerializer.h"
#include <Core/JSON.h>

bool MapSerializer::LoadFromFile(const String& filePath, LayerManager& layerMgr) {
    // Load JSON file
    String jsonText = LoadFile(filePath);
    if(jsonText.IsEmpty()) {
        Exclamation("Failed to load file: " + filePath);
        return false;
    }

    // Parse JSON
    Value json = ParseJSON(jsonText);
    if(json.IsError()) {
        Exclamation("Failed to parse JSON: " + filePath);
        return false;
    }

    // Extract grid dimensions
    int columns = json["columns"];
    int rows = json["rows"];
    int gridSize = json["gridSize"];
    int mapCols = json["mapCols"];
    int mapRows = json["mapRows"];

    // Clear and reinitialize layer manager
    layerMgr.Clear();
    layerMgr.Initialize(columns, rows);

    // Get layers
    Layer* terrainLayer = layerMgr.FindLayerByType(LAYER_TERRAIN);
    Layer* backgroundLayer = layerMgr.FindLayerByType(LAYER_BACKGROUND);

    if(!terrainLayer || !backgroundLayer) {
        Exclamation("Failed to find default layers");
        return false;
    }

    // Set map area
    terrainLayer->GetGrid().SetMapArea(mapCols, mapRows);
    backgroundLayer->GetGrid().SetMapArea(mapCols, mapRows);

    // Load walls
    if(json.Exists("walls")) {
        Vector<int> wallIndices = LoadTileIndices(json["walls"]);
        for(int index : wallIndices) {
            Point pt = IndexToColRow(index, columns);
            terrainLayer->GetGrid().SetTile(pt.x, pt.y, TILE_WALL);
        }
    }

    // Load fullBlocks
    if(json.Exists("fullBlocks")) {
        Vector<int> blockIndices = LoadTileIndices(json["fullBlocks"]);
        for(int index : blockIndices) {
            Point pt = IndexToColRow(index, columns);
            terrainLayer->GetGrid().SetTile(pt.x, pt.y, TILE_FULLBLOCK);
        }
    }

    // Load background
    if(json.Exists("background")) {
        Vector<int> bgIndices = LoadTileIndices(json["background"]);
        for(int index : bgIndices) {
            Point pt = IndexToColRow(index, columns);
            backgroundLayer->GetGrid().SetTile(pt.x, pt.y, TILE_BACKGROUND);
        }
    }

    // TODO: Load spawns (entity layer - future task)

    return true;
}

Vector<int> MapSerializer::LoadTileIndices(const Value& jsonArray) {
    Vector<int> indices;

    if(jsonArray.IsArray()) {
        for(int i = 0; i < jsonArray.GetCount(); i++) {
            indices.Add(jsonArray[i]);
        }
    }

    return indices;
}

Point MapSerializer::IndexToColRow(int index, int columns) {
    return Point(index % columns, index / columns);
}

bool MapSerializer::SaveToFile(const String& filePath, const LayerManager& layerMgr) {
    // TODO: Implement in future task
    Exclamation("Save functionality not yet implemented");
    return false;
}
```

---

## Integration with MapEditor

### Update MapEditor::OpenFileAction()

```cpp
void MapEditorApp::OpenFileAction() {
    FileSel fs;

    // Set initial directory to levels folder
    fs.BaseDir(ShareDirFile("mods/umbrella/levels"));

    // File types
    fs.Type("JSON Level files", "*.json");
    fs.AllFilesType();

    if(fs.ExecuteOpen("Open Map File")) {
        String filePath = fs.Get();
        OpenFile(filePath);
    }
}

void MapEditorApp::OpenFile(const String& fileName) {
    if(MapSerializer::LoadFromFile(fileName, layerManager)) {
        currentFilePath = fileName;

        // Update window title
        Title("Umbrella Map Editor - " + GetFileName(fileName));

        // Refresh canvas
        mapCanvas.Refresh();

        // Zoom to fit new map
        mapCanvas.ZoomToFit();

        PromptOK("Level loaded successfully!");
    }
    else {
        PromptOK("Failed to load level: " + fileName);
    }
}
```

### Add Member to MapEditor.h

```cpp
class MapEditorApp : public TopWindow {
private:
    String currentFilePath;  // Currently open file

public:
    const String& GetCurrentFilePath() const { return currentFilePath; }
```

---

## Testing with Existing Levels

### Test Files
Use existing levels in `share/mods/umbrella/levels/`:

1. **Simple level**: `world1-stage1.json`
   - 32x24 visible map
   - Walls and fullBlocks
   - Background tiles

2. **Complex level**: `world2-stage5.json`
   - 56x24 visible map
   - More walls and background
   - Larger, more intricate design

3. **Level with spawns**: `world1-stage3.json`
   - Has enemy spawn points
   - (Spawns won't render yet - entity layer is future task)

### Test Procedure

1. **Launch Editor**:
   ```bash
   bin/Umbrella --editor
   ```

2. **Open Level**:
   - File → Open
   - Navigate to `share/mods/umbrella/levels/`
   - Select `world1-stage1.json`
   - Click Open

3. **Verify Loading**:
   - Check title bar shows filename
   - Verify grid dimensions (should be 100x100)
   - Verify visible map area (32x24 for world1-stage1)
   - Verify tiles render in correct positions

4. **Visual Verification**:
   - Zoom to fit (should see ~32x24 tiles)
   - Verify walls (red-pink color)
   - Verify fullBlocks (yellow color)
   - Verify background (dark blue color)
   - Verify empty tiles (very dark canvas color)

5. **Test Multiple Levels**:
   - Open different levels (world2-stage5, world3-stage1)
   - Verify each loads correctly
   - Verify different map sizes work

---

## Error Handling

### File Not Found
```cpp
if(jsonText.IsEmpty()) {
    Exclamation("File not found or empty: " + filePath);
    return false;
}
```

### Invalid JSON
```cpp
if(json.IsError()) {
    Exclamation("Invalid JSON format in file: " + filePath +
                "\nError: " + json.GetErrorText());
    return false;
}
```

### Missing Required Fields
```cpp
if(!json.Exists("columns") || !json.Exists("rows")) {
    Exclamation("Missing required fields (columns, rows) in: " + filePath);
    return false;
}
```

---

## Future Enhancements (Not Part of Task A4)

### Save Functionality
- Convert layers back to JSON
- Save to file
- Preserve spawn points and other data

### Manifest Integration
- Load manifest.json
- Show world/level selection dialog
- Quick navigation between levels

### Backup on Load
- Create .bak file before loading
- Prevent data loss on accidental overwrite

---

## Success Criteria

- [x] MapSerializer.h/cpp created
- [x] LoadFromFile() implemented
- [x] Can load world1-stage1.json successfully
- [x] Tiles render in correct positions and colors
- [x] Grid dimensions loaded correctly
- [x] Map area (mapCols, mapRows) loaded
- [x] Walls, fullBlocks, background all load
- [x] Error handling for missing/invalid files
- [x] Window title shows filename
- [x] Project builds and runs

---

## Next Tasks After Completion

- **Task B1**: Implement Brush tool (to edit loaded maps)
- **Task B4**: Implement Save functionality
- **Task C1**: Implement entity/spawn loading and editing

---

## Estimated Effort

**Time**: 2-3 hours

**Complexity**: Medium (JSON parsing, error handling)

**Dependencies**: Task A1 (Grid), Task A2 (Layers)

---

## Example Manual Test

```cpp
// In MapEditorApp constructor, for testing:
#ifdef _DEBUG
    // Auto-load a test level on startup
    String testLevel = ShareDirFile("mods/umbrella/levels/world1-stage1.json");
    if(FileExists(testLevel)) {
        OpenFile(testLevel);
    }
#endif
```

This allows quickly testing the rendering without clicking through the file dialog each time.
