# Task 001: Analyze ESC Usage

## Objective

Catalog all engine function calls used in ESC files to understand what needs Python bindings.

## Steps

1. Read all `.esc` files in `uppsrc/Adventure/`
2. Extract all function calls (patterns like `function_name(...)`)
3. Categorize by type (core, UI, drawing, game logic)
4. Create a master list with usage counts

## Files to Analyze

- `Game.esc` (main file - ~1563 lines)
- `CarverTest.esc`
- `Demo.esc`
- `C8_Intro.esc`
- `C8_Part1.esc`
- `C8_Part2.esc`

## Expected Output

Create `uppsrc/Adventure/plan/original-fix/01-bindings/function-catalog.md` with:

```markdown
# Engine Function Catalog

## Core Functions
- change_room(room, fade) - Used 15 times in Game.esc
- camera_follow(actor) - Used 8 times
- camera_at(position) - Used 3 times
- ...

## UI Functions
- say_line(text) - Used 50 times
- print_line(text, x, y, ...) - Used 10 times
- ...

## Drawing Functions
- map_draw(...) - Used 20 times
- set_trans_col(col, enable) - Used 15 times
- ...

## Game Logic Functions
- cutscene(flags, fn1, fn2) - Used 5 times
- pickup_obj(obj) - Used 10 times
- put_at(obj, x, y, room) - Used 20 times
- ...
```

## Acceptance Criteria

- [x] All function calls identified
- [x] Functions categorized by type
- [x] Usage counts recorded
- [x] File saved to plan directory
- [x] Tricky patterns documented

## Status: COMPLETED ✓

## Output

**Created**: [`function-catalog.md`](./function-catalog.md)

## Notes: Tricky Patterns Found

1. **Lambda syntax**: ESC uses `@(params) { body }` for anonymous functions - Python bindings need to support callable parameters

2. **Object method calls**: Pattern `me.verb()` and `room.scripts.script_name()` require property access on objects

3. **Void as null parameter**: `void` is used as a special null value (e.g., `cutscene(1, fn, void)`) - different from Python's `None`

4. **String-based state keys**: States use string keys like `"state_open"`, `"face_front"` - need consistent string handling

5. **Reference vs value**: Objects are passed by reference (`put_at(:actor, x, y, room)`), modifications persist

6. **PICO-8 legacy code**: C8_*.esc files contain mixed PICO-8 Lua syntax that was partially converted - some functions like `spr()`, `sspr()`, `sfx()` are PICO-8 specific

7. **Dialog loop pattern**: Complex dialog system uses `while(true)` loops with `dialog_set()`/`dialog_start()`/`selected_sentence` - needs careful Python translation

8. **Member property access**: Both `object.property` and `object["property"]` patterns appear (e.g., `me.state` and `me."state"`)

9. **Script background flag**: `start_script(fn, true)` continues on room change, `false` stops - important for lifecycle management

10. **Dual function forms**: Some functions have variants like `say_line(text)` and `say_line_actor(actor, text, wait, duration)` - need to decide on unified API

## Summary

- **Total unique functions**: 47
- **Files analyzed**: 6 ESC files (~6000 lines total)
- **Most used functions**: `say_line` (50+), `break_time` (40+), `put_at` (30+), `camera_follow` (15+)
- **Key categories**: Core (12), UI (9), Game Logic (8), Drawing (8), Audio (2), Object Methods (10+)

## Time Actual

~1.5 hours

