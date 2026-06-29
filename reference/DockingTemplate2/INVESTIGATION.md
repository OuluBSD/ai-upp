# DockingTemplate2 Investigation

## Executive Summary

`DockingTemplate1` (and its direct copy `PKR/src/ThemeEditor`) already covers the
essential per-tab dock-layout persistence pattern well. What it lacks is:
per-tab `ToolBar`; main `MenuBar` integration (menu sections beyond File/View/Help);
a modal dock-layout management dialog driven by `DockConfigDlg`; a dedicated
`Windows` menu subtree for dock windows and editor modes; cursor-linking
infrastructure; edit-mode activation/deactivation; and a `Debug` log tab.

`../PKR/src/nide` was **not found** at the originally specified path but was located
at `E:\active\sblo\Dev\ConvNetCpp\src\nide` and inspected in full. `nide` is a
CLI/GUI routing shell for the NeuralIDE family of annotation and recognition tools;
it dispatches to `AnnotationEditorWindow` (from `ConvNetCpp/src/AnnotationEditor`)
and `RecognizerEditorWindow` (from `ConvNetCpp/src/RecognizerEditorBase`), both of
which are mature `DockWindow` subclasses. These provided the key patterns for
`InitDockers` / default-layout caching / workspace-state serialization / virtual
toolbar/menu extension hooks.

The proposed `DockingTemplate2` is achievable as a single self-contained U++ package
with no external dependencies beyond `Core`, `CtrlLib`, and `Docking`.

---

## Source Paths Inspected

| Path | Status |
|------|--------|
| `reference/DockingTemplate1/main.cpp` | Found, read in full |
| `reference/DockingTemplate1/DockingTemplate1.upp` | Found, read in full |
| `reference/DockingExample1/main.cpp` | Found, read in full |
| `reference/DockingExample2/main.cpp` | Found, read in full |
| `reference/DockingExample3/main.cpp` | Found, read in full |
| `../PKR/src/ThemeEditor/main.cpp` | Found, read in full (identical to DockingTemplate1) |
| `../PKR/src/nide` | **NOT FOUND** at that path |
| `ConvNetCpp/src/nide/main.cpp` | Found at `E:\active\sblo\Dev\ConvNetCpp\src\nide`, read in full |
| `ConvNetCpp/src/nide/ProjectManager.h` | Found, read in full |
| `ConvNetCpp/src/AnnotationEditor/AnnotationEditorWindow.h` | Found, read in full |
| `ConvNetCpp/src/AnnotationEditor/AnnotationEditorWindow.cpp` (layout section) | Found, read |
| `ConvNetCpp/src/RecognizerEditorBase/RecognizerEditorWindow.h` | Found, read in full |
| `ConvNetCpp/src/RecognizerEditorBase/RecognizerEditorLayout.cpp` | Found, read in full |
| `ConvNetCpp/src/RecognizerEditorBase/RecognizerEditorSerialization.cpp` (workspace state section) | Found, read |
| `ConvNetCpp/src/RecognizerEditorBase/RecognizerEditorWindow.cpp` (InitUI section) | Found, read |
| `uppsrc/Docking/Docking.h` | Found, read in full |
| `uppsrc/Docking/DockMenu.h` | Found, read in full |
| `uppsrc/CtrlLib/Bar.h` | Found, read in full |
| `uppsrc/CtrlLib/TabCtrl.h` | Found, read (first 80 lines) |

---

## Existing DockingTemplate1 Findings

### What it demonstrates well

1. **`DockWindow` inheritance** — the mandatory base class pattern; why the constructor
   is safe but `DockInit()` is required for all dock operations.
2. **`Dockable()` wrapping** — how to wrap a plain `Ctrl` into a `DockableCtrl` with a
   title and size hint in a single expression, retaining a pointer for later use.
3. **`Register()` before serialise** — registering all dockables before calling
   `SerializeWindow` so the system can match names across sessions.
4. **Per-tab layout save/load** — saving to a tab-specific `.dat` file on tab change and
   on close; falling back to a hard-coded default if no file exists.
5. **`SetDockVisible()` helper** — the correct sequence for hiding/showing a dockable
   (`DockWindow::Close()` vs `RestoreDockerPos()`) guards against calling these before
   the window is open.
6. **`DockWindowMenu(sub)`** — delegating the Windows submenu to the Docking library's
   built-in menu so the user gets dock/float/autohide/group management for free.
7. **`WhenSet` tab-change hook** — the guard (`!loaded`, `next == current_tab`) that
   avoids double-saves on startup.

### What it is missing

1. No `ToolBar` — there is no per-tab toolbar, no main toolbar, and no toolbar
   synchronisation with the menu.
2. No `App` or `Edit` menu sections — only File, Windows, View, Help.
3. No built-in modal dock-layout management (`DockConfigDlg` / `DockManager()`).
4. No `Windows` menu section for editor modes.
5. No cursor or selection linking between dock windows.
6. No edit-mode system — nothing activates or deactivates tab commands.
7. No `Debug` tab for logging UI state changes.
8. Default layout is hard-coded imperatively inside `ApplyDefaultDockSetForTab()`; the
   in-memory `StringStream` cache pattern (`CacheDefaultLayout` / `SetDefaultLayout`)
   used by `RecognizerEditorWindow` is missing. The `.brc`-embedded binary shown in
   `DockingExample2` is **not** the recommended approach for `DockingTemplate2`.
9. No `DockableCtrl`-derived classes or `WithLayout<DockableCtrl>` pattern (both shown in
   `DockingExample2`) — all dock views are bare `ParentCtrl`+`Label`.
