# Core Extraction Plan (RunManager/Linter/PathManager/IDESettings)

## Goal
Execute a low-risk, checkpointed migration of first non-GUI files from ScriptIDE to ScriptCommon.

## Move Set
- `IDESettings.h`
- `PathManager.h/.cpp`
- `Linter.h/.cpp`
- `RunManager.h/.cpp`

## Ordered Steps

### Step 1: Introduce ScriptCommon package skeleton
- Create `uppsrc/ScriptCommon/ScriptCommon.upp` with non-GUI `uses` only.
- Create `uppsrc/ScriptCommon/ScriptCommon.h` umbrella header.
- Add placeholder file entries for move set.

Build checkpoint:
- `script/build.py ScriptIDE`
- `script/build.py ScriptCLI` (may fail until package exists; expected once added)

### Step 2: Move IDESettings first
- Copy `ScriptIDE/IDESettings.h` -> `ScriptCommon/IDESettings.h`.
- Update includes in ScriptIDE files from local include to `<ScriptCommon/ScriptCommon.h>`.
- Keep serialization compatibility unchanged.

Build checkpoint:
- `script/build.py ScriptIDE`

### Step 3: Move PathManager
- Move `PathManager.h/.cpp` to ScriptCommon.
- Replace `.cpp` include from `"ScriptIDE.h"` to `"ScriptCommon.h"`.
- Ensure no GUI header leaks.

Build checkpoint:
- `script/build.py ScriptIDE`

### Step 4: Move Linter and RunManager
- Move `Linter.h/.cpp`, `RunManager.h/.cpp`.
- Replace `.cpp` includes from `"ScriptIDE.h"` to `"ScriptCommon.h"`.
- Update ScriptIDE call sites includes.

Build checkpoint:
- `script/build.py ScriptIDE`

### Step 5: Update package manifests
- Remove moved files from `uppsrc/ScriptIDE/ScriptIDE.upp`.
- Add moved files to `uppsrc/ScriptCommon/ScriptCommon.upp`.
- Ensure ScriptIDE `uses` includes `ScriptCommon`.

Build checkpoint:
- `script/build.py ScriptIDE`
- `script/build.py ScriptCLI`

## Dependent Files to Touch in ScriptIDE
- `ScriptIDE.h` (include adjustments)
- `PythonIDE.h/.cpp` (indirect include impacts)
- Any files including moved headers directly

## Safety/Validation Commands
- Forbidden include scan:
  - `rg -n "<CtrlCore/|<CtrlLib/|<Docking/|\"ScriptIDE.h\"" uppsrc/ScriptCommon`
- Verify package dependencies:
  - `rg -n "CtrlCore|CtrlLib|Docking|CodeEditor|RichEdit|FormEditor" uppsrc/ScriptCommon/ScriptCommon.upp`

## Rollback Strategy
- Keep moves in small commits per step (or equivalent patch chunks).
- If build breaks after a step, revert only the last moved unit and re-run checkpoint.

## Acceptance
- [x] Ordered migration sequence defined.
- [x] Per-step include/path updates defined.
- [x] Build checkpoints and validation commands defined.
