# Task C1: Entity/Spawn Point Placement

## Priority: MEDIUM - Gameplay Integration

## Overview
Implement entity spawn point placement for player start positions, enemy spawns, and other game objects.

## Dependencies
- **Task A1** (Grid/Tile model)
- **Task A2** (Layer system)
- **Task A4** (Level loading - spawns exist in JSON)

## Entity Types (from existing levels)

### From JSON Analysis
```json
"spawns": [
  {"col": 27, "row": 18},
  {"col": 5, "row": 18}
]
```

### Entity Categories
1. **Player Start** - Where player spawns
2. **Enemy Spawns** - Enemy spawn points
3. **Item Pickups** - Collectible items
4. **Droplet Spawns** - Water droplets (from RainbowGame)
5. **Trigger Zones** - Special event triggers

## Files to Create

### 1. Entity.h
**Location**: `game/Umbrella/Entity.h`

```cpp
enum EntityType {
    ENTITY_PLAYER_START,
    ENTITY_ENEMY_SPAWN,
    ENTITY_ITEM,
    ENTITY_DROPLET_SPAWN,
    ENTITY_TRIGGER
};

struct Entity {
    EntityType type;
    int col;
    int row;
    String name;  // Optional display name
    ValueMap properties;  // Custom properties per entity

    Entity(EntityType t, int c, int r)
        : type(t), col(c), row(r) {}
};

// Convert EntityType to string
String EntityTypeToString(EntityType type);

// Get display color for entity type
Color EntityTypeToColor(EntityType type);
```

---

### 2. EntityLayer.h
**Location**: `game/Umbrella/EntityLayer.h`

```cpp
class EntityLayer : public Layer {
private:
    Vector<Entity*> entities;

public:
    EntityLayer(const String& name, int cols, int rows);
    ~EntityLayer();

    // Entity operations
    Entity* AddEntity(EntityType type, int col, int row);
    void RemoveEntity(int index);
    void RemoveEntityAt(int col, int row);

    // Entity access
    int GetEntityCount() const { return entities.GetCount(); }
    Entity* GetEntity(int index);
    const Entity* GetEntity(int index) const;

    // Find entity at position
    Entity* FindEntityAt(int col, int row);

    // Clear all entities
    void ClearEntities();
};
```

---

### 3. EntityTool.h
**Location**: `game/Umbrella/EntityTool.h`

```cpp
class EntityTool {
private:
    EntityType currentEntityType;
    bool placementMode;  // True = place, False = delete

public:
    EntityTool();

    void SetEntityType(EntityType type) { currentEntityType = type; }
    EntityType GetEntityType() const { return currentEntityType; }

    void SetPlacementMode(bool place) { placementMode = place; }
    bool IsPlacementMode() const { return placementMode; }

    // Place or delete entity at position
    void ClickAt(int col, int row, LayerManager& layerMgr);
};
```

---

### 4. Entity.cpp

```cpp
#include "Entity.h"

String EntityTypeToString(EntityType type) {
    switch(type) {
        case ENTITY_PLAYER_START: return "Player Start";
        case ENTITY_ENEMY_SPAWN: return "Enemy Spawn";
        case ENTITY_ITEM: return "Item";
        case ENTITY_DROPLET_SPAWN: return "Droplet Spawn";
        case ENTITY_TRIGGER: return "Trigger";
        default: return "Unknown";
    }
}

Color EntityTypeToColor(EntityType type) {
    switch(type) {
        case ENTITY_PLAYER_START: return LtGreen();   // Bright green
        case ENTITY_ENEMY_SPAWN: return LtRed();      // Bright red
        case ENTITY_ITEM: return LtBlue();            // Bright blue
        case ENTITY_DROPLET_SPAWN: return LtCyan();   // Cyan
        case ENTITY_TRIGGER: return Yellow();         // Yellow
        default: return White();
    }
}
```

---

### 5. EntityLayer.cpp