10. Dock window visibility per tab is a switch statement that will need to be
    duplicated for every new tab; no data-driven registry.

### Patterns worth keeping verbatim

- `DockInit()` structure (register first, then serialise, then default).
- `SaveTabLayout` / `LoadTabLayout` / `LayoutFileForTab` trio.
- `SetDockVisible()` helper guarding `IsOpen()`.
- `WhenSet` + guard variables `loaded` and `current_tab`.
- `TopWindow::Close()` call chain in `Close()`.

---

## nide Findings

`nide` was found at `E:\active\sblo\Dev\ConvNetCpp\src\nide`. It is a CLI/GUI
routing shell (`main.cpp` + `ProjectManager`) that dispatches to specialized editor
windows in the surrounding `ConvNetCpp` project. The docking and layout management
patterns worth adopting are in `RecognizerEditorWindow` and `AnnotationEditorWindow`.

### `nide` architecture (routing layer)

`nide/main.cpp` routes on first CLI flag (`-A` annotate, `-R` recognizer, `-F` live
feed, `-N` recognizer-app stub, no flag → `ProjectManager`). It does not contain any
docking logic itself. `ProjectManager` is a `TopWindow` (not `DockWindow`) with a
`TabCtrl` and a `MenuBar`; it is a workflow hub, not an editor frame.

**Pattern to avoid:** building a central routing/launcher in `DockingTemplate2` — it
should be a single self-contained window.

### `RecognizerEditorWindow` — key patterns

#### `InitDockers()` — idempotent dock registration

```cpp
void RecognizerEditorWindow::InitDockers() {
    dock_sources_.Title("Sources").SizeHint(Size(200, 200));
    if(!source_list_.GetParent())
        dock_sources_.Add(source_list_.SizePos());
    Register(dock_sources_);
    // ... repeated for each dock panel
    RegisterExtraDocks(); // virtual hook for derived classes
}
```

The `!GetParent()` guard makes `InitDockers()` safe to call multiple times (e.g.,
on `OnResetDockLayout()`). Dock views are declared as `DockableCtrl` member fields
directly, then filled with content ctrls via `Add()`. This avoids the
`Dockable(ctrl, "Title")` wrapper call on every reset.

**Adopt for `DockingTemplate2`:** declare `DockableCtrl dock_a_`, `dock_b_`,
`dock_c_` as members; wire content in `InitDockers()`; keep the `!GetParent()` guard.

#### `OnResetDockLayout()` — close-all-then-apply default

```cpp
void RecognizerEditorWindow::OnResetDockLayout() {
    InitDockers();  // ensure titles/content are set
    for(DockableCtrl* dc : GetDockableCtrls())
        DockWindow::Close(*dc);  // purge all containers
    // Apply default positions
    DockLeft(dock_sources_);
    DockLeft(dock_images_);
    DockRight(dock_properties_);
    DockBottom(dock_nodes_);
    Tabify(dock_processed_summary_, dock_run_log_);
    ExtraDockLayout(); // virtual hook
}
```

**Critical insight:** closing all dockers before re-docking is required to avoid
double-registration artifacts when the layout is reset at runtime. `DockingTemplate1`
and `DockingExample2` never reset at runtime (they only apply the default once at
startup); `RecognizerEditorWindow` exposes this as a user-triggerable menu action.

**Adopt for `DockingTemplate2`:** implement `OnResetDockLayout()` as a named method
(not just inline in `DockInit`) so it can be wired to a "Reset Layout" menu item.

#### `CacheDefaultLayout()` / `SetDefaultLayout()` — in-memory string buffer

```cpp
void RecognizerEditorWindow::CacheDefaultLayout() {
    StringStream out;
    SerializeWindow(out);
    default_layout_data = out.GetResult();
}

void RecognizerEditorWindow::SetDefaultLayout() {
    if(default_layout_data.IsEmpty()) return;
    for(DockableCtrl* dc : GetDockableCtrls())
        DockWindow::Close(*dc);
    StringStream in(default_layout_data);
    SerializeWindow(in);
}
```

The default layout is cached as an in-memory `String` immediately after
`OnResetDockLayout()` is applied (in `DockInit()`). This is cleaner than a `.brc`
embedded binary and requires no build-time regeneration step. The tradeoff is that
the cached default reflects the initial window geometry at first launch rather than a
developer-chosen snapshot.

**Adopt for `DockingTemplate2`:** use `StringStream` + `default_layout_data` string
instead of the `.brc` approach. Drop the `.brc`/`DefaultLayout.dat` from the file
layout proposal.

#### `DockInit()` — canonical four-step sequence

```cpp
void RecognizerEditorWindow::DockInit() {
    InitDockers();          // 1. register dock panels (idempotent)
    OnResetDockLayout();    // 2. close all + apply default positions
    CacheDefaultLayout();   // 3. snapshot default as in-memory string
    LoadConfig();           // 4. load user's saved layout from disk
    loaded_ = true;
    // Add main area ctrl
    Add(main_stage_tabs_.SizePos());
    OnMainStageTabChanged();
}
```

This four-step sequence is the definitive correct order. Step 3 must come after
step 2 (so the cached default is the clean state, not a previous user layout). Step 4
loads over the default with the user's saved state.

**Adopt verbatim for `DockingTemplate2`.**

#### Separate workspace state (JSON) vs. dock layout (binary)

`RecognizerEditorWindow` separates two kinds of persistence:

- **Dock layout** — `SerializeWindow(stream)` written to a `.dat` file on `Close()`.
  Binary, not human-readable. Stores panel positions, sizes, tab memberships,
  autohide state, and window placement.
