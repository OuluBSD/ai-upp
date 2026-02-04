# Task A2: Layer System Implementation

## Priority: CRITICAL - Foundation

## Overview
Implement a layer management system for the MapEditor. Layers allow organizing different types of map data (terrain, background, entities, annotations) separately.

## Dependencies
- **Task A1** must be completed first (Grid/Tile model)

## Layer Types (from RainbowGame)

### 1. TerrainLayer
- Contains walls and solid blocks
- Affects collision
- Rendered on top of background

### 2. BackgroundLayer
- Visual decoration only
- No collision
- Rendered behind terrain

### 3. EntityLayer (future)
- Entity spawn points
- Player start position
- Item placements

### 4. AnnotationLayer (future)
- Editor comments
- Debug markers
- Development notes

## Files to Create

### 1. Layer.h
**Location**: `game/Umbrella/Layer.h`

```cpp
enum LayerType {
    LAYER_TERRAIN,
    LAYER_BACKGROUND,
    LAYER_ENTITY,
    LAYER_ANNOTATION
};

class Layer {
private:
    String name;
    LayerType type;
    bool visible;
    bool locked;
    int opacity;  // 0-100%
    MapGrid grid; // Each layer has its own grid

public:
    Layer(const String& name, LayerType type, int cols, int rows);

    // Getters
    const String& GetName() const { return name; }
    LayerType GetType() const { return type; }
    bool IsVisible() const { return visible; }
    bool IsLocked() const { return locked; }
    int GetOpacity() const { return opacity; }
    MapGrid& GetGrid() { return grid; }
    const MapGrid& GetGrid() const { return grid; }

    // Setters
    void SetName(const String& n) { name = n; }
    void SetVisible(bool v) { visible = v; }
    void SetLocked(bool l) { locked = l; }
    void SetOpacity(int o) { opacity = clamp(o, 0, 100); }

    // Operations
    void Clear();
};
```

---

### 2. LayerManager.h
**Location**: `game/Umbrella/LayerManager.h`

```cpp
class LayerManager {
private:
    Vector<Layer*> layers;
    int activeLayerIndex;
    int gridColumns;
    int gridRows;

public:
    LayerManager();
    ~LayerManager();

    // Initialization
    void Initialize(int cols, int rows);
    void Clear();

    // Layer operations
    Layer* AddLayer(const String& name, LayerType type);
    void RemoveLayer(int index);
    void MoveLayerUp(int index);
    void MoveLayerDown(int index);

    // Layer access
    Layer* GetLayer(int index);
    const Layer* GetLayer(int index) const;
    int GetLayerCount() const { return layers.GetCount(); }

    // Active layer
    Layer* GetActiveLayer();
    const Layer* GetActiveLayer() const;
    int GetActiveLayerIndex() const { return activeLayerIndex; }
    void SetActiveLayer(int index);

    // Find layers by type
    Layer* FindLayerByType(LayerType type);

    // Rendering helpers
    void ForEachVisibleLayer(const Function<void(const Layer&, int index)>& callback) const;
};
```

---

### 3. Layer.cpp
**Location**: `game/Umbrella/Layer.cpp`

```cpp
#include "Layer.h"

Layer::Layer(const String& name, LayerType type, int cols, int rows)
    : name(name), type(type), visible(true), locked(false), opacity(100),
      grid(cols, rows) {
}

void Layer::Clear() {
    grid.Clear();
}
```

---

### 4. LayerManager.cpp
**Location**: `game/Umbrella/LayerManager.cpp`

