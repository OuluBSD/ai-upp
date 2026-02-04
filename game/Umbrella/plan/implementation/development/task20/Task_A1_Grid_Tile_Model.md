# Task A1: Grid/Tile Data Model Implementation

## Priority: CRITICAL - Foundation

## Overview
Implement the core grid and tile data structures for the MapEditor. This is the foundation that all other features build upon.

## Existing Level Format Analysis

Based on files in `share/mods/umbrella/levels/`:

### JSON Format Structure
```json
{
  "columns": 100,          // Grid dimensions (column count)
  "rows": 100,             // Grid dimensions (row count)
  "gridSize": 14,          // Tile size in pixels (14x14)
  "mapCols": 32,           // Visible map area width in tiles
  "mapRows": 24,           // Visible map area height in tiles
  "walls": [0, 1, 2, ...], // Array of linear indices for wall tiles
  "background": [...],      // Array of linear indices for background tiles
  "fullBlocks": [...],      // Array of linear indices for solid block tiles
  "spawns": [               // Entity spawn points (optional)
    {"col": 27, "row": 18},
    {"col": 5, "row": 18}
  ]
}
```

### Coordinate System
- **Linear index**: `index = row * columns + col`
- **To (col, row)**: `col = index % columns`, `row = index / columns`
- Grid dimensions: typically 100x100 cells
- Visible map area: typically 32x24 tiles (mapCols x mapRows)
- Tile size: 14x14 pixels (gridSize)

### Tile Types
1. **Empty** - No tile (passable)
2. **Wall** - Solid collision tile
3. **Background** - Visual only, no collision
4. **FullBlock** - Solid block (like wall but different visual/gameplay)

## Files to Create

### 1. MapGrid.h
**Location**: `game/Umbrella/MapGrid.h`

**Class**: `MapGrid`

**Members**:
```cpp
class MapGrid {
private:
    int columns;       // Total grid width
    int rows;          // Total grid height
    int gridSize;      // Tile size in pixels (14)
    int mapCols;       // Visible map width in tiles
    int mapRows;       // Visible map height in tiles

    Vector<TileType> tiles;  // Linear array of tiles (columns * rows)

public:
    MapGrid();
    MapGrid(int cols, int rows, int tileSize = 14);

    // Grid operations
    void Resize(int newCols, int newRows);
    void Clear();

    // Tile access (by column, row)
    TileType GetTile(int col, int row) const;
    void SetTile(int col, int row, TileType type);

    // Tile access (by linear index)
    TileType GetTileByIndex(int index) const;
    void SetTileByIndex(int index, TileType type);

    // Coordinate conversion
    int ToIndex(int col, int row) const;
    Point ToColRow(int index) const;

    // Bounds checking
    bool IsValid(int col, int row) const;

    // Getters
    int GetColumns() const { return columns; }
    int GetRows() const { return rows; }
    int GetGridSize() const { return gridSize; }
    int GetMapCols() const { return mapCols; }
    int GetMapRows() const { return mapRows; }
    Size GetTotalSize() const { return Size(columns, rows); }

    // Setters for visible map area
    void SetMapArea(int cols, int rows);
};
```

---

### 2. Tile.h
**Location**: `game/Umbrella/Tile.h`

**Enum**: `TileType`

```cpp
enum TileType {
    TILE_EMPTY = 0,      // No tile (passable)
    TILE_WALL,           // Solid collision wall
    TILE_BACKGROUND,     // Visual only, no collision
    TILE_FULLBLOCK,      // Solid block
};
```

**Functions**:
```cpp
// Convert TileType to color (for rendering)
Color TileTypeToColor(TileType type);

// Convert TileType to string (for debugging)
String TileTypeToString(TileType type);
```

---

### 3. MapGrid.cpp
**Location**: `game/Umbrella/MapGrid.cpp`

**Implementation**:

