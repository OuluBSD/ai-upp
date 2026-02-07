# Missing Features - U++ Umbrella vs Java RainbowGame

**Date**: 2026-02-07
**Comparison**: `game/Umbrella/` vs `../RainbowGame/trash/core/`

---

## Executive Summary

The U++ Umbrella implementation has successfully ported core platformer gameplay (Phases 1-3, partial Phase 5). However, RainbowGame contains significant additional systems that are missing:

- **Water/Droplet system** (primary collectible mechanic)
- **Parasol attack system** (player combat)
- **Rewards/Treats system** (score bonuses, special items)
- **Advanced enemy types** (9+ additional enemy variants)
- **Sprite/animation system** (all entities currently use colored rectangles)
- **Entity system** (data-driven entity definitions with animations)
- **Advanced map editor features** (annotation system, entity editor, import tools)
- **Debug overlay** (performance metrics, collision visualization)
- **Level manifest system** (world/level progression)

---

## Phase 6: Missing Gameplay Systems

### Track 6.1: Water/Droplet System
**Priority**: HIGH (Core collectible mechanic)
**Java Files**:
- `gameplay/water/Droplet.java`
- `gameplay/water/DropletManager.java`
- `gameplay/water/DropletPhysics.java`
- `gameplay/water/DropletSpawn.java`
- `gameplay/water/LevelWaterConfig.java`
- `gameplay/water/DropletMode.java` (enum: RAINBOW, ICE, FIRE)

**Description**:
Droplets are the primary collectible in RainbowGame. They spawn from tiles marked in annotations, have physics (bounce, gravity), and come in three modes (Rainbow, Ice, Fire) with different behaviors.

**Implementation Tasks**:
1. Create `Droplet` class with physics (bounce velocity, gravity, rotation)
2. Create `DropletManager` to spawn and update droplets
3. Parse droplet spawn points from level annotations
4. Implement droplet-player collision (collect)
5. Track collected droplets per level
6. Add droplet rendering (colored circles for now, sprites later)

**Estimated Effort**: 12-16 hours

---

### Track 6.2: Parasol Attack System
**Priority**: HIGH (Player combat mechanic)
**Java Files**:
- `gameplay/components/Player.java` (parasolHitbox, attackHeld logic)
- `ui/screens/GameScreen.java` (parasol collision with enemies)

**Description**:
Player holds an umbrella/parasol that can be used to attack enemies. When attack button is held, a hitbox extends from the player. Enemies defeated by parasol drop treats/rewards.

**Current Status**: Player has `keyAttack` tracking but no parasol hitbox or enemy defeat logic.

**Implementation Tasks**:
1. Add `parasolHitbox` Rectangle to Player
2. Update parasol hitbox position based on player facing and attack state
3. Implement parasol-enemy collision in GameScreen
4. Add `Enemy::Defeat()` method (kill enemy, spawn reward)
5. Track enemy defeats for scoring/rewards
6. Add parasol rendering (colored rectangle extending from player)

**Estimated Effort**: 6-8 hours

---

### Track 6.3: Rewards/Treats System
**Priority**: MEDIUM (Score enhancement)
**Java Files**:
- `gameplay/rewards/TreatCatalog.java`
- `gameplay/rewards/TreatDefinition.java`
- `gameplay/rewards/RewardContext.java`
- `gameplay/rewards/RewardTrigger.java` (enum: PLANNED_TAKEDOWN, COMBO_STREAK, etc.)
- `gameplay/rewards/RewardConditions.java`

**Description**:
When enemies are defeated, they drop "treats" (food items) that give score bonuses. Different enemies drop different treats (patroller→pear, jumper→banana, shooter→blueberry). Additional treats spawn for special conditions (combos, time bonuses, etc.).

**Java mapping**:
```java
THEMED_TREAT_IDS.put(EnemyPatroller.class, "pear");
THEMED_TREAT_IDS.put(EnemyJumper.class, "banana");
THEMED_TREAT_IDS.put(EnemyShooter.class, "blueberry");
```