```cpp
#include "EntityLayer.h"

EntityLayer::EntityLayer(const String& name, int cols, int rows)
    : Layer(name, LAYER_ENTITY, cols, rows) {
}

EntityLayer::~EntityLayer() {
    ClearEntities();
}

Entity* EntityLayer::AddEntity(EntityType type, int col, int row) {
    // Check if entity already exists at position
    Entity* existing = FindEntityAt(col, row);
    if(existing) {
        // Replace existing entity
        existing->type = type;
        return existing;
    }

    // Create new entity
    Entity* entity = new Entity(type, col, row);
    entities.Add(entity);
    return entity;
}

void EntityLayer::RemoveEntity(int index) {
    if(index >= 0 && index < entities.GetCount()) {
        delete entities[index];
        entities.Remove(index);
    }
}

void EntityLayer::RemoveEntityAt(int col, int row) {
    for(int i = 0; i < entities.GetCount(); i++) {
        if(entities[i]->col == col && entities[i]->row == row) {
            RemoveEntity(i);
            return;
        }
    }
}

Entity* EntityLayer::GetEntity(int index) {
    if(index >= 0 && index < entities.GetCount()) {
        return entities[index];
    }
    return nullptr;
}

const Entity* EntityLayer::GetEntity(int index) const {
    if(index >= 0 && index < entities.GetCount()) {
        return entities[index];
    }
    return nullptr;
}

Entity* EntityLayer::FindEntityAt(int col, int row) {
    for(Entity* entity : entities) {
        if(entity->col == col && entity->row == row) {
            return entity;
        }
    }
    return nullptr;
}

void EntityLayer::ClearEntities() {
    for(Entity* entity : entities) {
        delete entity;
    }
    entities.Clear();
}
```

---

### 6. EntityTool.cpp

```cpp
#include "EntityTool.h"

EntityTool::EntityTool()
    : currentEntityType(ENTITY_PLAYER_START), placementMode(true) {
}

void EntityTool::ClickAt(int col, int row, LayerManager& layerMgr) {
    // Find or create entity layer
    EntityLayer* entityLayer = dynamic_cast<EntityLayer*>(
        layerMgr.FindLayerByType(LAYER_ENTITY)
    );

    if(!entityLayer) {
        // Create entity layer if it doesn't exist
        Layer* newLayer = layerMgr.AddLayer("Entities", LAYER_ENTITY);
        entityLayer = dynamic_cast<EntityLayer*>(newLayer);
    }

    if(!entityLayer) return;

    if(placementMode) {
        // Place entity
        entityLayer->AddEntity(currentEntityType, col, row);
    }
    else {
        // Delete entity
        entityLayer->RemoveEntityAt(col, row);
    }
}
```

---

## Rendering Entities

### Update MapCanvas::Paint()

Add entity rendering after tiles:
```cpp
void MapCanvas::Paint(Draw& w) {
    // ... existing tile rendering ...

    // Render entities
    EntityLayer* entityLayer = dynamic_cast<EntityLayer*>(
        parentEditor->GetLayerManager().FindLayerByType(LAYER_ENTITY)
    );

    if(entityLayer && entityLayer->IsVisible()) {
        int tileSize = int(14 * zoom);

        for(int i = 0; i < entityLayer->GetEntityCount(); i++) {
            const Entity* entity = entityLayer->GetEntity(i);
            if(!entity) continue;

            int screenX = entity->col * tileSize + offset.x;
            int screenY = entity->row * tileSize + offset.y;

            // Get entity color
            Color entityColor = EntityTypeToColor(entity->type);

            // Draw entity as circle
            int centerX = screenX + tileSize / 2;
            int centerY = screenY + tileSize / 2;
            int radius = tileSize / 3;

            w.DrawEllipse(centerX - radius, centerY - radius,
                          radius * 2, radius * 2, entityColor);

            // Draw outline
            w.DrawEllipse(centerX - radius, centerY - radius,
                          radius * 2, radius * 2, 1, Black());

            // Draw entity type indicator (first letter)
            String typeStr = EntityTypeToString(entity->type);
            w.DrawText(centerX - 5, centerY - 8,
                       typeStr.Left(1), StdFont(12), White());
        }
    }

    // ... existing preview code ...
}
```

---

## Integration with MapEditor

### Update MapEditor.h

Add entity tool:
```cpp
class MapEditorApp : public TopWindow {
private:
    EntityTool entityTool;

    enum EditTool {
        TOOL_BRUSH,
        TOOL_ERASER,
        TOOL_FILL,
        TOOL_SELECT,
        TOOL_ENTITY  // Add entity tool
    };

public:
    EntityTool& GetEntityTool() { return entityTool; }
```

---

### Update MapCanvas Mouse Handler

```cpp
void MapCanvas::LeftDown(Point pos, dword flags) {
    // ... get col, row ...

    if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_ENTITY) {
        EntityTool& entity = parentEditor->GetEntityTool();
        LayerManager& layerMgr = parentEditor->GetLayerManager();

        entity.ClickAt(cursorCol, cursorRow, layerMgr);
        Refresh();
    }
    // ... other tools ...
}
```

