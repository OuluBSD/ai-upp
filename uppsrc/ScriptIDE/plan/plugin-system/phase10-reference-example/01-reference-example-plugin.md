# Task: Reference Example Plugin Plan

## Goal
Design a concrete reference plugin that implements the custom card-game views, hooks into the Python VM, and executes the Hearts `.gamestate` project.

## Background / Rationale
The architecture needs validation. A reference plugin will prove that `.gamestate` files can be opened, `.form` files can be edited and loaded, and Python logic can drive a U++ UI layer successfully.

## Scope
- Defining the `CardGamePlugin` class implementing `IPlugin`.
- Defining the `CardGameView` (derived from `Ctrl` and `IDocumentHost`) for rendering `.form` at runtime.
- Defining the `CardGameEditor` for the `.form` FormEditor integration.
- Demonstrating VM-bound draw functions (e.g., `set_card(player, slot, card_id)`).

## Non-goals
- Releasing a polished, commercial-grade game engine.

## Dependencies
- All prior phases (Architecture, Formats, Editor Hosting, VM Bridge).

## Concrete Investigation Steps
1. Outline the `CardGamePlugin` factory and registration process.
2. Draft the implementation of `ICustomExecuteProvider` that intercepts `.gamestate` Run commands.
3. List the minimum set of C++ functions to expose via `IPythonBindingProvider` to make Hearts playable.

## Affected Subsystems
- Plugin System (New reference plugin package)

## Implementation Direction
Create a high-level design of the plugin's internal class structure. Define the exact file layout of the Hearts example project (e.g., `main.py`, `table.form`, `hearts.gamestate`, `assets/`).

## Risks
- Scope creep: making the reference plugin too generic instead of focusing on the immediate needs of the Hearts example.

## Acceptance Criteria
- [ ] Defined `CardGamePlugin` class architecture.
- [ ] Documented minimal C++ to Python API surface.
- [ ] Outlined structure for the Hearts `.gamestate` project folder.
