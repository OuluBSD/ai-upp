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

- [ ] All function calls identified
- [ ] Functions categorized by type
- [ ] Usage counts recorded
- [ ] File saved to plan directory

## Time Estimate

2-4 hours
