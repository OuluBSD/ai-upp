# Task: Player State API Exposure

## Overview
Expose player state (position, velocity, parasol state, lives, score) to Python scripts via `GameScriptBridge` class.

## Objective
Enable Python test scripts to query and manipulate player state for automated gameplay testing.

## Prerequisites
- **Phase 1 Task 1** complete: ByteVM integrated, `--test` flag working
- Understanding of `PyValue::Function()` for exposing C++ functions to Python
- Familiarity with `game/Umbrella/Player.h` API

## Background

The Player class (Player.h/cpp) contains all player state:
```cpp
class Player {
public:
    Rectf GetBounds() const;
    Pointf GetVelocity() const;
    ParasolState GetParasolState() const;  // IDLE, ATTACKING, GLIDING
    int GetLives() const;
    int GetScore() const;
    int GetFacing() const;  // -1 left, 1 right
    bool IsOnGround() const;

    void SetPosition(float x, float y);
    void SetVelocity(const Pointf& vel);
    void AddScore(int points);
    void TakeDamage();
};
```

We need to expose these methods to Python for test automation.

## Implementation Steps

### Step 1: Create GameScriptBridge Class
**Files**: `game/Umbrella/GameScriptBridge.h`, `GameScriptBridge.cpp`

**Header** (`GameScriptBridge.h`):
```cpp
#ifndef _Umbrella_GameScriptBridge_h_
#define _Umbrella_GameScriptBridge_h_

#include <ByteVM/ByteVM.h>
#include "GameScreen.h"

using namespace Upp;

class GameScriptBridge {
private:
    PyVM vm;
    GameScreen* gameScreen;  // Reference to active game

public:
    GameScriptBridge();

    void SetGameScreen(GameScreen* screen) { gameScreen = screen; }
    GameScreen* GetGameScreen() { return gameScreen; }

    PyVM& GetVM() { return vm; }

    // Register all game API functions
    void RegisterGameAPI();

    // Execute script file
    bool ExecuteScript(const String& scriptPath);
};

#endif
```

**Implementation** (`GameScriptBridge.cpp` - skeleton):
```cpp
#include "Umbrella.h"
#include "GameScriptBridge.h"

GameScriptBridge::GameScriptBridge() {
    gameScreen = nullptr;
}

void GameScriptBridge::RegisterGameAPI() {
    // Will add API function registrations here
}

bool GameScriptBridge::ExecuteScript(const String& scriptPath) {
    if(!FileExists(scriptPath)) {
        LOG("ERROR: Script not found: " << scriptPath);
        return false;
    }

    String source = LoadFile(scriptPath);
    if(source.IsEmpty()) {
        LOG("ERROR: Failed to load script: " << scriptPath);
        return false;
    }

    try {
        Tokenizer tokenizer;
        tokenizer.SkipPythonComments(true);
        tokenizer.Process(source, scriptPath);
        tokenizer.CombineTokens();

        PyCompiler compiler(tokenizer.GetTokens());
        Vector<PyIR> ir;
        if(!compiler.Compile(ir)) {
            LOG("ERROR: Failed to compile script");
            return false;
        }

        vm.SetIR(ir);
        vm.Run();
        return true;
    }
    catch(const Exc& e) {
        LOG("ERROR: Script exception: " << e);
        return false;
    }
}
```

### Step 2: Implement Player State Query Functions
**File**: `game/Umbrella/GameScriptBridge.cpp`

