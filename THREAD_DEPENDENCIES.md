# Thread Dependencies

This document describes the dependencies between different development threads in the ai-upp project.

---

## Thread Overview

### Active Threads (IN PROGRESS)
1. **VfsShell** - Shell with VFS overlay + ConsoleIde
2. **Vfs** - VFS tree fixes and Eon test fixes
3. **stdsrc** - U++ to STL wrapper implementation

### Planned Threads (TODO)
4. **uppstd** - U++ ↔ STL mapping documentation
5. **UxIde** - User experience IDE (see roadmap/uxide.md)

### Completed Threads
6. **ShaderEditor** - GraphLib node editor + ShaderToy integration

### Backlog
7. **TODO** - Collection of future enhancement tasks

---

## Dependency Graph

```
┌─────────┐
│   Vfs   │ (Core VFS implementation)
└────┬────┘
     │
     ├──────────────┬──────────────┬──────────────┐
     │              │              │              │
     v              v              v              v
┌─────────┐   ┌──────────┐   ┌─────────┐   ┌──────────┐
│VfsShell │   │ShaderEditor│  │ stdsrc  │   │All Eon   │
│         │   │  (files) │   │         │   │  Tests   │
└─────────┘   └──────────┘   └────┬────┘   └──────────┘
                                   │
                                   v
                              ┌─────────┐
                              │ uppstd  │ (mapping)
                              └────┬────┘
                                   │
                                   v
                              ┌─────────┐
                              │Code Conv│ (conversion tools)
                              └─────────┘
```

---

## Detailed Dependencies

### 1. Vfs Thread
**Status**: IN PROGRESS
**Description**: Core VFS tree structure and Eon test fixes

**Requires**:
- Nothing (foundational)

**Blocks**:
- VfsShell (needs working VFS)
- ShaderEditor (for shader file management)
- All Eon tests (requires correct VFS-AST)

**Current Focus**: Eon06 test 06a texture artifacts investigation

---

### 2. VfsShell Thread
**Status**: IN PROGRESS
**Description**: Shell with VFS overlay + ConsoleIde

**Requires**:
- Vfs core functionality

**Blocks**:
- Nothing (provides tooling, not core functionality)

**Components**:
- VfsShell overlay implementation (COMPLETED)
- ConsoleIde integration (COMPLETED)

**Notes**: VfsShell provides developer tools but doesn't block other threads

---

### 3. ShaderEditor Thread
**Status**: COMPLETED (some verification pending)
**Description**: GraphLib node editor + ShaderToy integration

**Requires**:
- GraphLib package
- OpenGL backend
- (Optional) Vfs for shader file management

**Blocks**:
- Nothing (standalone shader editing capability)

**Notes**:
- Core functionality complete
- ShaderToy GUI needs verification
- Can use file system directly, VFS integration optional

---

### 4. stdsrc Thread
**Status**: IN PROGRESS
**Description**: U++ to STL wrapper implementation

**Requires**:
- Standard C++ library (STL)
- Understanding of U++ APIs

**Blocks**:
- uppstd (provides implementation that uppstd documents)
- Code conversion tools (need working wrapper)

**Notes**:
- Core/Draw/CtrlLib wrappers at various completion stages
- Some mappings learned from stdsrc implementation

---

### 5. uppstd Thread
**Status**: TODO
**Description**: U++ ↔ STL mapping documentation

**Requires**:
- stdsrc implementation (to learn from)
- Knowledge of both U++ and STL APIs

**Blocks**:
- Automated code conversion tools

**Notes**:
- More documentation than code
- Will extract information from stdsrc
- Creates bi-directional conversion reference

---

### 6. UxIde Thread
**Status**: TODO (roadmap exists)
**Description**: User experience IDE

**Requires**:
- U++ framework (for instrumentation)
- Possibly VfsShell (for project management)

**Blocks**:
- Nothing (separate tooling project)

**Notes**:
- See roadmap/uxide.md for detailed plan
- Separate task/roadmap structure
- Not blocking core development

---

### 7. TODO Thread
**Status**: BACKLOG
**Description**: Collection of future enhancements

**Requires**:
- Various (depends on specific task)

**Blocks**:
- Nothing (backlog items)

**Notes**:
- Future work items
- Low priority enhancements
- API extensions and improvements

---

## Critical Path

The critical path for core functionality:

1. **Vfs** - Must complete Eon test fixes
   ↓
2. **VfsShell** - Already functional, polish ongoing
   ↓
3. **stdsrc** - Continue wrapper implementation
   ↓
4. **uppstd** - Document mappings
   ↓
5. **Code Conversion Tools** - Enable U++ ↔ STL conversion

**Parallel Work** (not on critical path):
- ShaderEditor - Standalone, mostly complete
- UxIde - Separate tooling project
- TODO items - Future enhancements

---

## Thread Interaction Notes

### VFS and All Consumers
- Vfs thread affects: VfsShell, ShaderEditor (optional), all Eon tests
- Changes to VFS core may require updates in consumer threads
- Eon tests validate VFS correctness

### stdsrc and uppstd
- stdsrc provides implementation
- uppstd documents the mappings
- Information flows both ways:
  - stdsrc → uppstd: Implementation details
  - uppstd → stdsrc: Gaps in mapping coverage

### Independent Threads
- ShaderEditor can work independently (file system fallback)
- UxIde is separate tooling project
- TODO items don't block anything

---

## Priority Order

1. **HIGH**: Vfs (blocks multiple threads)
2. **MEDIUM**: VfsShell (developer tooling)
3. **MEDIUM**: stdsrc (enables code portability)
4. **LOW**: uppstd (documentation)
5. **LOW**: UxIde (separate project)
6. **BACKLOG**: TODO items

---

## Notes

- ShaderEditor marked complete but needs GUI verification
- Eon06 test 06a under active investigation
- VfsShell and ConsoleIde phases mostly complete
- stdsrc Core complete, Draw/CtrlLib partial
- uppstd not yet started (needs stdsrc progress)
