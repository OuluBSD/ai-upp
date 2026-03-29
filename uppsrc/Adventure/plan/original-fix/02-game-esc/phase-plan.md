# Phase 02: Convert Game.esc to Python

## Overview

Convert the main game logic file `Game.esc` to Python syntax.

## Scope

- Convert `Game.esc` (~1563 lines) to `Game.py`
- Maintain exact game logic and behavior
- Use Python idioms where appropriate

## Tasks

1. [ ] **001-convert-header-comments** - Convert file header and comments
2. [ ] **002-convert-reset-functions** - Convert reset() and reset_ui()
3. [ ] **003-convert-startup-script** - Convert startup_script()
4. [ ] **004-convert-main-objects** - Convert object definitions
5. [ ] **005-convert-room-definitions** - Convert room definitions
6. [ ] **006-convert-verb-functions** - Convert verb handlers
7. [ ] **007-convert-cutscene-code** - Convert cutscene lambdas
8. [ ] **008-verify-conversion** - Compare with original ESC for accuracy

## Syntax Conversions

| ESC | Python |
|-----|--------|
| `:name` | `name` (global) |
| `@(me) { ... }` | `lambda me: ...` or `def func(me): ...` |
| `me.field = value` | `me.field = value` |
| `if (cond) { }` | `if cond:` |
| `else { }` | `else:` |
| `true/false` | `True/False` |
| `nil` | `None` |

## Expected Output

- `Game.py` - Converted Python file
- Conversion notes documenting any tricky conversions

## Acceptance Criteria

- [ ] All ESC syntax converted to valid Python
- [ ] Game logic preserved exactly
- [ ] Python file compiles without syntax errors
- [ ] All lambdas and callbacks properly converted

## Dependencies

- Phase 01 (bindings) must be complete first
- Need working Python VM integration

## Time Estimate

4-8 hours
