# Node Plan Tree

## What This Planning Tree Is
This directory is the implementation planning tree for migrating and redesigning `GraphLib` into the new `Node` package split:
- `./uppsrc/Node/Core/Core.upp` (headless core)
- `./uppsrc/Node/Ctrl/Ctrl.upp` (GUI/editor integration)

It is intentionally phase/task oriented so execution can proceed in small, reviewable work items without revisiting architecture boundaries each time.

## Organization
- Track directories: `./uppsrc/Node/plan/<track>/`
- Phase directories: `./uppsrc/Node/plan/<track>/<phase>/`
- Task files: `./uppsrc/Node/plan/<track>/<phase>/<task>.md`

Tracks and phase order come from `PLAN-SKELETON.md` and are expanded into concrete implementation tasks.

## Package-Boundary Rules
1. `Node/Core` owns document model, serialization, scene/render description, geometry, routing/path generation, hit testing, transforms, and command/undo logic.
2. `Node/Ctrl` owns `Ctrl` paint bridging, event routing, menus, focus, clipboard integration, viewport control, and child-`Ctrl` hosting.
3. Rendering logic stays in Core except final `Ctrl::Paint` playback.
4. Ctrl code must not directly own or reimplement model persistence, routing, hit testing semantics, or command-domain rules.
5. Boundary tasks freeze cross-package contracts and must be completed before heavy integration work.

## Execution Order
Recommended order:
1. Critical path tracks first:
   - `core-model-document`
   - `core-scene-render`
   - `core-editor-commands`
   - `ctrl-integration`
2. Then parallel/secondary tracks:
   - `core-layout-routing`
   - `widgets-inside-nodes`
   - `performance-scaling`
   - `migration-compat`

Within each track, execute phases in numeric order and tasks in numeric order unless a task explicitly allows parallelism.

## What Not To Do During Implementation
- Do not move domain logic from Core into Ctrl for convenience.
- Do not introduce `Ctrl` dependencies into `Node/Core`.
- Do not implement persistence or routing behavior in `Node/Ctrl`.
- Do not skip boundary-contract tasks and then improvise interfaces in code.
- Do not treat this as a one-shot rewrite; use phased migration and compatibility checks.
