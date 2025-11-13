# Task Organization

This directory contains the project's development threads, split from the original monolithic TASKS.md file for better organization and maintainability.

---

## Thread Files

### Active Development

- **[Vfs.md](Vfs.md)** - VFS tree structure fixes and Eon test suite
  - Status: IN PROGRESS
  - Focus: Fixing VFS-AST and getting all Eon tests running
  - Current: Investigating Eon06 test 06a texture artifacts

- **[VfsShell.md](VfsShell.md)** - VFS shell overlay and ConsoleIde
  - Status: IN PROGRESS (mostly complete)
  - Components: VfsShell overlay, ConsoleIde integration
  - Provides: Developer tooling for VFS navigation and headless IDE

- **[stdsrc.md](stdsrc.md)** - U++ to STL wrapper implementation
  - Status: IN PROGRESS
  - Goal: Implement U++ Core/Draw/CtrlLib using STL
  - Enables: Cross-platform development and code portability

### Completed

- **[ShaderEditor.md](ShaderEditor.md)** - GraphLib node editor + ShaderToy
  - Status: COMPLETED (verification pending)
  - Features: Node-based shader editing, ShaderToy compatibility
  - Note: Core functionality complete, GUI needs verification

### Planned

- **[uppstd.md](uppstd.md)** - U++ ↔ STL mapping documentation
  - Status: TODO
  - Purpose: Document conversion mappings for code translation
  - Depends: stdsrc implementation

### Backlog

- **[TODO.md](TODO.md)** - Future enhancements and feature additions
  - Status: BACKLOG
  - Contains: API enhancements, new features, future work items

---

## Roadmaps

Detailed roadmaps for specific projects are in the `../roadmap/` directory:

- **[uxide.md](../roadmap/uxide.md)** - User Experience IDE roadmap
  - Comprehensive plan for UX-focused IDE development
  - Separate from core ai-upp development

---

## Dependencies

See **[THREAD_DEPENDENCIES.md](../THREAD_DEPENDENCIES.md)** for:
- Dependency graph between threads
- Critical path analysis
- Priority ordering
- Thread interaction notes

---

## Quick Reference

### Thread Status Summary

| Thread | Status | Priority | Blocks |
|--------|--------|----------|--------|
| Vfs | IN PROGRESS | HIGH | Multiple threads |
| VfsShell | IN PROGRESS | MEDIUM | None |
| ShaderEditor | COMPLETED | - | None |
| stdsrc | IN PROGRESS | MEDIUM | uppstd, code conversion |
| uppstd | TODO | LOW | Code conversion tools |
| TODO | BACKLOG | LOW | None |

### Critical Path

1. Vfs → Complete Eon test fixes
2. stdsrc → Continue wrapper implementation
3. uppstd → Document mappings
4. Code conversion tools

---

## Thread Naming Convention

Original TASKS.md threads have been renamed for clarity:

- ~~"vfs-ast-fix"~~ → **Vfs** (broader scope: all VFS fixes)
- ~~"GraphLib Node Editor Features"~~ → **ShaderEditor** (includes ShaderToy)
- ~~"ShaderToy"~~ → (merged into **ShaderEditor**)
- ~~"VfsShell Overlay Implementation"~~ → **VfsShell** (includes ConsoleIde)
- ~~"ConsoleIde"~~ → (merged into **VfsShell**)

New threads:
- **uppstd** - U++ to STL mapping (documentation for AI)
- **TODO** - General backlog items

---

## How to Use

1. **Find your thread**: Check the thread files above
2. **Check dependencies**: See THREAD_DEPENDENCIES.md
3. **Update status**: Mark tasks complete as you work
4. **Track progress**: Keep thread files up to date

---

## Notes

- Each thread file is self-contained with its own status and tasks
- Dependencies between threads are documented separately
- Roadmaps for major features go in `../roadmap/`
- Backlog items that don't fit a thread go in TODO.md
