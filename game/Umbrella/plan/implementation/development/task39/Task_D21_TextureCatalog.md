# Task D21: Texture Catalog — Sprite Management

## Priority: MEDIUM — Tier 2 (supports sprite system)

## Overview

Central registry of all game textures with metadata. Manages loading, caching,
and lookup of sprite sheets and individual textures by ID. Supports the entity
definition system (D20) and future entity editor (D25).

## Java Reference
- `editor/entity/TextureCatalog.java`
- `editor/entity/TextureDefinition.java`
- `editor/entity/TextureType.java` (PLAYER, ENEMY, PICKUP, TILE, etc.)

## Design

### TextureCatalog
```cpp
class TextureCatalog {
    VectorMap<String, Image> textures;       // Cached loaded images
    VectorMap<String, SpriteSheet> sheets;   // Cached sprite sheets

    void LoadFromDirectory(const String& dir);  // Scan and load all
    Image GetTexture(const String& id) const;
    SpriteSheet& GetSheet(const String& id);
    void Reload(const String& id);  // Hot-reload for editor
};
```

### Texture Manifest (JSON)
```json
{
  "textures": [
    { "id": "player_sheet", "path": "sprites/player.png", "type": "spritesheet", "frameWidth": 16, "frameHeight": 16 },
    { "id": "enemy_patroller", "path": "sprites/patroller.png", "type": "spritesheet", "frameWidth": 14, "frameHeight": 14 },
    { "id": "tilesheet", "path": "sprites/tiles.png", "type": "tilesheet", "tileSize": 14 }
  ]
}
```

## Implementation Steps

1. **TextureCatalog class** — `TextureCatalog.h/.cpp`
2. **Texture manifest parser** — Load from `share/mods/umbrella/textures.json`
3. **Integration with EntityDefRegistry** — Entity defs reference textures by ID
4. **Hot-reload support** — `Reload()` for editor workflow

## Files to Create
- `game/Umbrella/TextureCatalog.h` / `TextureCatalog.cpp`
- `share/mods/umbrella/textures.json` (manifest)

## Files to Modify
- `game/Umbrella/EntityDefRegistry.h/cpp` — use catalog for texture loading
- `game/Umbrella/GameScreen.cpp` — initialize catalog on level load
- `game/Umbrella/Umbrella.upp` — add new files

## Dependencies
- D20 (Sprite/Animation System)

## Acceptance Criteria
- [ ] Catalog loads textures from manifest JSON
- [ ] Textures accessible by string ID
- [ ] Sprite sheets properly parsed with frame dimensions
- [ ] Hot-reload works for editor use case
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