- **Workspace state** — `StoreAsJsonFile(ws, path)` via a `WorkspaceState` struct
  with `Jsonize()`. Stores which source/image/session/variant/tab was selected, and
  whether certain UI flags are set. JSON, human-readable, forward-compatible.

```cpp
struct WorkspaceState {
    String source_path, image_path, session_name, variant_id, node_id;
    int    main_tab = -1;
    bool   canvas_is_session = false;
    bool   show_all_rects = false;
    void Jsonize(JsonIO& jio) { jio("source_path", source_path)...; }
};
```

**Adopt for `DockingTemplate2`:** define a simple `AppState` struct with `Jsonize()`
storing: **active tab stable id/name** (primary key for restoration), **active tab
index** (fallback when the name is not found), current editor mode, and any boolean
UI flags. Keep this separate from the dock layout `.dat` file.

```cpp
struct AppState {
    String active_tab_name;   // primary: stable label, e.g. "Tab 1"
    int    active_tab_index = 0; // fallback if name not found
    int    editor_mode = 0;
    void Jsonize(JsonIO& jio) {
        jio("active_tab_name", active_tab_name)
           ("active_tab_index", active_tab_index)
           ("editor_mode", editor_mode);
    }
};
```

On restore: find the tab whose label matches `active_tab_name`; if not found, use
`active_tab_index` clamped to valid range.

#### Virtual toolbar and menu extension hooks

```cpp
// In RecognizerEditorWindow (base class):
virtual void ExtendToolbar(Bar& bar) {}
virtual void ExtendMenu(Bar& bar) {}
virtual void ExtendFileMenu(Bar& bar) {}
virtual void RegisterExtraDocks() {}
virtual void ExtraDockLayout() {}
```

Derived classes (`RecognizerAppWindow`) override these to add app-specific items
without touching the base class menu/toolbar setup. This is the correct pattern for
a template that expects to be subclassed.

**Adopt for `DockingTemplate2`:** add the same virtual hooks to `MainWindow` so that
applications can extend without forking the template.

#### `SetDockVisible()` — identical to `DockingTemplate1`

The implementation in `RecognizerEditorWindow::SetDockVisible()` is identical to
`DockingTemplate1`. This confirms the pattern is canonical and correct.

### `AnnotationEditorWindow` — supplementary patterns

#### `ApplyDefaultDockLayout()` uses `Dockable()` inline

`AnnotationEditorWindow` wraps content at dock time using the `Dockable(ctrl, title)`
pattern (inline wrapping). `RecognizerEditorWindow` uses pre-declared `DockableCtrl`
members. The `DockableCtrl` member approach is preferable because:
- The `!GetParent()` guard works correctly.
- The dock window can be reset at runtime without creating a new wrapper.
- The dock panel can be referenced by pointer after registration.

#### Separate `SerializePlacement` for window position

`AnnotationEditorWindow` saves window geometry (`SerializePlacement`) to a separate
file from the dock layout (`SerializeWindow`). This allows window position to be
restored independently of dock panel arrangement.

**Consider for `DockingTemplate2`:** `SerializeWindow` already includes placement
via `TopWindow::SerializePlacement` internally, so a separate placement file is
redundant unless the application needs to reset dock layout without resetting the
window position.

#### "Set Default layout" menu item

`AnnotationEditorWindow` exposes `SetDefaultLayout()` as a menu item so the user can
explicitly restore the developer's default without restarting. This is user-friendly.

**Adopt for `DockingTemplate2`:** add "Reset to Default Layout" in the `View` menu,
wired to `OnResetDockLayout()` + no reload of user's saved `.dat`.

### Patterns to avoid from `nide`

- **GT replay, round editor, timing panels** — all domain-specific; do not port.
- **`RecognizerAppWindow`'s per-session tab switching** — 9 tabs with complex
  state sync; too domain-specific.
- **`WorkspaceState` CEE variant list** — domain-specific; use a minimal struct.
- **`TimingScope`/`TimingContextInterval`** — profiling infrastructure; not needed.

---

## Proposed DockingTemplate2 Scope

`DockingTemplate2` must be a self-contained, generic starting point. It is **not**
application-specific. It should illustrate every scaffold layer a real GUI tool needs
without containing any domain logic.

### In scope (generic scaffolding)

- Main `TabCtrl` with three illustrative tabs (Tab1, Tab2, Debug).
- Each tab owns its own `ToolBar` populated via a virtual method.
- Main `MenuBar` with sections: `App`, `Edit`, `View`, `Windows`, `Help`.
- `View` menu mirrors dock-window show/hide and layout commands.
- `Windows` menu exposes dock windows, saved layouts, and editor modes.
- Three registered dock windows: `DockA`, `DockB`, `DockC`.
- Per-tab dock layout persistence (save/load/default) using binary files.
- Programmatic default dock layout defined in C++ (in-memory `StringStream` cache via `CacheDefaultLayout()`). No `.brc` binary resource is needed or recommended.
- User-saved layout is loaded first when it exists and is valid; the C++ default is the fallback.
- Modal layout management via `DockManager()` / `DockConfigDlg`.
- Two editor modes: `View` (read-only) and `Edit` (interactive).
- Cursor/position link: dock windows subscribe to a shared `WhenCursorChanged` event.
- `Debug` tab: a `TextCtrl` that logs every UI state change.
- `AppPrefs` stub dialog (Preferences menu item).

### Out of scope

- Domain-specific logic (video, poker, game detection, etc.).
- Real document content — tabs contain placeholder ctrls only.
- Additional GUI frameworks, custom rendering, or network.
- Any content from a future video/change-detection tool.

---

## Proposed Package and File Layout