**Implementation Tasks**:
1. Create `Treat` class (position, velocity, sprite ID, score value)
2. Create `TreatCatalog` with treat definitions
3. Spawn treats when enemies are defeated
4. Implement treat physics (thrown upward, then falls)
5. Implement treat-player collision (collect, add score)
6. Add treat rendering (colored rectangles for now)
7. Implement reward triggers (COMBO_STREAK, PLANNED_TAKEDOWN, etc.)

**Estimated Effort**: 10-12 hours

---

### Track 6.4: Pickup System
**Priority**: MEDIUM (Health/powerup items)
**Java Files**:
- `gameplay/components/Pickup.java`
- `ui/screens/GameScreen.java` (pickup collision)

**Description**:
Pickups spawn in levels and give the player benefits:
- **Heart**: Restore 1 life
- **Gem**: Bonus score
- **Lightning**: Speed boost (temporary)
- **Speed**: Movement speed boost (temporary)

**Current Status**: No pickup system exists.

**Implementation Tasks**:
1. Create `Pickup` class (type enum: HEART, GEM, LIGHTNING, SPEED)
2. Parse pickup spawn points from level data
3. Implement pickup-player collision
4. Apply pickup effects (heal, score, speed boost)
5. Add temporary effect timers (speed boost duration)
6. Add pickup rendering

**Estimated Effort**: 6-8 hours

---

## Phase 7: Advanced Enemy Types

### Track 7.1: Additional Enemy Variants
**Priority**: MEDIUM (Gameplay variety)
**Java Files**:
- `gameplay/enemies/AirbornePathEnemy.java` (flies along predefined path)
- `gameplay/enemies/FlyingEnemy.java` (flies toward player)
- `gameplay/enemies/GrimReaper.java` (spawns after time limit)
- `gameplay/enemies/GroundPatrolEnemy.java` (walks on ground)
- `gameplay/enemies/IdleEnemy.java` (stationary, no AI)
- `gameplay/enemies/LightningEnemy.java` (electrical hazard)
- `gameplay/enemies/SidewaysEnemy.java` (moves horizontally on walls)
- `gameplay/enemies/SidewaysJumpEnemy.java` (jumps sideways)
- `gameplay/enemies/WaterEnemy.java` (water-based enemy)

**Current Status**: Only 3 enemy types (Patroller, Jumper, Shooter).

**Description**:
RainbowGame has 9+ additional enemy types with unique behaviors. Most are more advanced than current implementations.

**Implementation Priority**:
1. **FlyingEnemy** (flies toward player, ignores gravity)
2. **AirbornePathEnemy** (follows waypoint path)
3. **GrimReaper** (time pressure mechanic)
4. **Others** (as needed for level variety)

**Estimated Effort**: 20-30 hours (all variants)

---

### Track 7.2: Enemy Element System
**Priority**: LOW (Advanced mechanic)
**Java Files**:
- `gameplay/enemies/EnemyElementType.java` (enum: NONE, RAINBOW, ICE, FIRE, LIGHTNING)
- `gameplay/enemies/EnemyElementInteractionSystem.java`

**Description**:
Enemies can have elemental types (Ice, Fire, Lightning, Rainbow) that affect interactions with droplets and the player.

**Estimated Effort**: 8-10 hours

---

## Phase 8: Sprite and Animation System

### Track 8.1: Entity Definition System
**Priority**: HIGH (Replace colored rectangles)
**Java Files**:
- `editor/entity/EntityDefinition.java`
- `editor/entity/EntityAnimation.java`
- `editor/entity/EntityFrame.java`
- `editor/entity/EntitySerializer.java`
- `editor/entity/TextureCatalog.java`
- `editor/entity/TextureDefinition.java`

**Description**:
RainbowGame uses a data-driven entity system where entities (player, enemies, tiles) are defined in JSON with sprite sheets, animations, hitboxes, etc.