Add static helper functions (these will be exposed to Python):
```cpp
// Python API: get_player_position() -> [x, y]
static PyValue py_get_player_position(const Vector<PyValue>& args, void* user_data) {
    GameScriptBridge* bridge = (GameScriptBridge*)user_data;
    if(!bridge || !bridge->GetGameScreen()) {
        return PyValue::None();
    }

    const Player& player = bridge->GetGameScreen()->player;
    Rectf bounds = player.GetBounds();

    // Return list [x, y] (center of player bounds)
    float centerX = bounds.left + bounds.Width() / 2.0f;
    float centerY = bounds.top + bounds.Height() / 2.0f;

    Vector<PyValue> result;
    result.Add(PyValue((double)centerX));
    result.Add(PyValue((double)centerY));
    return PyValue::List(result);
}

// Python API: get_player_velocity() -> [vx, vy]
static PyValue py_get_player_velocity(const Vector<PyValue>& args, void* user_data) {
    GameScriptBridge* bridge = (GameScriptBridge*)user_data;
    if(!bridge || !bridge->GetGameScreen()) {
        return PyValue::None();
    }

    const Player& player = bridge->GetGameScreen()->player;
    Pointf vel = player.GetVelocity();

    Vector<PyValue> result;
    result.Add(PyValue((double)vel.x));
    result.Add(PyValue((double)vel.y));
    return PyValue::List(result);
}

// Python API: get_player_lives() -> int
static PyValue py_get_player_lives(const Vector<PyValue>& args, void* user_data) {
    GameScriptBridge* bridge = (GameScriptBridge*)user_data;
    if(!bridge || !bridge->GetGameScreen()) {
        return PyValue::None();
    }

    int lives = bridge->GetGameScreen()->player.GetLives();
    return PyValue((int64)lives);
}

// Python API: get_player_score() -> int
static PyValue py_get_player_score(const Vector<PyValue>& args, void* user_data) {
    GameScriptBridge* bridge = (GameScriptBridge*)user_data;
    if(!bridge || !bridge->GetGameScreen()) {
        return PyValue::None();
    }

    int score = bridge->GetGameScreen()->player.GetScore();
    return PyValue((int64)score);
}

// Python API: get_player_state() -> str ("IDLE", "ATTACKING", "GLIDING")
static PyValue py_get_player_state(const Vector<PyValue>& args, void* user_data) {
    GameScriptBridge* bridge = (GameScriptBridge*)user_data;
    if(!bridge || !bridge->GetGameScreen()) {
        return PyValue::None();
    }

    ParasolState state = bridge->GetGameScreen()->player.GetParasolState();
    String stateStr;
    switch(state) {
        case PARASOL_IDLE: stateStr = "IDLE"; break;
        case PARASOL_ATTACKING: stateStr = "ATTACKING"; break;
        case PARASOL_GLIDING: stateStr = "GLIDING"; break;
    }

    return PyValue(stateStr.ToStd());
}

// Python API: is_player_on_ground() -> bool
static PyValue py_is_player_on_ground(const Vector<PyValue>& args, void* user_data) {
    GameScriptBridge* bridge = (GameScriptBridge*)user_data;
    if(!bridge || !bridge->GetGameScreen()) {
        return PyValue::None();
    }

    bool onGround = bridge->GetGameScreen()->player.IsOnGround();
    return PyValue(onGround);
}
```

### Step 3: Implement Player State Manipulation Functions
**File**: `game/Umbrella/GameScriptBridge.cpp`

```cpp
// Python API: set_player_position(x, y)
static PyValue py_set_player_position(const Vector<PyValue>& args, void* user_data) {
    GameScriptBridge* bridge = (GameScriptBridge*)user_data;
    if(!bridge || !bridge->GetGameScreen()) {
        return PyValue::None();
    }

    if(args.GetCount() < 2) {
        LOG("ERROR: set_player_position requires 2 arguments (x, y)");
        return PyValue::None();
    }

    float x = (float)args[0].ToFloat();
    float y = (float)args[1].ToFloat();

    bridge->GetGameScreen()->player.SetPosition(x, y);
    return PyValue::None();
}

// Python API: set_player_velocity(vx, vy)
static PyValue py_set_player_velocity(const Vector<PyValue>& args, void* user_data) {
    GameScriptBridge* bridge = (GameScriptBridge*)user_data;
    if(!bridge || !bridge->GetGameScreen()) {
        return PyValue::None();
    }

    if(args.GetCount() < 2) {
        LOG("ERROR: set_player_velocity requires 2 arguments (vx, vy)");
        return PyValue::None();
    }

    float vx = (float)args[0].ToFloat();
    float vy = (float)args[1].ToFloat();

    bridge->GetGameScreen()->player.SetVelocity(Pointf(vx, vy));
    return PyValue::None();
}

// Python API: add_player_score(points)
static PyValue py_add_player_score(const Vector<PyValue>& args, void* user_data) {
    GameScriptBridge* bridge = (GameScriptBridge*)user_data;
    if(!bridge || !bridge->GetGameScreen()) {
        return PyValue::None();
    }

    if(args.GetCount() < 1) {
        LOG("ERROR: add_player_score requires 1 argument (points)");
        return PyValue::None();
    }

    int points = (int)args[0].ToInt();
    bridge->GetGameScreen()->player.AddScore(points);
    return PyValue::None();
}
```

### Step 4: Register Functions with PyVM
**File**: `game/Umbrella/GameScriptBridge.cpp`