```
reference/DockingTemplate2/
  DockingTemplate2.upp       -- package descriptor
  main.cpp                   -- GUI_APP_MAIN, instantiates MainWindow
  MainWindow.h               -- MainWindow class declaration
  MainWindow.cpp             -- constructor, DockInit, Close, InitDockers,
                                OnResetDockLayout, CacheDefaultLayout,
                                SetDefaultLayout, layout helpers
  MenuSetup.cpp              -- all Bar& callbacks: MainMenu, MenuApp, MenuEdit,
                                MenuView, MenuWindows, MenuHelp
  ToolBarSetup.cpp           -- UpdateToolBar(), per-tab toolbar builders
  EditMode.h                 -- EditMode enum, ModeManager class
  EditMode.cpp               -- ModeManager: activate/deactivate, cursor link
  DockViews.h                -- SampleDockView base; DockViewA/B/C declarations
  DockViews.cpp              -- dock view implementations, WhenCursorChanged wiring
  DebugTab.h                 -- DebugLog class (wraps TextCtrl, exposes Log(String))
  DebugTab.cpp               -- DebugLog implementation
  AppState.h                 -- AppState struct with Jsonize() for workspace state
```

No `.brc` or `.dat` source layout files are needed. The default layout is cached
in memory using `StringStream` (the `nide`/`RecognizerEditorWindow` pattern).

A single `.lay` file is not required because no dialog layout is complex enough to
need the layout editor at this stage.

---

## Main Window Architecture

```
class MainWindow : public DockWindow {
public:
    MainWindow();
    void DockInit() override;
    void Close() override;

private:
    // Frames
    MenuBar  menu;
    ToolBar  toolbar;        // single horizontal toolbar strip below the menu

    // Main area
    TabCtrl  main_tabs;

    // Tab content
    ParentCtrl  tab1_area;
    ParentCtrl  tab2_area;
    DebugLog    debug_tab;   // tab index 2, always present

    // Dock views (owned by DockingTemplate2; wired in DockInit)
    DockViewA   dock_a;
    DockViewB   dock_b;
    DockViewC   dock_c;

    // State
    ModeManager mode;        // current edit mode + event dispatch
    bool        loaded = false;
    int         current_tab = 0;

    // Helpers
    void UpdateToolBar();
    void OnMainTabChanged();
    // layout persistence
    String LayoutFileForTab(int tab) const;
    void SaveTabLayout(int tab);
    bool LoadTabLayout(int tab);
    void ApplyDefaultLayout(int tab);
    void SetDockVisible(DockableCtrl& dc, bool visible);
};
```

**Ownership rules:**
- `MainWindow` owns `ModeManager mode`. `ModeManager` holds an `Event<int>
  WhenModeChanged` and an `Event<Point> WhenCursorChanged`.
- Each `DockView` holds a `Callback1<Point>` slot; `MainWindow::DockInit()` wires
  `mode.WhenCursorChanged` to all dock views after registration.
- `debug_tab` (`DebugLog`) is wired to `mode.WhenModeChanged`,
  `main_tabs.WhenSet`, toolbar refheshed events, and dock layout events by posting a
  `String` via `DebugLog::Log()`.

**Frame order:**
```
AddFrame(menu);    // must come first
AddFrame(toolbar); // immediately below menu
Add(main_tabs.SizePos());
```
`menu.Set(THISBACK(MainMenu))` and `toolbar.Set(THISBACK(UpdateToolBar))` are called
in the constructor. `toolbar` is refreshed (`toolbar.Set(...)`) every time the active
tab changes.

---

## Main Area Tabs and Per-Tab Toolbars

### Tab registration

```cpp
main_tabs.Add(tab1_area.SizePos(), "Tab 1");
main_tabs.Add(tab2_area.SizePos(), "Tab 2");
main_tabs.Add(debug_tab.SizePos(), "Debug");
main_tabs.WhenSet = [=] { OnMainTabChanged(); };
```

### Per-tab toolbar

`toolbar` is a single `ToolBar` frame owned by `MainWindow`. Its content changes
with the active tab:

```cpp
void MainWindow::UpdateToolBar() {
    int tab = main_tabs.Get();
    if (tab == 0)      ToolBarTab1(toolbar);
    else if (tab == 1) ToolBarTab2(toolbar);
    else               ToolBarTabDebug(toolbar);
}
```

Each `ToolBarTabN(Bar& bar)` is a free function in `ToolBarSetup.cpp` following the
same `Bar&` convention as menu callbacks. This makes the toolbar automatically re-sync
whenever `toolbar.Set(...)` calls it, and allows `MenuBar` to mirror the same actions
via `Bar::Scan()`.

### Menu mirroring

To mirror toolbar actions into the `Edit` menu:

```cpp
void MainWindow::MenuEdit(Bar& bar) {
    int tab = main_tabs.Get();
    if (tab == 0) ToolBarTab1(bar);   // same function, different Bar type
    else if (tab == 1) ToolBarTab2(bar);
    bar.Separator();
    bar.Add("Preferences", THISBACK(ShowPreferences));
}
```

Because `ToolBar` and `MenuBar` both derive from `Bar`, tab-local toolbar functions
that take `Bar&` work identically in both contexts without duplication.

---

## MenuBar Integration

```
App
  New / Open / Save / Save As / Recent »
  ─────
  Preferences
  ─────
  Exit
Edit
  <mirrored tab toolbar commands>
  ─────
  Preferences
View
  Reset Layout For Current Tab
  ─────
  Manage Layouts…       → DockManager()
  Save Layout As…       → SaveLayout(name)
  Load Layout »         → LoadLayout(ix)
Windows
  <DockWindowMenu — built-in: show/hide/float/autohide per dock window>
  ─────
  Modes »
    View Mode
    Edit Mode
Help
  About
```