**Current Status**: All entities render as colored rectangles (placeholder).

**Implementation Tasks**:
1. Create `EntityDefinition` (sprite paths, animations, hitboxes)
2. Create `EntityAnimation` (frame sequences, durations, loops)
3. Create `EntityFrame` (texture region, hitbox offset)
4. Load entity definitions from JSON
5. Replace all colored rectangle rendering with sprites
6. Implement animation state machines (idle, walk, jump, attack)

**Estimated Effort**: 16-20 hours

---

### Track 8.2: Texture Catalog System
**Priority**: MEDIUM (Sprite management)
**Java Files**:
- `editor/entity/TextureCatalog.java`
- `editor/entity/TextureDefinition.java`
- `editor/entity/TextureType.java` (enum: PLAYER, ENEMY, PICKUP, TILE, etc.)
- `editor/persistable/TexturePersistence.java`

**Description**:
Central registry of all game textures with metadata (dimensions, hitboxes, tags).

**Implementation Tasks**:
1. Create texture catalog JSON format
2. Load and cache textures
3. Provide texture lookup by ID
4. Support texture hotloading for editor

**Estimated Effort**: 6-8 hours

---

## Phase 9: Advanced Map Editor Features

### Track 9.1: Annotation System
**Priority**: HIGH (Level design workflow)
**Java Files**:
- `editor/annotation/AnnotationStore.java`
- `editor/annotation/AnnotationPipeline.java`
- `editor/annotation/MapAnnotationContext.java`
- `editor/annotation/EnemySpawnMap.java`
- `editor/annotation/DropletSpawnMap.java`
- `editor/annotation/GridAnnotationMask.java`
- `editor/annotation/WaterSystemSnapshot.java`
- `editor/annotation/EditorComment.java`

**Description**:
The Java editor has a sophisticated annotation system that stores additional metadata per map:
- **Enemy spawn points** (type, position, facing)
- **Droplet spawn points** (mode, density)
- **Water system config** (flow direction, speed)
- **Editor comments** (design notes)
- **Grid masks** (walkable areas, hazard zones)

This data is stored separately from the tile map and used to generate gameplay at runtime.

**Current Status**: MapEditor only edits tiles. No annotation layer exists.

**Implementation Tasks**:
1. Create annotation layer storage (separate from tile layers)
2. Add annotation painting tools (place enemy spawns, droplet zones)
3. Implement annotation serialization (save/load with map)
4. Update GameScreen to read enemy spawns from annotations (not hardcoded)
5. Add annotation visualization in editor (colored overlays, icons)

**Estimated Effort**: 20-24 hours

---

### Track 9.2: Entity Editor Screen
**Priority**: MEDIUM (Entity authoring)
**Java Files**:
- `editor/entity/EntityEditorScreen.java`
- `editor/entity/PreviewList.java`
- `editor/TextureListPanel.java`

**Description**:
Separate editor screen for creating and editing entity definitions (sprites, animations, hitboxes). Allows designers to:
- Import sprite sheets
- Define animation frames
- Set hitboxes visually
- Preview animations
- Export entity JSON

**Current Status**: No entity editor exists.

**Estimated Effort**: 24-30 hours

---

### Track 9.3: Map Import and Stitching
**Priority**: LOW (Advanced workflow)
**Java Files**:
- `editor/importer/MapImportSession.java`
- `editor/importer/OrbStitcher.java`
- `editor/importer/StoryMapManifest.java`

**Description**:
The Java editor can import maps from various sources and "stitch" multiple maps together into larger worlds (Orb stitching).

**Estimated Effort**: 16-20 hours

---

### Track 9.4: Editor Save System
**Priority**: MEDIUM (Multi-slot saves)
**Java Files**:
- `editor/save/EditorSaveManager.java`
- `editor/save/EditorSaveManager.SaveSnapshot`
- `editor/save/EditorSaveManager.SlotMetadata`

**Description**:
The Java editor has a save system with multiple slots, metadata (timestamp, preview), and versioning.

