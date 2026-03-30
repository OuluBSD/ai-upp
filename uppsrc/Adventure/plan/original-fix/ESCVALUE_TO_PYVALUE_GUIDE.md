# EscValue → PyValue Conversion Guide

## Overview

The Adventure package has been converted from Esc scripts to Python scripts. However, **game data** is still stored in `EscValue` types. This document describes the remaining work to convert game data to `PyValue`.

## Current State

### ✅ Completed
- Esc script execution removed (317 lines deleted)
- Esc script files deleted (6 .esc files)
- Python bindings for engine functions (52 functions)
- PyVM integration for script execution

### ⏳ Remaining: Game Data Conversion

**Game data still uses EscValue:**
- `room_curr` - Current room object
- `rooms` - All rooms array
- `verbs` - Verb definitions
- `selected_actor` - Current actor
- Object/actor properties (x, y, state, inventory, etc.)

## Files Requiring Changes

### 1. Program.h (Core Data Members)

**Convert these members from EscValue to PyValue:**
```cpp
// Current (EscValue)
EscValue rooms;
EscValue cutscene_override;
EscValue verbs;
EscValue V_DEFAULT, V_USE, ...;
EscValue room_curr;
SObj cam_following_actor;  // SObj = EscValue
SObj talking_actor;
EscValue ui_arrows;
EscValue hover_curr_verb;
SObj noun1_curr, noun2_curr;
SObj hover_curr_object;
```

**To:**
```cpp
// Target (PyValue)
PyValue rooms;
PyValue cutscene_override;
PyValue verbs;
PyValue V_DEFAULT, V_USE, ...;
PyValue room_curr;
PyValue cam_following_actor;
PyValue talking_actor;
PyValue ui_arrows;
PyValue hover_curr_verb;
PyValue noun1_curr, PyValue noun2_curr;
PyValue hover_curr_object;
```

### 2. Program.h (Method Signatures)

**Convert these methods:**
```cpp
// Current
EscValue GetVerb(int idx);
String GetVerbString(SObj v);
PyValue GetSelectedActor();
void SetSelectedActor(const PyValue& actor);
PyValue GetInRoom(const PyValue& o);
// ... many more

// Target - all use PyValue instead of EscValue/SObj
```

### 3. ProgramDraw.cpp (Painting)

**Heavy EscValue usage for rendering:**
- Line 180: `EscValue map = room_curr("map");`
- Line 344: `EscValue inventory = selected_actor("inventory");`
- Line 593: `EscValue draw = o("draw");`
- Line 663: `EscValue curr_anim = obj("curr_anim");`
- Line 700: `EscValue r = p->room_curr;`
- Many more...

**Conversion pattern:**
```cpp
// Before (EscValue)
EscValue map = room_curr("map");
int celx = map(0);

// After (PyValue)
PyValue map = GetDictItem(room_curr, "map");
int celx = PyInt(map.GetList()[0]);
```

### 4. Physics.cpp (Collision Detection)

**EscValue usage:**
- Line 36: `EscValue objects = room_curr("objects");`
- Line 37: `Vector<EscValue>& room_arr = const_cast<Vector<EscValue>&>(objects.GetArray());`
- Line 40: `EscValue c = Classes(obj);`
- Line 135-138: Property access (`obj.MapGet("y")`, etc.)

**Conversion:**
```cpp
// Before
EscValue bounds = obj("bounds");

// After
PyValue bounds = GetDictItem(obj, "bounds");
```

### 5. Actor.cpp (Actor Logic)

**EscValue usage:**
- Line 10: `return global.Get("selected_actor", EscValue());`
- Line 82: `EscValue selected_actor = ctx.GetGlobal("selected_actor");`
- Line 105: `EscValue map = room_curr("map");`
- Line 217: `bool Program::VerbScript(EscValue vc2)`

### 6. Game.cpp (Game State)

**EscValue usage:**
- Line 12: `game = global.Get("game", EscValue());`
- Line 40: `rooms = global.Get("rooms", EscValue());`
- Line 53: `const Vector<EscValue>& verb_arr = verbs.GetArray();`

### 7. Ui.cpp (UI State)

**EscValue usage:**
- Line 31-38: Arrow definitions
- Line 128: `EscValue noun1_curr, noun2_curr;`
- Line 211: `EscValue Program::RunLambda1(...)`

### 8. Camera.cpp (Camera Logic)

**EscValue usage:**
- Line 36: Actor comparison
- Line 100-101: `room_curr.IsMap()`, `room_curr.MapGet("map")`

### 9. Room.cpp (Room Management)

**EscValue usage:**
- Line 77: `EscValue selected_actor = GetSelectedActor();`
- Line 156, 189: `StartScriptEsc(&room_curr, ...)`
- Line 279: `Vector<EscValue>& arr = (Vector<EscValue>&)objects.GetArray();`

### 10. Gfx.cpp (Graphics Helpers)

**EscValue usage:**
- Line 36: `Color ReadColor(const SObj& o, EscValue key, Color def)`
- Line 48: `bool TryReadColor(const SObj& o, EscValue key, Color& c)`

## Helper Functions Needed

Add these to `Program.h`/`Program.cpp`:

```cpp
// PyValue helpers for common EscValue operations
namespace Adventure {

// Get dict item by string key
PyValue GetDictItem(const PyValue& dict, const char* key);

// Get int from PyValue (handles int/float)
int PyInt(const PyValue& v, int def = 0);

// Get string from PyValue
String PyStr(const PyValue& v);

// Get list from PyValue
const Vector<PyValue>& PyList(const PyValue& v);

// Check if PyValue is dict
bool PyIsDict(const PyValue& v);

// Check if PyValue is list
bool PyIsList(const PyValue& v);

// Get property (shorthand for GetDictItem)
PyValue GetProp(const PyValue& obj, const char* key);

// Set property
void SetProp(PyValue& obj, const char* key, const PyValue& val);

}
```

## Conversion Strategy

### Phase 1: Core Game State
1. Convert `room_curr` to PyValue
2. Convert `rooms` to PyValue
3. Convert `selected_actor` to PyValue
4. Update `GetSelectedActor()` / `SetSelectedActor()`

### Phase 2: Object/Actor Properties
1. Convert property access helpers (`GetXY`, `GetState`, etc.)
2. Update `Classes()`, `GetInRoom()`
3. Update `RecalculateBounds()`

### Phase 3: Rendering
1. Update `ProgramDraw::PaintRoom()`
2. Update `ProgramDraw::PaintObject()`
3. Update `ProgramDraw::PaintActor()`

### Phase 4: Physics/Collision
1. Update `Physics.cpp` collision detection
2. Update bounds checking
3. Update object iteration

### Phase 5: UI/Actors
1. Update UI state variables
2. Update actor logic
3. Update camera logic

### Phase 6: Cleanup
1. Remove EscValue includes where no longer needed
2. Remove EscAnimContext dependency (keep only for animation)
3. Final testing

## Testing

After each phase:
```bash
script/build.py -mc 1 Adventure
bin/Adventure  # Should load Game.py and run
```

## Estimated Effort

- **Phase 1-2**: 4-6 hours (core data)
- **Phase 3**: 4-6 hours (rendering)
- **Phase 4**: 2-4 hours (physics)
- **Phase 5**: 4-6 hours (UI/actors)
- **Phase 6**: 2-4 hours (cleanup)
- **Total**: 16-26 hours (2-3 days)

## Notes

- Keep `EscAnimContext` for animation system (separate concern)
- `EscValue` → `PyValue` is a data format change, not a logic change
- Test frequently after each conversion
- Use helper functions to reduce code duplication