`DockWindowMenu(sub)` is the built-in call; it already handles the individual dock
window list. The `Modes` submenu is application-owned and calls
`mode.SetMode(ModeManager::VIEW)` / `mode.SetMode(ModeManager::EDIT)`.

The `View / Manage Layouts…` item calls `DockManager()` directly (opens
`DockConfigDlg` internally). `SaveLayout` and `LoadLayout` use the `DockWindow`
API directly. `GetLayoutName(ix)` / `LayoutCount()` enumerate saved layouts for the
submenu.

---

## Dock Window Registry and Lifecycle

### Registration (in `DockInit`)

```cpp
void MainWindow::DockInit() {
    Register(dock_a.Title("Dock A").SizeHint(Size(280, 220)));
    Register(dock_b.Title("Dock B").SizeHint(Size(280, 220)));
    Register(dock_c.Title("Dock C").SizeHint(Size(360, 220)));

    // Wire cursor link
    mode.WhenCursorChanged << [&](Point p) { dock_a.OnCursorChanged(p); };
    mode.WhenCursorChanged << [&](Point p) { dock_b.OnCursorChanged(p); };
    mode.WhenCursorChanged << [&](Point p) { dock_c.OnCursorChanged(p); };

    int tab = main_tabs.Get();
    if (!LoadTabLayout(tab))
        ApplyDefaultLayout(tab);
    loaded = true;
}
```

`DockViewA/B/C` are `DockableCtrl`-derived classes (not `DockableCtrl` wrapping a
`ParentCtrl`) so they can carry their own state and `OnCursorChanged` method.

### Late creation

If a dock view must be created after `DockInit` (e.g., dynamically spawned), use
`CreateDockable<T>(title)`, then call `Register()` manually. The template creates
and owns the ctrl internally; the caller receives a reference. Call `DockLeft()` /
`Float()` / `AutoHide()` immediately after.

### Show / hide

Use the same `SetDockVisible()` helper from `DockingTemplate1`:

```cpp
void MainWindow::SetDockVisible(DockableCtrl& dc, bool visible) {
    if (!IsOpen()) { dc.Show(visible); return; }
    if (visible)  { if (dc.IsHidden()) RestoreDockerPos(dc, false); }
    else          { if (!dc.IsHidden()) DockWindow::Close(dc); }
}
```

### Persistence

Serialize with `SerializeWindow(stream)`. This writes the dock window layout (all
positions, sizes, tab memberships, autohide state) plus the window's own placement
(size, maximise state). Separate per-tab files prevent one tab's layout from
overwriting another's.

---

## Persistence Layers

`DockingTemplate2` (and applications derived from it) must treat persistence as **six
distinct layers**. Mixing layers leads to corruption or loss of user data.

| Layer | What it stores | Format | When saved | Who owns it |
|-------|---------------|--------|------------|-------------|
| **Programmatic defaults** | Default dock positions hard-coded in `OnResetDockLayout()` | C++ code | n/a | Developer |
| **User UI state** | Active tab id/name, active editor mode, boolean UI flags | JSON via `AppState`/`AppRegistry` | On close; on change | `MainWindow` |
| **Dock-window layout blob** | Panel positions, sizes, tab memberships, autohide, window placement | Binary via `SerializeWindow` | On tab change, on close | `MainWindow` |
| **Internal/editor state** | Document cursors, selections, view-specific data | JSON or binary, per-view | Per-view on close | `DockViewN` |
| **Project/model state** | Application data (not GUI) | Domain-specific | Domain-specific | Application |
| **Cache/external blob** | Thumbnails, pre-computed data, etc. | Binary | Background | Application |

**Rule:** GUI code (dock layout blob, UI state) must never be the sole store for
meaningful application or editor state. If the dock layout `.dat` file is deleted, the
user should lose only their panel arrangement — not their work.

### AppRegistry — Core-level persistence utility

`AppRegistry` will be implemented in `uppsrc/Core` as a **Core-only** registry and
persistence utility. It must not depend on `CtrlLib`, `Docking`, `TopWindow`,
`TabCtrl`, or any GUI package. It is usable from headless tools, scripts, and tests.

- **Default vendor:** `AiUpp` (used for filesystem paths, registry keys, etc.).
  The visible display name may be `AI-UPP`.
- **Filesystem paths:** prefer `AiUpp` (no hyphen) for portability.
- **Responsibilities:** store key-value pairs, load/store JSON state files, provide
  `GetDataFile(filename)` equivalent, manage per-user config directories.
- `AppState` (defined in `AppState.h`) uses `AppRegistry` for its JSON persistence
  once `AppRegistry` is available. Until then, `GetDataFile` from `Core` is sufficient.

`AppRegistry` and `reference/AppRegistry` must be implemented **before**
`DockingTemplate2` persistence work (Tasks G–H below).

---

## Dock Layout Persistence

### Per-tab files

```cpp
String MainWindow::LayoutFileForTab(int tab) const {
    switch(tab) {
    case 1:  return "dockingtemplate2_tab2.dat";
    case 2:  return "dockingtemplate2_debug.dat";
    default: return "dockingtemplate2_tab1.dat";
    }
}

void MainWindow::SaveTabLayout(int tab) {
    if (!IsOpen()) return;
    FileOut out(GetDataFile(LayoutFileForTab(tab)));
    if (out.IsOpen()) SerializeWindow(out);
}

bool MainWindow::LoadTabLayout(int tab) {
    if (!IsOpen()) return false;
    FileIn in(GetDataFile(LayoutFileForTab(tab)));
    if (!in.IsOpen() || in.IsError()) return false;
    SerializeWindow(in);
    return !in.IsError();
}
```