**Current Status**: MapEditor saves directly to file, no save slots.

**Estimated Effort**: 8-10 hours

---

### Track 9.5: Reference Layer System
**Priority**: LOW (Design aid)
**Java Files**: `editor/MapEditorScreen.java` (ReferenceImageLayer)

**Description**:
Editor can overlay reference images (concept art, mockups) to guide tile placement.

**Estimated Effort**: 6-8 hours

---

## Phase 10: Level Management and Progression

### Track 10.1: Level Manifest System
**Priority**: HIGH (World structure)
**Java Files**:
- `levels/LevelManifest.java`
- `levels/LevelLoader.java`
- `levels/LiveLevelRegistry.java`
- `levels/LiveLevelHandle.java`

**Description**:
RainbowGame has a manifest system that defines:
- Worlds (1-N)
- Levels per world (1-1, 1-2, 1-3, etc.)
- Level file paths
- Unlock conditions
- Par times, score thresholds

**Current Status**: Umbrella only loads single level, no world/level progression.

**Implementation Tasks**:
1. Create level manifest JSON format
2. Implement world/level progression UI
3. Store level completion state (stars, best time, high score)
4. Implement level unlocking logic
5. Add level select screen

**Estimated Effort**: 12-16 hours

---

### Track 10.2: Level Goal System
**Priority**: HIGH (Win condition)
**Java Files**: `ui/screens/GameScreen.java` (level complete detection)

**Description**:
Levels have a goal (reach exit, collect all droplets, defeat all enemies, time limit, etc.).

**Current Status**: LEVEL_COMPLETE state exists but no goal detection.

**Implementation Tasks**:
1. Add goal tiles to level format (exit portal, finish line)
2. Detect player reaching goal
3. Check goal conditions (all droplets collected, time remaining, etc.)
4. Trigger LEVEL_COMPLETE state
5. Calculate level score (time bonus, no-damage bonus, droplet bonus)
6. Progress to next level

**Estimated Effort**: 8-10 hours

---

## Phase 11: Debug and Development Tools

### Track 11.1: Debug Overlay
**Priority**: MEDIUM (Development aid)
**Java Files**: `debug/DebugOverlay.java`

**Description**:
In-game overlay showing:
- FPS counter
- Player position, velocity
- Collision boxes visualization
- Enemy count, state
- Droplet count
- Memory usage

**Estimated Effort**: 6-8 hours

---

### Track 11.2: Cheat System
**Priority**: LOW (Testing aid)
**Java Files**: `editor/MapEditorScreen.java` (cheat overlay, cheat pages)

**Description**:
The Java editor has a cheat overlay with keyboard shortcuts for:
- Noclip mode
- Invincibility
- Level skip
- Spawn enemies
- Clear droplets

**Estimated Effort**: 4-6 hours

---

## Phase 12: Polish and Visual Feedback

### Track 12.1: Particle Effects
**Priority**: LOW (Visual polish)

**Description**:
Missing particle systems for:
- Enemy defeat (explosion, poof)
- Droplet collection (sparkle)
- Player hit (impact flash)
- Parasol swing (trail)

**Estimated Effort**: 10-12 hours

---

### Track 12.2: Screen Shake and Camera Effects
**Priority**: LOW (Juice)
**Java Files**: `camera/CameraSystem.java`

**Description**:
Camera shake on:
- Player hit
- Enemy defeat
- Explosion/impact

**Estimated Effort**: 4-6 hours

---

### Track 12.3: Audio System Integration
**Priority**: MEDIUM (Feedback)
**Java Files**: `assets/AudioSystem.java`

**Description**:
Currently AudioSystem.cpp exists but is not integrated into gameplay. Need:
- Jump sound
- Land sound
- Hit/damage sound
- Enemy defeat sound
- Droplet collect sound
- Parasol attack sound
- Level music
- Menu navigation sounds

**Current Status**: AudioSystem exists, not used in GameScreen.