```cpp
#include "MapGrid.h"

MapGrid::MapGrid()
    : columns(100), rows(100), gridSize(14), mapCols(32), mapRows(24) {
    tiles.SetCount(columns * rows, TILE_EMPTY);
}

MapGrid::MapGrid(int cols, int rows, int tileSize)
    : columns(cols), rows(rows), gridSize(tileSize), mapCols(32), mapRows(24) {
    tiles.SetCount(columns * rows, TILE_EMPTY);
}

void MapGrid::Resize(int newCols, int newRows) {
    Vector<TileType> newTiles;
    newTiles.SetCount(newCols * newRows, TILE_EMPTY);

    // Copy old tiles (preserve content)
    int minCols = min(columns, newCols);
    int minRows = min(rows, newRows);

    for(int row = 0; row < minRows; row++) {
        for(int col = 0; col < minCols; col++) {
            int oldIndex = row * columns + col;
            int newIndex = row * newCols + col;
            newTiles[newIndex] = tiles[oldIndex];
        }
    }

    columns = newCols;
    rows = newRows;
    tiles = pick(newTiles);
}

void MapGrid::Clear() {
    for(int i = 0; i < tiles.GetCount(); i++) {
        tiles[i] = TILE_EMPTY;
    }
}

TileType MapGrid::GetTile(int col, int row) const {
    if(!IsValid(col, row)) return TILE_EMPTY;
    return tiles[ToIndex(col, row)];
}

void MapGrid::SetTile(int col, int row, TileType type) {
    if(!IsValid(col, row)) return;
    tiles[ToIndex(col, row)] = type;
}

TileType MapGrid::GetTileByIndex(int index) const {
    if(index < 0 || index >= tiles.GetCount()) return TILE_EMPTY;
    return tiles[index];
}

void MapGrid::SetTileByIndex(int index, TileType type) {
    if(index < 0 || index >= tiles.GetCount()) return;
    tiles[index] = type;
}

int MapGrid::ToIndex(int col, int row) const {
    return row * columns + col;
}

Point MapGrid::ToColRow(int index) const {
    return Point(index % columns, index / columns);
}

bool MapGrid::IsValid(int col, int row) const {
    return col >= 0 && col < columns && row >= 0 && row < rows;
}

void MapGrid::SetMapArea(int cols, int rows) {
    mapCols = cols;
    mapRows = rows;
}
```

---

### 4. Tile.cpp
**Location**: `game/Umbrella/Tile.cpp`

**Implementation**:

```cpp
#include "Tile.h"

// Color scheme from MapPlaytestScreen.java
Color TileTypeToColor(TileType type) {
    switch(type) {
        case TILE_EMPTY:
            return Color(12, 17, 30);     // Canvas: RGB(0.05, 0.07, 0.12)
        case TILE_WALL:
            return Color(198, 79, 97);    // Wall: RGB(0.78, 0.31, 0.38)
        case TILE_BACKGROUND:
            return Color(40, 61, 87);     // Background: RGB(0.16, 0.24, 0.34)
        case TILE_FULLBLOCK:
            return Color(250, 217, 56);   // FullBlock: RGB(0.98, 0.85, 0.22)
        default:
            return Gray();
    }
}

String TileTypeToString(TileType type) {
    switch(type) {
        case TILE_EMPTY: return "Empty";
        case TILE_WALL: return "Wall";
        case TILE_BACKGROUND: return "Background";
        case TILE_FULLBLOCK: return "FullBlock";
        default: return "Unknown";
    }
}
```

---

## Integration with MapEditor

### Update MapEditor.h

Add member:
```cpp
class MapEditorApp : public TopWindow {
private:
    // ... existing members ...

    MapGrid mapGrid;  // The actual map data
```

### Update MapEditor.cpp

In constructor, initialize grid:
```cpp
MapEditorApp::MapEditorApp() {
    // ... existing code ...

    // Initialize with default 100x100 grid, 14px tiles
    mapGrid = MapGrid(100, 100, 14);
    mapGrid.SetMapArea(32, 24);
}
```

---

## Testing Requirements

### Unit Tests (manual for now)

1. **Grid Creation**:
   - Create 100x100 grid
   - Verify all tiles are TILE_EMPTY
   - Check dimensions match

2. **Coordinate Conversion**:
   - Test ToIndex(5, 10) == 10 * 100 + 5 = 1005
   - Test ToColRow(1005) == Point(5, 10)
   - Test round-trip conversions

3. **Tile Access**:
   - Set tile at (10, 20) to TILE_WALL
   - Get tile at (10, 20), verify it's TILE_WALL
   - Test bounds checking (negative, out of range)

4. **Grid Resize**:
   - Create 50x50 grid
   - Set some tiles
   - Resize to 75x75
   - Verify old tiles preserved
   - Verify new tiles are TILE_EMPTY
   - Resize to 25x25 (shrink)
   - Verify kept tiles still correct

5. **Clear**:
   - Set several tiles
   - Call Clear()
   - Verify all tiles are TILE_EMPTY

---

## Success Criteria

- [x] MapGrid.h created with all methods declared
- [x] MapGrid.cpp implemented with all methods
- [x] Tile.h created with TileType enum
- [x] Tile.cpp implemented with color/string conversion
- [x] MapEditor.h updated with mapGrid member
- [x] MapEditor.cpp constructor initializes mapGrid
- [x] Project builds successfully
- [x] Manual tests pass

---

## Next Tasks After Completion

Once Task A1 is complete:
- **Task A3**: Update MapCanvas to render tiles from mapGrid
- **Task A2**: Implement Layer system
- **Task A4**: Implement file I/O to load existing levels

---

## Estimated Effort

**Time**: 2-3 hours

**Complexity**: Low-Medium (straightforward data structures)

**Dependencies**: None (foundation task)