```cpp
#include "LayerManager.h"

LayerManager::LayerManager()
    : activeLayerIndex(0), gridColumns(100), gridRows(100) {
}

LayerManager::~LayerManager() {
    Clear();
}

void LayerManager::Initialize(int cols, int rows) {
    gridColumns = cols;
    gridRows = rows;

    // Create default layers
    AddLayer("Background", LAYER_BACKGROUND);
    AddLayer("Terrain", LAYER_TERRAIN);

    // Set terrain as active
    activeLayerIndex = 1;
}

void LayerManager::Clear() {
    for(int i = 0; i < layers.GetCount(); i++) {
        delete layers[i];
    }
    layers.Clear();
    activeLayerIndex = 0;
}

Layer* LayerManager::AddLayer(const String& name, LayerType type) {
    Layer* layer = new Layer(name, type, gridColumns, gridRows);
    layers.Add(layer);
    return layer;
}

void LayerManager::RemoveLayer(int index) {
    if(index < 0 || index >= layers.GetCount()) return;

    delete layers[index];
    layers.Remove(index);

    // Adjust active index if needed
    if(activeLayerIndex >= layers.GetCount()) {
        activeLayerIndex = max(0, layers.GetCount() - 1);
    }
}

void LayerManager::MoveLayerUp(int index) {
    if(index <= 0 || index >= layers.GetCount()) return;

    Swap(layers[index], layers[index - 1]);

    // Update active index
    if(activeLayerIndex == index) {
        activeLayerIndex = index - 1;
    }
    else if(activeLayerIndex == index - 1) {
        activeLayerIndex = index;
    }
}

void LayerManager::MoveLayerDown(int index) {
    if(index < 0 || index >= layers.GetCount() - 1) return;

    Swap(layers[index], layers[index + 1]);

    // Update active index
    if(activeLayerIndex == index) {
        activeLayerIndex = index + 1;
    }
    else if(activeLayerIndex == index + 1) {
        activeLayerIndex = index;
    }
}

Layer* LayerManager::GetLayer(int index) {
    if(index < 0 || index >= layers.GetCount()) return nullptr;
    return layers[index];
}

const Layer* LayerManager::GetLayer(int index) const {
    if(index < 0 || index >= layers.GetCount()) return nullptr;
    return layers[index];
}

Layer* LayerManager::GetActiveLayer() {
    return GetLayer(activeLayerIndex);
}

const Layer* LayerManager::GetActiveLayer() const {
    return GetLayer(activeLayerIndex);
}

void LayerManager::SetActiveLayer(int index) {
    if(index >= 0 && index < layers.GetCount()) {
        activeLayerIndex = index;
    }
}

Layer* LayerManager::FindLayerByType(LayerType type) {
    for(int i = 0; i < layers.GetCount(); i++) {
        if(layers[i]->GetType() == type) {
            return layers[i];
        }
    }
    return nullptr;
}

void LayerManager::ForEachVisibleLayer(const Function<void(const Layer&, int index)>& callback) const {
    for(int i = 0; i < layers.GetCount(); i++) {
        if(layers[i]->IsVisible()) {
            callback(*layers[i], i);
        }
    }
}
```

---

## Integration with MapEditor

### Update MapEditor.h

Replace single mapGrid with LayerManager:
```cpp
class MapEditorApp : public TopWindow {
private:
    // ... existing members ...

    // OLD: MapGrid mapGrid;
    LayerManager layerManager;  // NEW: Manages multiple layers
```

### Update MapEditor.cpp

In constructor:
```cpp
MapEditorApp::MapEditorApp() {
    // ... existing code ...

    // Initialize layer manager with default layers
    layerManager.Initialize(100, 100);
}
```

Update toolbar/menu to show active layer in status bar:
```cpp
// In some update function
void MapEditorApp::UpdateStatusBar() {
    Layer* active = layerManager.GetActiveLayer();
    if(active) {
        String status = Format("Layer: %s (%s)",
            active->GetName(),
            LayerTypeToString(active->GetType()));
        mainStatusBar.Set(status);
    }
}
```

---

## UI Integration (Left Panel - Tools)

### Layer List Display

Add to tools panel (future task):
```cpp
// In SetupUI(), add to toolsPanel:
ArrayCtrl layerList;
layerList.AddColumn("Visible", 30);
layerList.AddColumn("Layer", 150);
layerList.AddColumn("Opacity", 50);

// Populate layers
for(int i = 0; i < layerManager.GetLayerCount(); i++) {
    const Layer* layer = layerManager.GetLayer(i);
    layerList.Add(layer->IsVisible(), layer->GetName(), layer->GetOpacity());
}
```

### Layer Context Menu

Right-click on layer:
- Add Layer Above
- Add Layer Below
- Duplicate Layer
- Delete Layer
- Layer Properties (name, opacity)

---

## Testing Requirements

### Unit Tests

1. **LayerManager Initialization**:
   - Initialize with 100x100
   - Verify 2 default layers (Background, Terrain)
   - Verify active layer is Terrain (index 1)

2. **Add/Remove Layers**:
   - Add new layer
   - Verify layer count increased
   - Remove layer
   - Verify layer count decreased
   - Verify active index adjusted

3. **Layer Reordering**:
   - Create 3 layers (A, B, C)
   - Move B up -> order is (B, A, C)
   - Move C up -> order is (B, C, A)

4. **Active Layer**:
   - Set active layer to index 2
   - Verify GetActiveLayer() returns correct layer
   - Remove active layer
   - Verify active index adjusted to valid layer

5. **Layer Visibility**:
   - Create layer, set visible = false
   - ForEachVisibleLayer should skip it
   - Set visible = true
   - ForEachVisibleLayer should include it

---

## Success Criteria

- [x] Layer.h/cpp created with Layer class
- [x] LayerManager.h/cpp created with manager
- [x] MapEditor updated to use LayerManager instead of single grid
- [x] Default layers created (Background, Terrain)
- [x] Active layer tracking works
- [x] Layer operations work (add, remove, reorder)
- [x] Project builds successfully

---

## Next Tasks After Completion

- **Task A3**: Update MapCanvas rendering to draw from layers
- **Task B2**: Add layer UI controls (list, add/remove buttons)
- **Task B3**: Add layer properties panel

---

## Estimated Effort

**Time**: 3-4 hours

**Complexity**: Medium (managing collections, pointer ownership)

**Dependencies**: Task A1 (Grid/Tile model)