---

## UI Controls

### Entity Type Selector (Right Panel)

Update entity panel:
```cpp
// In SetupUI(), entityPanel
Label entityTypeLabel;
entityTypeLabel.SetText("Entity Type:");
entityPanel.Add(entityTypeLabel.HSizePos(5, 5).TopPos(30, 20));

DropList entityTypeList;
entityTypeList.Add(ENTITY_PLAYER_START, "Player Start");
entityTypeList.Add(ENTITY_ENEMY_SPAWN, "Enemy Spawn");
entityTypeList.Add(ENTITY_ITEM, "Item");
entityTypeList.Add(ENTITY_DROPLET_SPAWN, "Droplet Spawn");
entityTypeList.Add(ENTITY_TRIGGER, "Trigger");
entityTypeList.SetIndex(0);

entityTypeList <<= [=] {
    int type = entityTypeList.GetKey();
    entityTool.SetEntityType((EntityType)type);
};

entityPanel.Add(entityTypeList.HSizePos(5, 5).TopPos(55, 25));

// Place/Delete mode
Option placeMode, deleteMode;
placeMode.SetLabel("Place");
deleteMode.SetLabel("Delete");

placeMode.Set(true);
placeMode <<= [=] {
    entityTool.SetPlacementMode(placeMode.Get());
};

entityPanel.Add(placeMode.LeftPos(5, 80).TopPos(90, 25));
entityPanel.Add(deleteMode.LeftPos(90, 80).TopPos(90, 25));
```

---

### Keyboard Shortcut

```cpp
case K_P:  // P for Place entities
    currentTool = TOOL_ENTITY;
    return true;
```

---

## Load Entities from JSON

### Update MapSerializer::LoadFromFile()

```cpp
// Load spawns (entity layer)
if(json.Exists("spawns") && json["spawns"].IsArray()) {
    EntityLayer* entityLayer = dynamic_cast<EntityLayer*>(
        layerMgr.FindLayerByType(LAYER_ENTITY)
    );

    if(!entityLayer) {
        Layer* newLayer = layerMgr.AddLayer("Entities", LAYER_ENTITY);
        entityLayer = dynamic_cast<EntityLayer*>(newLayer);
    }

    if(entityLayer) {
        const Value& spawns = json["spawns"];
        for(int i = 0; i < spawns.GetCount(); i++) {
            int col = spawns[i]["col"];
            int row = spawns[i]["row"];

            // Default to enemy spawn (can enhance later with type field)
            entityLayer->AddEntity(ENTITY_ENEMY_SPAWN, col, row);
        }
    }
}
```

---

## Testing Requirements

### Manual Tests

1. **Place Entity**:
   - Select entity tool (P)
   - Select Player Start
   - Click on canvas
   - Verify green circle appears

2. **Place Multiple**:
   - Place 3 enemy spawns
   - Verify red circles appear
   - Verify at correct positions

3. **Replace Entity**:
   - Place Player Start at (5, 5)
   - Place Enemy Spawn at (5, 5)
   - Verify only Enemy Spawn remains

4. **Delete Entity**:
   - Place entities
   - Switch to Delete mode
   - Click on entity
   - Verify entity removed

5. **Load from JSON**:
   - Load world1-stage3.json (has spawns)
   - Verify 2 entities appear
   - Check positions match JSON (col 27 row 18, col 5 row 18)

6. **Layer Visibility**:
   - Place entities
   - Hide entity layer
   - Verify entities not visible
   - Show entity layer
   - Verify entities reappear

---

## Success Criteria

- [x] Entity.h/cpp created
- [x] EntityLayer class implemented
- [x] EntityTool implemented
- [x] Entities render as colored circles
- [x] Can place entities by clicking
- [x] Can delete entities
- [x] Entity types selectable (UI dropdown)
- [x] Load entities from JSON
- [x] Keyboard shortcut (P) works
- [x] Layer visibility respected
- [x] Project builds and runs

---

## Next Tasks After Completion

- **Task C2**: Entity Properties Panel (edit entity attributes)
- **Task D1**: Save entities to JSON
- **Task B4**: Selection tool (select/move entities)

---

## Estimated Effort

**Time**: 4-5 hours

**Complexity**: Medium-High (new data structures, rendering)

**Dependencies**: Task A1, A2, A4
