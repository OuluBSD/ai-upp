# separate-window / Phase 3 / Task 02: .brc Embedded Export (Single-File Standalone)

## Status: DONE

## Goal
Add a "File → Export as standalone executable" menu item that produces a single
self-contained binary with the `.gamestate` project (scripts, assets, layouts) embedded
using U++'s `.brc` resource system.

## Background: .brc files in U++
U++'s `.brc` format embeds arbitrary files as named binary blobs compiled into the
executable. The `BINARY(name, "filename")` macro in a `.brc` file causes the build
system to compile the file's contents into the binary and expose it via
`GetBinaryResource("name")` at runtime.

Example (`Hearts.brc`):
```
BINARY(hearts_main_py, "main.py")
BINARY(hearts_table_form, "table.form")
BINARY(hearts_logic_py, "hearts/logic.py")
BINARY(hearts_game_gamestate, "game.gamestate")
```

At runtime: `String src = GetBinaryResource("hearts_main_py");`

## Export flow

### Step 1: User triggers export
Menu: `File → Export → Export as standalone executable...`
(This menu item lives in the `IDocumentHost::MainMenu()` hook of `CardGameDocumentHost`
or in the global File menu when a `.gamestate` tab is active.)

### Step 2: Export dialog
A small dialog asks:
- **Output binary name** (e.g., `Hearts`, `Solitaire`)
- **Output directory** (default: same as `.gamestate` file)
- **Include assets**: checkbox list of detected asset files (auto-discovered from
  `.gamestate` JSON's `entry_script` and the referenced `.form` + any `assets/` dir)
- **Platform**: (future) currently always "current platform"

### Step 3: Code generation
The export generates:
1. A temporary `.brc` file listing all project files
2. A `main.cpp` based on `GameLauncher/main.cpp` but with embedded resource loading:
   ```cpp
   // Instead of loading from disk:
   String game_json = GetBinaryResource("game_gamestate");
   // ... pass to CardGameDocumentHost as string, not file path
   ```
3. A temporary `.upp` file that `uses GameLauncher` and lists the generated `.brc`
4. Invokes `upp-build` (or the U++ build system) to compile the temporary package
5. Copies the output binary to the chosen output directory

### Runtime resource loading in GameLauncher
`CardGameDocumentHost` needs a `LoadFromString(const String& gamestate_json, const String& base_dir)` variant (or a virtual file system abstraction) so it can load from embedded resources:

```cpp
class IFileResolver {
public:
    virtual String ReadFile(const String& relative_path) = 0;
    virtual bool   Exists(const String& relative_path) = 0;
};

// Disk-based (normal IDE operation):
class DiskFileResolver : public IFileResolver { ... };

// Embedded resource-based (standalone binary):
class BrcFileResolver : public IFileResolver {
    String prefix; // e.g., "hearts_"
public:
    String ReadFile(const String& path) override {
        // map "main.py" → GetBinaryResource("hearts_main_py")
        String key = prefix + ToLower(path).Replace("/", "_").Replace(".", "_");
        return GetBinaryResource(key);
    }
};
```

`CardGameDocumentHost` receives an `IFileResolver` at construction; defaults to
`DiskFileResolver` when running inside the IDE.

## Scope and dependencies
This task depends on:
- Phase 3 task 01: `GameLauncher` package must exist
- Phase 2 task 01: `StandaloneGameWindow` / `CardGameDocumentHost` outside IDE
- `CardGameDocumentHost` must support `IFileResolver` (or similar) injection

## Deliverables
- `IFileResolver` interface + `DiskFileResolver` + `BrcFileResolver` implementations
- `CardGameDocumentHost` updated to use `IFileResolver` for all file I/O
- Export dialog UI in `ScriptIDE` (triggered from File menu when `.gamestate` active)
- Export code generator (temp `.brc` + temp `main.cpp` + build invocation)
- Documentation: what files are automatically included, how to add extras

## Implementation Notes (2026-03-26)
- Added `Game -> Export standalone executable...` action in `CardGameDocumentHost`.
- Added `File -> Export -> Standalone executable...` in `PythonIDE` when the active
  document is a `.gamestate` host (`CardGameDocumentHost`).
- Export now generates a standalone package in `<output>/<Name>_standalone_src` with:
  - `<Name>.brc` containing embedded project files
  - `Main.cpp` runtime that extracts embedded files to a temp folder and runs
    `RunStandaloneGameWindow(...)` against the extracted `.gamestate`
  - `<Name>.upp` using `ScriptIDE` runtime dependencies
- Export invokes `script/build.py` automatically (`-mc 1 -c -j8`) and places the
  built binary in the user-selected output location.
- A helper script `build_export.sh` and build log `build.log` are written into the
  generated source folder for rebuild/debug.

## Scope Notes
- This v1 keeps embedded-runtime loading simple by extracting resources to a temp
  directory before launch, instead of introducing a new in-memory file resolver.
- The generated binary is still self-contained for end users (no external project
  files are required at runtime).

## Acceptance Criteria
- Export `reference/Hearts/` → produces a `Hearts` binary.
- Running `./Hearts` opens the Hearts game without any external files.
- The binary includes `main.py`, `hearts/*.py`, `table.form`, and all `assets/`.
- Running `./Hearts` in a directory with no project files still works (resources embedded).

## Notes
- Build invocation from inside the IDE is non-trivial (needs the build system path,
  flags, etc.). For v1, a helper script (`export_standalone.sh`) generated alongside
  the sources is acceptable; the IDE runs it via `LocalProcess`.
- The `.brc` approach is U++-native and does not require external tools.
- Asset discovery: read `game.gamestate` JSON, find `entry_script`, scan its imports
  for local modules, scan `assets/` directory. All found files → `.brc` entries.
  Over-inclusion is better than under-inclusion.
