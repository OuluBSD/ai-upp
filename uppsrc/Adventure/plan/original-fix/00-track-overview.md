# Track: Original Fix - Convert Adventure from ESC to Python

## Overview

Convert the Adventure package from custom ESC scripting language to Python using the ByteVM backend.

**Goal**: Get the adventure game working with Python scripts instead of ESC, using the authoritative scumm-8 source as reference.

**Authoritative Source**: `/tmp/scumm-8/` (original pico-8 → C++ → ESC conversion by Seppo Pakonen)

## Current State

- **Package**: `uppsrc/Adventure/`
- **Engine**: C++ with ESC VM integration
- **Scripts**: 7 `.esc` files (~5000+ lines total)
- **Status**: ESC version has never worked

## Target State

- **Engine**: C++ with ByteVM Python backend
- **Scripts**: 7 `.py` files (converted from ESC)
- **Status**: Working adventure game with Python scripting

## Progress

**Phase 01: Python Bindings** - ✓ COMPLETE (7/7 tasks, 52 functions)
**Phase 1.5: PyVM Integration** - ✓ COMPLETE (Engine converted to PyVM)
**Phase 02: Convert Game.esc** - ✓ COMPLETE (Game.py, 1569 lines)
**Phase 03: Convert Other ESC** - ✓ COMPLETE (5 files, 2360 lines)
**Phase 04: C++ Integration** - ✓ COMPLETE (done in Phase 1.5)
**Phase 05: Testing** - ⏳ PENDING

## Summary

### Completed

| Phase | Status | Output |
|-------|--------|--------|
| 01 - Bindings | ✓ | 52 Python functions |
| 1.5 - PyVM Integration | ✓ | Engine uses PyVM |
| 02 - Game.esc | ✓ | Game.py (1569 lines) |
| 03 - Other ESC | ✓ | 5 Python files (2360 lines) |
| 04 - Integration | ✓ | Done in Phase 1.5 |

### Total Python Code

- **6 Python files**: Game.py, Demo.py, C8_Intro.py, C8_Part1.py, C8_Part2.py, CarverTest.py
- **~3930 lines** of Python game logic
- **52 Python bindings** for engine functions
- **Build**: ✓ Successful (bin/Adventure, 56 MB)

**Build configurations**:
```
[0] Console = GUI
[1] X11 = GUI X11
[2] X11 (Valgrind) = GUI X11 USEMALLOC

Usage:
  script/build.py -mc 1 Adventure    # Build with X11
  script/build.py -mc 2 Adventure    # Build for Valgrind
```

### Remaining

- **Phase 05**: Testing and debugging
- Runtime crash fix (pre-existing graphics issue, unrelated to Python)

## Files to Convert

| ESC File | Lines | Description |
|----------|-------|-------------|
| Game.esc | ~1563 | Main game logic, rooms, objects, actors |
| CarverTest.esc | ? | Test/demo content |
| Demo.esc | ? | Demo content |
| C8_Intro.esc | ? | Chapter 8 intro |
| C8_Part1.esc | ? | Chapter 8 part 1 |
| C8_Part2.esc | ? | Chapter 8 part 2 |

## Key Conversions

| ESC Syntax | Python Equivalent |
|------------|-------------------|
| `:name` | `name` (global variable) |
| `@(args) { body }` | `lambda args: body` or `def func(args): body` |
| `fn() { body }` | `def fn(): body` |
| `true/false` | `True/False` |
| `nil` | `None` |
| `// comment` | `# comment` |
| `if (cond) { }` | `if cond:` |
| `me.field` | `self.field` (in methods) |

## Dependencies

- `uppsrc/ByteVM/` - Python VM implementation
- `uppsrc/EscAnim/` - Animation system (used by Adventure)
- `/tmp/scumm-8/` - Authoritative source reference

## Notes

- The ESC version has never worked - this is a fresh start with Python
- Keep the C++ engine structure, only replace scripting layer
- Test incrementally - convert one file at a time
- Reference the original scumm-8 pico-8 source for game logic verification