### In-memory default layout caching (from `nide`)

Following `RecognizerEditorWindow`: apply the default positions in
`OnResetDockLayout()`, then immediately capture the result in memory:

```cpp
void MainWindow::DockInit() {
    InitDockers();
    OnResetDockLayout();    // apply default positions
    CacheDefaultLayout();   // snapshot to default_layout_data string
    LoadUserLayout();       // load user's saved .dat (may override above)
    loaded = true;
}

void MainWindow::CacheDefaultLayout() {
    StringStream out;
    SerializeWindow(out);
    default_layout_data = out.GetResult();
}

void MainWindow::OnResetDockLayout() {
    for(DockableCtrl* dc : GetDockableCtrls())
        DockWindow::Close(*dc);
    DockLeft(dock_a); DockRight(dock_b); DockBottom(dock_c);
    SetDockVisible(dock_a, true);
    SetDockVisible(dock_b, true);
    SetDockVisible(dock_c, true);
}
```

`LoadUserLayout()` loads the per-tab `.dat` file and falls back to
`CachedDefaultLayout()` if the file is missing or corrupt. No `.brc` file is needed.

**Drop `DefaultLayout.brc` and `DefaultLayout.dat` from the file layout proposal.**

### Modal layout management

```cpp
bar.Add("Reset to Default Layout", [=] { OnResetDockLayout(); });
bar.Add("Manage Layouts…", [=] { DockManager(); });
bar.Add("Save Layout As…", [=] { OnSaveDockLayoutAs(); });
bar.Add("Load Layout…", [=] { OnLoadDockLayout(); });
```

`DockManager()` opens `DockConfigDlg` (save/load/delete named layouts, group
management, per-window dock-allow flags, lock toggle). No custom dialog is needed.

`OnResetDockLayout()` is the nide pattern: close all dockers, re-apply the hard-coded
default positions. It does **not** restore `default_layout_data` (which is the
constructor-time snapshot); instead it re-applies the programmatic defaults so the
user always gets the developer's intent, not an old snapshot.

`OnSaveDockLayoutAs()` / `OnLoadDockLayout()` use `FileSel` + `SerializeWindow`,
exactly as in `RecognizerEditorWindow`.

For a custom "Save Layout As…" flow:

```cpp
String name;
if (EditText(name, "Save Layout", "Name:"))
    SaveLayout(name);
```

---

## Edit Modes

```cpp
// EditMode.h
class ModeManager {
public:
    enum Mode { MODE_VIEW, MODE_EDIT };

    Event<int>   WhenModeChanged;    // fires with new Mode value
    Event<Point> WhenCursorChanged;  // fires when any linked cursor moves

    void SetMode(Mode m);
    Mode GetMode() const { return current; }
    void FireCursor(Point p);        // called by any dock view

private:
    Mode current = MODE_VIEW;
};
```

```cpp
// EditMode.cpp
void ModeManager::SetMode(Mode m) {
    if (m == current) return;
    current = m;
    WhenModeChanged(current);
}

void ModeManager::FireCursor(Point p) {
    WhenCursorChanged(p);
}
```

`MainWindow` wires `WhenModeChanged` in the constructor:

```cpp
mode.WhenModeChanged << [=](int m) {
    toolbar.Set(THISBACK(UpdateToolBar));   // re-sync toolbar enabled states
    debug_tab.Log(Format("mode changed: %s", m == ModeManager::MODE_EDIT ? "Edit" : "View"));
};
```

Tab-local toolbar functions use `mode.GetMode()` to enable/disable items:

```cpp
void ToolBarTab1(Bar& bar, ModeManager& mode) {
    bar.Add(mode.GetMode() == ModeManager::MODE_EDIT, "Action A", CtrlImg::open(),
            [] { /* ... */ });
}
```

---

## Cursor Linking Model

Each `DockableCtrl`-derived view implements:

```cpp
class DockViewA : public DockableCtrl {
public:
    void OnCursorChanged(Point p);
    // ... content ctrl members
};
```

`MainWindow::DockInit()` wires `mode.WhenCursorChanged` to all views. Any view
that generates a cursor event calls `mode.FireCursor(p)`, which fans it out to all
registered views including itself. Views must guard against re-entrancy:

```cpp
void DockViewA::OnCursorChanged(Point p) {
    if (updating) return;
    updating = true;
    // update internal cursor/selection display
    updating = false;
}
```

The link is **active only in `MODE_EDIT`** — views check `mode.GetMode()` before
calling `mode.FireCursor()`. In `MODE_VIEW`, views still receive `OnCursorChanged`
(for read-only sync) but do not emit.

When edit mode is deactivated:

```cpp
mode.WhenModeChanged << [=](int m) {
    if (m == ModeManager::MODE_VIEW) {
        // detach emit: views stop calling FireCursor
    }
};
```

In practice, the guard is inside `DockViewN::OnMouseMove` / `OnCursorSet`. Dock views
**must not hold a hard compile-time dependency on `MainWindow`**. Prefer a callback
or adapter pattern:

```cpp
// Preferred: dock view holds a std::function or Event<> slot
class DockViewA : public DockableCtrl {
public:
    // Caller (MainWindow) wires this in DockInit:
    Event<Point> WhenCursorEmit;      // fires when view wants to emit
    bool emit_cursor = false;         // set by mode changes via MainWindow wiring

    void OnSomeCursorEvent(Point p) {
        if (!emit_cursor) return;
        WhenCursorEmit(p);
    }
};
```

`MainWindow::DockInit()` wires everything:

