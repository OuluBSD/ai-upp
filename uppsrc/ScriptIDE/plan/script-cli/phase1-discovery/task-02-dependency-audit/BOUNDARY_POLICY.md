# Boundary Policy: No-CtrlCore / No-CtrlLib for ScriptCommon and ScriptCLI

## Goal
Define enforceable dependency boundaries so `ScriptCommon` and `ScriptCLI` remain headless and reusable by CLI/MCP/TUI workflows.

## Dependency Model
Default is **deny**. Only explicitly allowed package dependencies may be added.

### Allowed Packages
- For `ScriptCommon`:
  - `Core`
  - `ByteVM`
  - Other non-GUI core packages only when justified and reviewed
- For `ScriptCLI`:
  - `Core`
  - `ByteVM`
  - `ScriptCommon`
  - `MCP` (only if chosen as transport host dependency)

### Forbidden Packages
- `CtrlCore`
- `CtrlLib`
- `CodeEditor`
- `RichEdit`
- `Docking`
- `FormEditor`
- Any package that transitively forces GUI stack into ScriptCommon/ScriptCLI

## Header Policy
### Allowed include examples
- `#include <Core/Core.h>`
- `#include <ByteVM/ByteVM.h>`
- `#include <ScriptCommon/ScriptCommon.h>` (from ScriptCLI)

### Forbidden include examples
- `#include <CtrlCore/CtrlCore.h>`
- `#include <CtrlLib/CtrlLib.h>`
- `#include <Docking/Docking.h>`
- `#include "ScriptIDE.h"` from ScriptCommon/ScriptCLI

## Candidate Extraction Dependency Matrix

| File | Current GUI Include Risk | Decision |
|---|---|---|
| `IDESettings.h` | none | move directly to ScriptCommon |
| `RunManager.h/.cpp` | indirect via `ScriptIDE.h` include in `.cpp` | move + replace include with `ScriptCommon.h` |
| `Linter.h/.cpp` | indirect via `ScriptIDE.h` include in `.cpp` | move + replace include with `ScriptCommon.h` |
| `PathManager.h/.cpp` | indirect via `ScriptIDE.h` include in `.cpp` | move + replace include with `ScriptCommon.h` |
| `PluginInterfaces.h` | direct (`Ctrl`, `Bar`, `PythonIDE`) | split core vs GUI contracts |
| `PluginManager.*` | direct (`PythonIDE`, `DockableCtrl`) | split registry/runtime core vs GUI pane wiring |

## Verification Checklist
Run for each migration step:
1. No forbidden headers in destination package:
   - `rg -n "<CtrlCore/|<CtrlLib/|<Docking/|<CodeEditor/|<RichEdit/|\"ScriptIDE.h\"" uppsrc/ScriptCommon uppsrc/ScriptCLI`
2. No forbidden `uses` in package manifests:
   - `rg -n "CtrlCore|CtrlLib|CodeEditor|RichEdit|Docking|FormEditor" uppsrc/ScriptCommon/*.upp uppsrc/ScriptCLI/*.upp`
3. Build checks:
   - `script/build.py ScriptIDE`
   - `script/build.py ScriptCLI`
4. If violation found, block merge until split is done.

## Mixed-Class Split Strategy
1. Extract pure logic/state model to ScriptCommon service class.
2. Keep GUI control as adapter/wrapper in ScriptIDE.
3. Exchange via plain data models/events (Core types only).
4. For plugins, separate headless lifecycle and registration from pane/menu/document-host GUI hooks.

## Acceptance Check
- [x] Allowed/forbidden boundaries documented.
- [x] Verification commands documented.
- [x] Split strategy documented for mixed components.