Update `RegisterGameAPI()`:
```cpp
void GameScriptBridge::RegisterGameAPI() {
    VectorMap<PyValue, PyValue>& globals = vm.GetGlobals();

    // Query functions
    globals.GetAdd("get_player_position") =
        PyValue::Function("get_player_position", py_get_player_position, this);
    globals.GetAdd("get_player_velocity") =
        PyValue::Function("get_player_velocity", py_get_player_velocity, this);
    globals.GetAdd("get_player_lives") =
        PyValue::Function("get_player_lives", py_get_player_lives, this);
    globals.GetAdd("get_player_score") =
        PyValue::Function("get_player_score", py_get_player_score, this);
    globals.GetAdd("get_player_state") =
        PyValue::Function("get_player_state", py_get_player_state, this);
    globals.GetAdd("is_player_on_ground") =
        PyValue::Function("is_player_on_ground", py_is_player_on_ground, this);

    // Manipulation functions
    globals.GetAdd("set_player_position") =
        PyValue::Function("set_player_position", py_set_player_position, this);
    globals.GetAdd("set_player_velocity") =
        PyValue::Function("set_player_velocity", py_set_player_velocity, this);
    globals.GetAdd("add_player_score") =
        PyValue::Function("add_player_score", py_add_player_score, this);
}
```

### Step 5: Integrate with Test Mode
**File**: `game/Umbrella/main.cpp`

Update `RunAutomationTest()` to create GameScreen and bridge:
```cpp
int RunAutomationTest(const String& scriptPath) {
    // Create game screen in headless mode (no window)
    GameScreen* screen = new GameScreen();
    screen->LoadLevel("share/mods/umbrella/levels/world1-stage1.json");

    // Create script bridge
    GameScriptBridge bridge;
    bridge.SetGameScreen(screen);
    bridge.RegisterGameAPI();

    // Execute script
    bool success = bridge.ExecuteScript(scriptPath);

    delete screen;
    return success ? 0 : 1;
}
```

**Note**: May need to add headless mode support to GameScreen (disable rendering, no GUI)

### Step 6: Create Test Script
**File**: `game/Umbrella/tests/gameplay/test_player_state.py`

```python
# Test player state query and manipulation

print("Testing player state API...")

# Query initial state
pos = get_player_position()
print("Initial position:", pos)

vel = get_player_velocity()
print("Initial velocity:", vel)

lives = get_player_lives()
print("Lives:", lives)

score = get_player_score()
print("Score:", score)

state = get_player_state()
print("Parasol state:", state)

on_ground = is_player_on_ground()
print("On ground:", on_ground)

# Test manipulation
set_player_position(100.0, 200.0)
new_pos = get_player_position()
print("New position after set:", new_pos)

assert abs(new_pos[0] - 100.0) < 1.0, "X position not set correctly"
assert abs(new_pos[1] - 200.0) < 1.0, "Y position not set correctly"

set_player_velocity(50.0, -100.0)
new_vel = get_player_velocity()
print("New velocity:", new_vel)

add_player_score(500)
new_score = get_player_score()
print("Score after add:", new_score)

assert new_score == score + 500, "Score not added correctly"

print("All player state tests passed!")
```

## Testing

### Build Test
```bash
script/build.py -mc 1 -j 12 Umbrella
```
Expected: Build succeeds

### Execution Test
```bash
bin/Umbrella --test game/Umbrella/tests/gameplay/test_player_state.py
```
Expected output:
```
Testing player state API...
Initial position: [56.0, 280.0]
Initial velocity: [0.0, 0.0]
Lives: 3
Score: 0
Parasol state: IDLE
On ground: True
New position after set: [100.0, 200.0]
New velocity: [50.0, -100.0]
Score after add: 500
All player state tests passed!
```

## Success Criteria
- [ ] GameScriptBridge class created and integrated
- [ ] All 9 player API functions implemented and registered
- [ ] Test script executes without errors
- [ ] Position/velocity manipulation works correctly
- [ ] Assertions pass
- [ ] No memory leaks

## Known Issues / Gotchas
1. **Headless Mode**: GameScreen may need modifications to run without GUI
2. **PyValue Conversion**: Ensure proper type conversion (float ↔ double, int ↔ int64)
3. **Null Checks**: Always check `bridge` and `gameScreen` pointers
4. **User Data**: Pass `this` pointer correctly to PyValue::Function()

## Next Steps
- **Phase 2 Task 2**: Expose enemy array API
- **Phase 2 Task 3**: Expose collision/physics queries
- **Phase 2 Task 4**: Expose level/map state

## References
- `examples/SingerTrainer/Scripting.cpp`: Complete custom API example
- `game/Umbrella/Player.h`: Player class API
- `uppsrc/ByteVM/PyValue.h`: PyValue::Function() documentation