```cpp
// outbound: view emits → mode fan-out → all views receive
dock_a.WhenCursorEmit = [&](Point p) { mode.FireCursor(p); };
dock_b.WhenCursorEmit = [&](Point p) { mode.FireCursor(p); };

// mode controls emit flag on views
mode.WhenModeChanged << [&](int m) {
    dock_a.emit_cursor = (m == ModeManager::MODE_EDIT);
    dock_b.emit_cursor = (m == ModeManager::MODE_EDIT);
};
```

This keeps `DockViewA.h` free of `MainWindow.h` include — the view is usable in
isolation and testable without a full window.

---

## Debug Tab and Event Log

`DebugLog` wraps a read-only `TextCtrl` with a public `Log(String)` method:

```cpp
class DebugLog : public ParentCtrl {
public:
    DebugLog();
    void Log(const String& msg);   // prepends timestamp, appends line
    void Clear();

private:
    TextCtrl text;
};

void DebugLog::Log(const String& msg) {
    text.Set(text.Get() + Format("[%s] %s\n", Format("%02d:%02d:%02d",
        GetSysTime().hour, GetSysTime().minute, GetSysTime().second), msg));
    text.SetCursor(text.GetLength());
}
```

### What to log

| Event | Log message |
|-------|-------------|
| Tab changed | `"tab: N → M"` |
| Mode changed | `"mode: View/Edit"` |
| Cursor linked | `"cursor: (x, y) from DockViewN"` |
| Dock layout saved | `"layout saved: tab N → filename"` |
| Dock layout loaded | `"layout loaded: tab N ← filename"` (or `"default"`) |
| Default layout applied | `"layout default: tab N"` |
| Dock window shown | `"dock shown: DockA/B/C"` |
| Dock window hidden | `"dock hidden: DockA/B/C"` |
| Toolbar refreshed | `"toolbar: tab N refreshed"` |
| Menu bar resynced | `"menu: refreshed"` |
| Mode manager event | `"mode event: N"` |
| Preferences opened | `"prefs: opened"` |
| Layout saved by user | `"layout saved as: <name>"` |
| Layout loaded by user | `"layout loaded: <name>"` |

The `Debug` tab is always at index 2 and is never hidden. Log entries are always
written regardless of which tab is active.

---

## Build and Verification Plan

### Verifying DockingTemplate1 (existing)

No standard build command was found in the repository. There is no `Makefile`,
`CMakeLists.txt`, or shell script at the project root targeting U++ packages. The
U++ build system uses `upp` / `uide` / `umk` CLI tools.

Expected command (for Linux/WSL with U++ installed; not verified in this workspace):

```sh
umk reference/DockingTemplate1 DockingTemplate1 -o /tmp/dt1_build
```

On Windows the equivalent is the UPP IDE (`TheIDE`) or `umk.exe`.

No U++ build tools were invoked in this investigation (read-only inspection).

### Verifying DockingTemplate2 (future)

After implementation:

```sh
# Build
umk reference/DockingTemplate2 DockingTemplate2 -o /tmp/dt2_build

# Smoke test: launch, exercise each tab, verify dock layout save/load survives restart
# (manual, no automated test harness available in U++ reference apps)
```

Source inspection commands used during this investigation:

```sh
rg --files reference/DockingTemplate1
rg --files reference/DockingExample1 reference/DockingExample2 reference/DockingExample3
rg "Dock|Docking|Window|MenuBar|ToolBar|TabCtrl|Serialize|Load|Store|Mode|Cursor" \
   reference/DockingTemplate1
```

---

## Risks and Open Questions

1. **`ToolBar` per-tab refresh jank** — calling `toolbar.Set(THISBACK(UpdateToolBar))`
   on every tab change redraws the entire toolbar; if items have state (toggles),
   the state must be recalculated inside `UpdateToolBarTabN()` each call.
   *Mitigation*: keep toolbar callback functions stateless; compute enabled/checked
   from application state, not from saved UI state.

2. **`SerializeWindow` and dock position mismatch** — when new dock windows are added
   to a future application that inherits from this template, old `.dat` files will
   silently ignore unknown window names. This is safe (graceful degradation) but new
   windows will be hidden until the user explicitly shows them.
   *Mitigation*: document the behaviour; apply `ApplyDefaultLayout` if the loaded
   layout has fewer visible docks than expected.

3. **Cursor re-entrancy** — if `WhenCursorChanged` is fired synchronously and a
   handler modifies state that triggers another cursor event, infinite loops are
   possible. The `bool updating` guard in each view is necessary and must not be
   forgotten.

4. **Default layout reflects first-launch window size** — `CacheDefaultLayout()` is
   called during `DockInit()`, so the cached default reflects whatever window geometry
   existed when the app first opened. On a very small screen the default layout may
   look different than on a developer's machine. *Mitigation*: call
   `OnResetDockLayout()` + `CacheDefaultLayout()` after the window has been given a
   reasonable minimum size in the constructor.

5. **`AppRegistry` is a pre-requisite** — `AppState` JSON persistence ideally uses
   `AppRegistry` once it is available. Until then, `GetDataFile("...state.json")` from
   `Core` is an acceptable interim. Do not implement the full persistence layer before
   `AppRegistry` exists.

6. **Old desktop/video label names** — labels such as `desktop-parser`,
   `desktop-website`, and `desktop-diary` appear in earlier notes and planning
   documents. These are **historical working names only**. They are not final package
   names, not final architecture decisions, and must not appear in `DockingTemplate2`
   code, comments, or documentation. All references to these labels in planning
   documents should be treated as provisional.

