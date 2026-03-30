# Adventure Esc → PyVM Conversion Plan

## Overview

Convert Adventure package from Esc VM to PyVM (Python VM).

**Scope**: Only Adventure package and direct dependencies (not entire codebase)

## Current State

### Esc Usage in Adventure

| Component | Esc Types | Usage Count |
|-----------|-----------|-------------|
| Program.h | EscValue, SObj | ~100 references |
| Program.h | EscEscape | ~40 function declarations |
| Program.h | EscAnimContext | 1 (ctx member) |
| Program.h | EscAnimProgram | 2 (Script inheritance) |
| AdventureBindings.cpp | EscValue | ~30 conversions |
| Room.cpp | EscValue/SObj | Room management |
| Script.cpp | EscEscape | Script execution |

### Key Esc Types to Replace

1. **EscValue** → **PyValue**
   - Used for all game objects, rooms, actors
   - Map/dict storage
   - Function references

2. **EscEscape** → **PyVM**
   - Script execution engine
   - Lambda execution

3. **EscAnimProgram** → **Script** (custom PyVM wrapper)
   - Script state management
   - Background/foreground execution

4. **EscAnimContext** → Keep for animation system only
   - Animation is separate from scripting
   - Can keep EscAnim for animations while using PyVM for scripts

## Conversion Strategy

### Phase 1: Data Types (EscValue → PyValue)

**Files**: `AdventureBindings.cpp`, `Program.h`

Replace `EscValue` with `PyValue` in:
- Function parameters
- Member variables
- Local variables
- Type conversions

**Pattern**:
```cpp
// Before (Esc)
EscValue room = args[0];
room.IsMap()

// After (PyVM)
PyValue room = args[0];
room.GetType() == PY_DICT
```

### Phase 2: Script Execution (EscEscape → PyVM)

**Files**: `Script.cpp`, `Program.h/cpp`

Replace Esc script execution with PyVM:
- `EscEscape::Execute()` → `PyVM::Call()`
- Lambda storage: `EscValue` lambdas → `PyValue` functions
- Script state: Keep `Script` class but change base

**Pattern**:
```cpp
// Before (Esc)
EscEscape esc;
esc.Execute(lambda, args);

// After (PyVM)
PyVM vm;
vm.Call(py_func, args);
```

### Phase 3: Room/Callback Integration

**Files**: `Room.cpp`, `Program.cpp`

Update room enter/exit callbacks:
- Store Python functions from room dicts
- Call via `PyVM::Call()` instead of `StartScriptEsc()`

**Pattern**:
```cpp
// Before (Esc)
StartScriptEsc(&room_curr, "enter", 0, room_curr);

// After (PyVM)
PyValue enter_fn = room_curr.GetItem("enter");
if(enter_fn.IsFunction())
    py_vm.Call(enter_fn, {room_curr});
```

### Phase 4: Cleanup

- Remove Esc includes where no longer needed
- Keep `EscAnimContext` for animation system only
- Update type aliases (`SObj` → use `PyValue` directly)

## Files to Modify

| File | Changes | Priority |
|------|---------|----------|
| `AdventureBindings.cpp` | PyToEscValue → identity (PyValue stays PyValue) | High |
| `Program.h` | EscValue → PyValue, EscEscape* → PyVM* | High |
| `Program.cpp` | Script execution, room management | High |
| `Script.cpp` | Script class implementation | High |
| `Room.cpp` | ChangeRoom, callbacks | Medium |
| `Adventure.h` | Include PyVM instead of Esc | Medium |

## Testing Strategy

1. **Unit tests**: Test individual bindings with PyValue
2. **Integration tests**: Test room loading, script execution
3. **Full game test**: Run Game.py end-to-end

## Risks

1. **Animation system**: Keep EscAnimContext for animations
2. **Backward compat**: ESC scripts may still exist (Game.esc)
3. **Type conversions**: PyValue ↔ EscValue for animation objects

## Acceptance Criteria

- [ ] No EscValue in AdventureBindings.cpp
- [ ] No EscEscape in Program.h/cpp
- [ ] Python room enter/exit callbacks work
- [ ] Game.py runs without Esc dependencies
- [ ] Build succeeds: `script/build.py Adventure`
- [ ] Game runs: `bin/Adventure`

## Estimated Effort

- Phase 1: 4-6 hours
- Phase 2: 6-8 hours
- Phase 3: 4-6 hours
- Phase 4: 2-4 hours
- **Total**: 16-24 hours (2-3 days)
