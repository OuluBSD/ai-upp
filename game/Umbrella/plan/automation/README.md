# Automation Tracks - Umbrella Game

This directory contains the automation infrastructure planning for the Umbrella game project. The automation system enables:

1. **Automated Testing**: Script both editor and gameplay interactions
2. **Regression Prevention**: Catch breakages automatically
3. **QA Efficiency**: Reduce manual testing workload
4. **AI Playthrough**: Automated game completion for validation

## Architecture Overview

The automation system follows the proven AriaHub/MaestroHub pattern using ByteVM:

```
┌─────────────────────────────────────────────────────────┐
│            Python Test Scripts (.py files)              │
│   - Editor: test_brush_tool.py, test_layer_ops.py      │
│   - Gameplay: test_player_movement.py, test_ai.py      │
└──────────────────────┬──────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────┐
│                    PyVM (ByteVM)                        │
│   - Tokenize, compile, execute Python bytecode          │
│   - Expose game APIs via PyValue::Function()            │
└──────────────────────┬──────────────────────────────────┘
                       │
          ┌────────────┴────────────┐
          ▼                         ▼
┌──────────────────────┐  ┌──────────────────────┐
│  GameScriptBridge    │  │  EditorScriptBridge   │
│  - Game state API    │  │  - GUI automation     │
│  - Input injection   │  │  - Ctrl::Access()     │
│  - Frame stepping    │  │  - Element discovery  │
│  - Entity queries    │  │  - Click/type/verify  │
└──────────────────────┘  └──────────────────────┘
          │                         │
          ▼                         ▼
┌──────────────────────┐  ┌──────────────────────┐
│   GameScreen         │  │   MapEditorApp       │
│   - Public members   │  │   - Dockable panels  │
│   - GameTick()       │  │   - Tools/Layers     │
│   - State machine    │  │   - Entity placement │
└──────────────────────┘  └──────────────────────┘
```

## Track Structure

### Track 1: Foundation (automation/foundation/)
Building blocks for automation infrastructure.

**Phase 1: ByteVM Integration**
- Add dependencies: ByteVM, Ctrl/Automation packages
- Command-line flag: `--test <script.py>`
- PyVM initialization and script execution

**Phase 2: Game State API**
- Expose player state (position, velocity, parasol, lives, score)
- Expose entity arrays (enemies, treats, droplets)
- Expose collision/physics queries
- Expose level/map state (tiles, layers, spawn points)

**Phase 3: Input Injection System**
- Direct InputState manipulation
- Frame-by-frame GameTick control
- Deterministic random seeding (per-entity RNG)
- Timestep following for real-time automation

**Phase 4: Editor Automation**
- Implement Ctrl::Access() for all MapEditor controls
- GUI automation visitor pattern
- Editor test scripts (tools, layers, save/load)

**Phase 5: Gameplay Automation**
- Game event hooks (enemy death, level complete, collect treat)
- Entity assertion framework (position, velocity, alive state)
- Gameplay test scripts (movement, combat, AI behavior)
- Replay system (record input trace, playback for regression)

### Track 2: AI Planning (automation/ai-planner/)
Automated playthrough using A* and action planning.

**Phase 1: Pathfinding Foundation**
- A* algorithm implementation
- Level navigation graph (walkable tiles, jump arcs)
- Movement cost calculation (distance, danger zones)

**Phase 2: Action Planner**
- Action primitives: move, jump, attack, glide, capture_enemy
- Goal system: reach_position, kill_all_enemies, collect_treats
- Plan execution engine with real-time replanning

**Phase 3: Integration**
- Auto-player controller (plan → input injection)
- Full game playthrough automation (all 36 levels)
- Performance metrics (completion time, deaths, score)
- Level validation (solvability, difficulty rating)

## Dependencies

**U++ Packages:**
- ByteVM: Python VM for script execution
- Ctrl/Automation: GUI automation visitor and bindings
- CtrlLib: Base GUI framework
- Core: U++ core library

**Python API Functions (planned):**
```python
# Game state queries
get_player_position() -> [x, y]
get_player_velocity() -> [vx, vy]
get_player_lives() -> int
get_enemy_count() -> int
get_enemy_position(index) -> [x, y]

# Game control
set_player_position(x, y)
simulate_input(key, is_pressed)
wait_frames(count)
wait_time(seconds)
load_level(path)

# Assertions
assert_player_alive()
assert_enemy_dead(index)
assert_position_near(actual, expected, tolerance)

# Editor functions
find(element_path) -> AutomationElement
click(element_path)
set_value(element_path, value)
dump_ui() -> str
```

## Testing Strategy

**Unit Tests** (C++)
- Existing: `bin/GameScreenTest`
- Tests individual components in isolation
- No GUI, no automation required

**Integration Tests** (Python via ByteVM)
- Editor: Tool correctness, layer operations, save/load
- Gameplay: Player physics, enemy AI, collision detection
- Scenarios: Level transitions, game over, level complete

**System Tests** (Python + AI Planner)
- Full playthrough of all 36 levels
- Difficulty validation
- Performance benchmarks

## Development Workflow

1. **Start with Foundation Phase 1**: Add ByteVM dependencies first
2. **Implement incrementally**: One API function at a time, test immediately
3. **Follow AriaHub patterns**: Use proven examples from `uppsrc/AriaHub/tests/`
4. **Write tests early**: Create test scripts as APIs are exposed
5. **Document thoroughly**: Update this README and task files as work progresses

## Success Criteria

**Foundation Track Complete When:**
- [ ] `bin/Umbrella --test tests/editor/test_brush_tool.py` passes
- [ ] `bin/Umbrella --test tests/gameplay/test_player_movement.py` passes
- [ ] Deterministic replay works (same inputs → same outcomes)
- [ ] All editor features testable via Python

**AI Planner Track Complete When:**
- [ ] Auto-player completes world1-stage1 without human input
- [ ] Auto-player completes all 36 levels with success rate > 90%
- [ ] Level solvability validation runs in CI pipeline
- [ ] Difficulty ratings generated for all levels

## Reference Materials

**Upstream Examples:**
- `uppsrc/AriaHub/main.cpp` (lines 166-204): --test flag integration
- `uppsrc/AriaHub/tests/*.py`: GUI automation test scripts
- `uppsrc/Ctrl/Automation/Automation.cpp`: Python API implementations
- `examples/SingerTrainer/Scripting.cpp`: Custom domain bindings

**Key Documentation:**
- `uppsrc/ByteVM/README.md`: PyVM architecture
- `uppsrc/AI/plan/MaestroHub/14_TestAutomation/`: Automation implementation plan
- This file: Architecture and task breakdown

## Current Status

**Not Started**: Both tracks are planned but not yet implemented.

**Next Step**: Begin Foundation Phase 1, Task 1 (Add ByteVM dependencies to Umbrella.upp)

See `plan/cookie.txt` for ongoing progress updates.