7. **`DockConfigDlg` is internal** — `DockManager()` opens `DockConfigDlg` via the
   library; the dialog is not easily customisable. If `DockingTemplate2` needs a
   custom layout dialog later, it must replicate the `BackupLayout()` /
   `RestoreLayout()` API calls manually.

---

## Recommended Implementation Tasks

The following tasks are self-contained and can each be copied to a separate Codex
task file.

---

### Task A: Create `DockingTemplate2` package skeleton

Create `reference/DockingTemplate2/DockingTemplate2.upp` and all source files listed
in the Package and File Layout section as empty stubs. The `.upp` must declare
`uses Core, CtrlLib, Docking` and list all `.cpp` files. Verify the package
descriptor syntax matches `DockingTemplate1.upp`.

---

### Task B: Implement `MainWindow` with `TabCtrl`, per-tab `ToolBar`, and nide layout pattern

Implement `MainWindow` in `MainWindow.h/.cpp` with:
- `MenuBar menu` + `ToolBar toolbar` frames (added in constructor in that order).
- `TabCtrl main_tabs` with Tab1, Tab2, Debug as tabs.
- `DebugLog debug_tab` at tab index 2.
- `UpdateToolBar()` switching on `main_tabs.Get()`, calling per-tab `ToolBarTabN(bar)`
  free functions in `ToolBarSetup.cpp`.
- `WhenSet` wired to `OnMainTabChanged()`.
- `DockableCtrl dock_a_, dock_b_, dock_c_` as member fields (not `Dockable()` wrappers).
- `InitDockers()` with `!GetParent()` guard, wiring content and calling `Register()`.
- `OnResetDockLayout()` — close all, then `DockLeft(dock_a_)` etc.
- `CacheDefaultLayout()` / `SetDefaultLayout()` using `StringStream`.
- `DockInit()` in the four-step nide order: `InitDockers` → `OnResetDockLayout` →
  `CacheDefaultLayout` → `LoadUserLayout`.
- Per-tab `.dat` file persistence via `SaveTabLayout` / `LoadTabLayout`.

---

### Task C: Implement full `MenuBar` setup

Implement `MenuSetup.cpp` with all five menu sections (`App`, `Edit`, `View`,
`Windows`, `Help`) as described in the MenuBar Integration section. `Edit` must call
the same per-tab `ToolBarTabN` functions so toolbar and menu stay in sync. `View`
must include layout save/load and `DockManager()`. `Windows` must include
`DockWindowMenu` and the `Modes` submenu.

---

### Task D: Implement `ModeManager` and edit-mode activation

Implement `EditMode.h/.cpp` with `ModeManager` as described. Wire `WhenModeChanged`
in `MainWindow` to refresh the toolbar and log to `DebugLog`. Add `View Mode` and
`Edit Mode` items to the `Windows / Modes` submenu.

---

### Task E: Implement dock views with cursor linking

Implement `DockViewA/B/C` as `DockableCtrl`-derived classes in `DockViews.h/.cpp`.
Each must show a placeholder `Label` and implement `OnCursorChanged(Point)` with
re-entrancy guard. Wire `mode.WhenCursorChanged` to all three views in `DockInit`.
Each view must call `mode.FireCursor(p)` only when `mode.GetMode() == MODE_EDIT`.

---

### Task F: Implement `DebugLog` tab

Implement `DebugLog` in `DebugTab.h/.cpp` as described. Wire it to: mode changes,
tab changes, toolbar refresh, dock layout events (save/load/default/show/hide).
Verify that log lines appear in real time as the user interacts with the UI.

---

### Task G: Implement `AppRegistry` (Core-level utility)

Implement `AppRegistry` in `uppsrc/Core` as a Core-only registry and persistence
helper. Requirements:
- No dependency on `CtrlLib`, `Docking`, `TopWindow`, `TabCtrl`, or any GUI package.
- Default vendor name `AiUpp` (filesystem/registry paths); display name may be `AI-UPP`.
- Provide: `GetAppDataDir(vendor, app)`, `GetConfigFile(name)`, `StoreJson(key, val)`,
  `LoadJson(key, val)`, and a thin wrapper around `StoreAsJsonFile`/`LoadFromJsonFile`.
- Add `uppsrc/Core/AppRegistry.h` and `uppsrc/Core/AppRegistry.cpp` to the `Core`
  package descriptor.

This task must be completed before Task I (AppState persistence in DockingTemplate2).

---

### Task H: Create `reference/AppRegistry` example

Create a minimal `reference/AppRegistry` package that demonstrates `AppRegistry`
usage: store and restore a simple struct via JSON, read the per-user config directory,
and print the resolved config path. This serves as documentation-by-example.

---

### Task I: Add `AppState` workspace-state persistence

Define `AppState` in `AppState.h` with `Jsonize()` storing: active tab stable
id/name (primary), active tab index (fallback), current editor mode, and any boolean
UI flags. In `MainWindow::Close()`, serialize to `GetDataFile("dockingtemplate2_state.json")`
via `StoreAsJsonFile` (or `AppRegistry::StoreJson` once Task G is done). In `DockInit()`
after `LoadUserLayout`, load with `LoadFromJsonFile` and restore active tab by name
first, then by index fallback, then by mode.

No `.brc` or embedded binary layout is needed; the `StringStream` default-layout
cache (Task B) replaces that mechanism entirely.

---

### Task J: Write package `README`

Document (in `reference/DockingTemplate2/README.md`) how to build the package, how
to add new dock windows (update `InitDockers()` and `OnResetDockLayout()`), and the
layout file naming convention (`dockingtemplate2_tabN.dat`,
`dockingtemplate2_state.json`). List the U++ build command. This task is
documentation-only.