**Implementation Tasks**:
1. Load sound effect assets
2. Add sound playback calls in Player, Enemy, GameScreen
3. Implement background music
4. Add volume controls

**Estimated Effort**: 6-8 hours

---

## Comparison Summary

| **Feature Category** | **Java (RainbowGame)** | **U++ (Umbrella)** | **Status** |
|----------------------|-----------------------|-------------------|-----------|
| Core Platformer Physics | ✓ | ✓ | COMPLETE |
| Basic Enemies (3 types) | ✓ | ✓ | COMPLETE |
| HUD (lives, score) | ✓ | ✓ | COMPLETE |
| Game States (pause, game over) | ✓ | ✓ | COMPLETE |
| Water/Droplet System | ✓ | ✗ | MISSING |
| Parasol Attack | ✓ | ✗ | MISSING |
| Rewards/Treats | ✓ | ✗ | MISSING |
| Advanced Enemies (9+ types) | ✓ | ✗ | MISSING |
| Pickups (health, powerups) | ✓ | ✗ | MISSING |
| Sprite/Animation System | ✓ | ✗ | MISSING |
| Entity Definition System | ✓ | ✗ | MISSING |
| Annotation System | ✓ | ✗ | MISSING |
| Entity Editor | ✓ | ✗ | MISSING |
| Level Manifest/Progression | ✓ | ✗ | MISSING |
| Level Goal System | ✓ | Partial | INCOMPLETE |
| Debug Overlay | ✓ | ✗ | MISSING |
| Particle Effects | ✓ | ✗ | MISSING |
| Audio Integration | ✓ | ✗ | MISSING |
| Camera Effects | ✓ | ✗ | MISSING |

---

## Recommended Implementation Order

### Immediate Priority (Next 2-3 weeks)
1. **Track 6.2**: Parasol Attack System (core gameplay loop)
2. **Track 6.1**: Water/Droplet System (primary collectible)
3. **Track 6.3**: Rewards/Treats System (scoring enhancement)
4. **Track 10.2**: Level Goal System (win condition)

### High Priority (Next 4-6 weeks)
5. **Track 8.1**: Entity Definition System (replace placeholders)
6. **Track 9.1**: Annotation System (level design workflow)
7. **Track 10.1**: Level Manifest (world progression)
8. **Track 6.4**: Pickup System (health/powerups)

### Medium Priority (Next 8-12 weeks)
9. **Track 7.1**: Additional Enemy Variants (gameplay variety)
10. **Track 8.2**: Texture Catalog (sprite management)
11. **Track 9.2**: Entity Editor (entity authoring)
12. **Track 12.3**: Audio Integration (feedback)

### Low Priority (Backlog)
13. **Track 11.1**: Debug Overlay
14. **Track 9.4**: Editor Save System
15. **Track 12.1**: Particle Effects
16. **Track 12.2**: Camera Effects
17. **Track 7.2**: Enemy Element System
18. **Track 9.3**: Map Import/Stitching
19. **Track 9.5**: Reference Layer System
20. **Track 11.2**: Cheat System

---

## Total Estimated Effort

- **Immediate Priority**: 36-46 hours
- **High Priority**: 52-66 hours
- **Medium Priority**: 60-78 hours
- **Low Priority**: 68-88 hours
- **Grand Total**: 216-278 hours (~5-7 months at 40 hrs/week)

---

## Notes

- The Java implementation is ~12,600 lines (MapEditorScreen alone is 10,712 lines)
- The Java implementation has been in development much longer and has extensive polish
- Many Java features (particle effects, advanced editor tools) are "nice to have" rather than essential
- Focus should remain on core gameplay loop first, then expand
- The U++ implementation has solid fundamentals (physics, enemies, game state) to build upon

---

## Next Steps

1. Review this document with project stakeholders
2. Prioritize tracks based on project goals
3. Break down highest-priority tracks into implementable tasks
4. Update GameplayImplementation.md with new phases/tasks
5. Begin implementation of parasol attack and water systems
