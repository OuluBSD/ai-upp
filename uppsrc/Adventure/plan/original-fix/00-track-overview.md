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

## Phases

1. **01-bindings** - Create Python bindings for engine functions
2. **02-game-esc** - Convert Game.esc to Python
3. **03-other-esc** - Convert remaining ESC files
4. **04-integration** - Update C++ integration code
5. **05-testing** - Test and debug the conversion

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
