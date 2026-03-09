# Exact Menu Bar Specification (Captured UI Surface)

This document records the **exact menu-bar structure observed from the
screenshots**, including separators, shortcuts, submenu nesting and a
short implementation-oriented explanation for each action.

This is a **captured UI specification**, not a guess.\
Where something was not visible in the screenshots, it is marked as
**not captured**.

------------------------------------------------------------------------

# Top-Level Menu Bar

Observed top-level menus:

1.  File
2.  Edit
3.  Search
4.  Source
5.  Run
6.  Debug
7.  Consoles
8.  Projects
9.  Tools
10. Window
11. Help

Recommended U++ skeleton:

``` cpp
void IDEWindow::MainMenu(Bar& bar)
{
    bar.Sub("File",      [=](Bar& b){ FileMenu(b); });
    bar.Sub("Edit",      [=](Bar& b){ EditMenu(b); });
    bar.Sub("Search",    [=](Bar& b){ SearchMenu(b); });
    bar.Sub("Source",    [=](Bar& b){ SourceMenu(b); });
    bar.Sub("Run",       [=](Bar& b){ RunMenu(b); });
    bar.Sub("Debug",     [=](Bar& b){ DebugMenu(b); });
    bar.Sub("Consoles",  [=](Bar& b){ ConsolesMenu(b); });
    bar.Sub("Projects",  [=](Bar& b){ ProjectsMenu(b); });
    bar.Sub("Tools",     [=](Bar& b){ ToolsMenu(b); });
    bar.Sub("Window",    [=](Bar& b){ WindowMenu(b); });
    bar.Sub("Help",      [=](Bar& b){ HelpMenu(b); });
}
```

------------------------------------------------------------------------

# File Menu

``` text
File
├─ New file...                                  Ctrl+N
├─ [separator]
├─ Open...                                      Ctrl+O
├─ Open last closed                             Ctrl+Shift+T
├─ Open recent >
│  ├─ <recent file path item(s)>
│  ├─ [separator]
│  ├─ Maximum number of recent files...
│  └─ Clear this list
├─ [separator]
├─ Save                                         Ctrl+S
├─ Save all                                     Ctrl+Alt+S
├─ Save as                                      Ctrl+Shift+S
├─ Save copy as...
├─ Revert
├─ [separator]
├─ Print preview
├─ Print...
├─ [separator]
├─ Close
├─ Close all                                    Ctrl+Shift+W
├─ [separator]
├─ File switcher...                             Ctrl+P
├─ Symbol finder...                             Ctrl+Alt+P
├─ [separator]
├─ Restart                                      Alt+Shift+R
├─ Restart in debug mode
└─ Quit                                         Ctrl+Q
```

## File Menu Explanations

-   **New file...** Create a new empty editor document.
-   **Open...** Open a file from disk.
-   **Open last closed** Re-open the last recently closed editor tab.
-   **Open recent \>** Recently opened file list, plus maintenance
    actions for the recent-file registry.
-   **Save / Save all / Save as / Save copy as** Standard editor
    persistence actions.
-   **Revert** Reload file contents from disk, discarding unsaved
    changes.
-   **Print preview / Print...** Editor-document printing.
-   **Close / Close all** Close one editor document or all of them.
-   **File switcher...** Open the quick file-search popup.
-   **Symbol finder...** Open current-file symbol popup.
-   **Restart** Full application restart.
-   **Restart in debug mode** Full application restart with debug
    instrumentation enabled.
-   **Quit** Exit the application.

------------------------------------------------------------------------

# Edit Menu

``` text
Edit
├─ Undo                                         Ctrl+Z
├─ Redo                                         Ctrl+Shift+Z
├─ [separator]
├─ Cut                                          Ctrl+X
├─ Copy                                         Ctrl+C
├─ Paste                                        Ctrl+V
├─ Select All                                   Ctrl+A
├─ [separator]
├─ Comment/uncomment                            Ctrl+1
├─ Add block comment                            Ctrl+4
├─ Remove block comment                         Ctrl+5
├─ [separator]
├─ Indent
├─ Unindent
├─ [separator]
├─ Toggle UPPERCASE                             Alt+Shift+U
├─ Toggle lowercase                             Alt+U
├─ [separator]
├─ Convert end-of-line characters >
│  ├─ LF (Linux/macOS)
│  ├─ CRLF (Windows)
│  └─ CR (legacy Mac)
├─ Remove trailing spaces
└─ Convert tabs to spaces
```

## Edit Menu Explanations

-   **Undo / Redo** Standard editor history operations.
-   **Cut / Copy / Paste / Select All** Standard clipboard commands.
-   **Comment/uncomment** Toggle line comments for selected lines.
-   **Add block comment / Remove block comment** Insert or remove
    block-style comments.
-   **Indent / Unindent** Shift selected block indentation.
-   **Toggle UPPERCASE / Toggle lowercase** Change selected text casing.
-   **Convert end-of-line characters \>** Normalize line endings in the
    current document.
-   **Remove trailing spaces** Strip trailing whitespace.
-   **Convert tabs to spaces** Normalize indentation characters.

------------------------------------------------------------------------

# Search Menu

``` text
Search
├─ Find text
├─ Find next
├─ Find previous
├─ Replace text
├─ [separator]
├─ Last edit location                           Ctrl+Alt+Shift+Left
├─ Previous cursor position                     Alt+Left
├─ Next cursor position                         Alt+Right
├─ [separator]
└─ Search text in files...                      Alt+Shift+F
```

## Search Menu Explanations

-   **Find text / Find next / Find previous / Replace text**
    Current-editor incremental search actions.
-   **Last edit location** Jump to the most recent edit location.
-   **Previous cursor position / Next cursor position** Navigation
    history stack.
-   **Search text in files...** Open the docked Find pane for
    project/directory search.

------------------------------------------------------------------------

# Source Menu

``` text
Source
├─ Show invisible characters                    [checkbox]
├─ Wrap lines                                   [checkbox]
├─ Show indent guides                           [checkbox]
├─ Show code folding                            [checkbox]
├─ Show class/function selector                 [checkbox]
├─ Show docstring style warnings                [checkbox]
├─ Underline errors and warnings                [checkbox]
├─ [separator]
├─ Show todo list >                             [disabled / not expanded]
├─ Show warning/error list >                    [disabled / not expanded]
├─ Previous warning/error                       Ctrl+Alt+Shift+,
├─ Next warning/error                           Ctrl+Alt+Shift+.
├─ [separator]
├─ Run code analysis                            F8
└─ Format file or selection with Autopep8
```

## Source Menu Explanations

-   **Show invisible characters** Toggle rendering of
    spaces/tabs/newlines.
-   **Wrap lines** Soft wrap editor lines visually.
-   **Show indent guides** Draw indentation guide lines.
-   **Show code folding** Enable fold markers and folding UI.
-   **Show class/function selector** Show current symbol selector in the
    editor UI.
-   **Show docstring style warnings** Highlight docstring-style issues.
-   **Underline errors and warnings** Real-time lint/analysis underline
    rendering.
-   **Show todo list / Show warning-error list** Diagnostic views;
    submenu contents not captured.
-   **Previous warning/error / Next warning/error** Jump among
    diagnostics.
-   **Run code analysis** Perform static analysis on current file.
-   **Format file or selection with Autopep8** Apply code formatter.

------------------------------------------------------------------------

# Run Menu

``` text
Run
├─ Run                                          F5
├─ Re-run last file                             F6
├─ Configuration per file                       Ctrl+F6
├─ Global presets
├─ [separator]
├─ Run cell                                     Ctrl+Return
├─ Run cell and advance                         Shift+Return
├─ Re-run last cell                             Alt+Return
├─ Run current line/selection                   F9
├─ Run to line                                  Shift+F9
├─ Run from line                                Alt+F9
├─ [separator]
├─ Run in external terminal
├─ [separator]
├─ Profile file                                 F10
├─ Profile cell                                 Alt+F10
└─ Profile current line or selection
```

## Run Menu Explanations

-   **Run / Re-run last file** Main file execution commands.
-   **Configuration per file** Open per-file run configuration dialog.
-   **Global presets** Open run preset configuration in Preferences.
-   **Run cell / Run cell and advance / Re-run last cell** Cell-based
    execution.
-   **Run current line/selection / Run to line / Run from line**
    Granular execution helpers.
-   **Run in external terminal** Launch code in a standalone system
    terminal process.
-   **Profile file / Profile cell / Profile current line or selection**
    Profiling entry points.

------------------------------------------------------------------------

# Debug Menu

``` text
Debug
├─ Debug file                                   Ctrl+F5
├─ Debug cell
├─ Debug the current line or selection
├─ [separator]
├─ Debug current line                           Ctrl+F10       [disabled in screenshot]
├─ Step into function or method                 Ctrl+F11       [disabled in screenshot]
├─ Execute until function returns               Ctrl+Shift+F11 [disabled in screenshot]
├─ Execute until next breakpoint                Ctrl+F12       [disabled in screenshot]
├─ Stop debugging                               Ctrl+Shift+F12 [disabled in screenshot]
├─ [separator]
├─ Toggle breakpoint                            F12
├─ Set/edit conditional breakpoint              Shift+F12
├─ Clear breakpoints in all files
└─ List breakpoints
```

## Debug Menu Explanations

-   **Debug file / Debug cell / Debug the current line or selection**
    Start a debugger session for different scopes.
-   **Debug current line / Step into / Execute until returns / Execute
    until next breakpoint / Stop debugging** Active-session debugger
    controls.
-   **Toggle breakpoint** Add or remove a breakpoint on the current
    editor line.
-   **Set/edit conditional breakpoint** Open condition editor for
    breakpoint.
-   **Clear breakpoints in all files** Global breakpoint reset.
-   **List breakpoints** Open breakpoint listing UI.

------------------------------------------------------------------------

# Consoles Menu

``` text
Consoles
├─ New console (default settings)               Ctrl+T
├─ New console in environment >
│  └─ Conda: spyder-runtime 0
├─ New special console >
│  ├─ New Pylab console (data plotting)
│  ├─ New SymPy console (symbolic math)
│  └─ New Cython console (Python with C extensions)
├─ New console in remote server >
│  └─ Manage remote connections
├─ Connect to existing kernel...
├─ [separator]
├─ Interrupt kernel
├─ Restart kernel                               Ctrl+.
└─ Remove all variables                         Ctrl+Alt+R
```

## Consoles Menu Explanations

-   **New console (default settings)** Start a new IPython console with
    standard configuration.
-   **New console in environment \>** Start a console bound to a
    selected environment.
-   **New special console \>** Launch specialty consoles with
    preconfigured scientific tooling.
-   **New console in remote server \>** Entry point to remote kernel /
    server workflows.
-   **Connect to existing kernel...** Attach to an already running
    Jupyter/IPython kernel.
-   **Interrupt kernel / Restart kernel / Remove all variables** Session
    lifecycle and namespace management.

------------------------------------------------------------------------

# Projects Menu

``` text
Projects
├─ New Project...
├─ Open Project...
├─ Close Project
├─ Delete Project
├─ [separator]
├─ Recent Projects >
│  ├─ <recent project path item(s)>
│  ├─ [separator]
│  ├─ Clear this list
│  └─ Maximum number of recent projects
```

## Projects Menu Explanations

-   **New Project...** Create a project root with project metadata.
-   **Open Project...** Open an existing project.
-   **Close Project** Detach current project context.
-   **Delete Project** Remove project metadata / project entry.
-   **Recent Projects \>** Recently opened project registry and
    maintenance actions.

------------------------------------------------------------------------

# Tools Menu

``` text
Tools
├─ PYTHONPATH manager
├─ User environment variables
├─ Manage remote connections
├─ [separator]
├─ Preferences
└─ Reset all preferences to defaults
```

## Tools Menu Explanations

-   **PYTHONPATH manager** Open list editor for extra module search
    paths.
-   **User environment variables** Open environment-variable editor.
-   **Manage remote connections** Open SSH / JupyterHub remote
    connection manager.
-   **Preferences** Open global preferences dialog.
-   **Reset all preferences to defaults** Reset stored configuration to
    factory settings.

------------------------------------------------------------------------

# Window Menu

``` text
Window
├─ Panes >
│  ├─ Editor                                    Ctrl+Shift+E      [checked]
│  ├─ IPython Console                           Ctrl+Shift+I      [checked]
│  ├─ Variable Explorer                         Ctrl+Shift+V      [checked]
│  ├─ Debugger                                  Ctrl+Shift+D      [checked]
│  ├─ Help                                      Ctrl+Shift+H      [checked]
│  ├─ Plots                                     Ctrl+Shift+G      [checked]
│  ├─ [separator]
│  ├─ Files                                     Ctrl+Shift+X      [checked]
│  ├─ Outline                                   Ctrl+Shift+O      [unchecked]
│  ├─ Project                                   Ctrl+Shift+P      [checked]
│  ├─ Find                                      Ctrl+Shift+F      [unchecked]
│  ├─ [separator]
│  ├─ History                                   Ctrl+Shift+L      [checked]
│  ├─ Profiler                                  Ctrl+Shift+R      [checked]
│  ├─ Code Analysis                             Ctrl+Shift+C      [unchecked]
│  ├─ [separator]
│  ├─ Online help                               [unchecked]
│  └─ Internal console                          [unchecked]
├─ Unlock panes and toolbars                    Ctrl+Shift+F5
├─ Maximize current pane                        Ctrl+Alt+Shift+M
├─ Close current pane                           Ctrl+Shift+F4
├─ [separator]
├─ Toolbars >
│  ├─ File toolbar                              [checked]
│  ├─ Run toolbar                               [checked]
│  ├─ Debug toolbar                             [checked]
│  ├─ Profile toolbar                           [checked]
│  ├─ Main toolbar                              [checked]
│  └─ Current working directory                 [checked]
├─ Hide toolbars
├─ [separator]
├─ Window layouts >
│  ├─ Default layout
│  ├─ Rstudio layout
│  ├─ Matlab layout
│  ├─ Horizontal split
│  ├─ Vertical split
│  ├─ [separator]
│  ├─ Save current layout
│  ├─ Layout preferences
│  └─ Reset to Spyder default
├─ Use next layout                              Alt+Shift+PgDown
├─ Use previous layout                          Alt+Shift+PgUp
├─ [separator]
└─ Fullscreen mode                              F11
```

## Window Menu Explanations

-   **Panes \>** Show/hide dockable panes.
-   **Unlock panes and toolbars** Toggle layout lock.
-   **Maximize current pane** Temporarily maximize focused pane.
-   **Close current pane** Close the currently focused pane.
-   **Toolbars \> / Hide toolbars** Manage toolbar visibility.
-   **Window layouts \>** Built-in layout presets plus layout
    persistence actions.
-   **Use next layout / Use previous layout** Cycle through saved
    layouts.
-   **Fullscreen mode** Toggle fullscreen.

------------------------------------------------------------------------

# Help Menu

``` text
Help
├─ Interactive tour
├─ Built-in tutorial
├─ Shortcuts summary
├─ [separator]
├─ Spyder documentation                         F1
├─ Tutorial videos
├─ IPython documentation >
│  ├─ Intro to IPython
│  ├─ Console help
│  └─ Quick reference
├─ Troubleshooting guide
├─ Spyder Google group
├─ Dependency status
├─ Report issue...
├─ [separator]
├─ Check for updates
├─ Help Spyder...
└─ About Spyder
```

## Help Menu Explanations

-   **Interactive tour / Built-in tutorial / Shortcuts summary**
    Onboarding and training resources.
-   **Spyder documentation** Open main documentation.
-   **Tutorial videos** External learning material.
-   **IPython documentation \>** IPython-focused documentation
    shortcuts.
-   **Troubleshooting guide** Diagnostic/support documentation.
-   **Spyder Google group** Community support entry.
-   **Dependency status** Show installed dependency report.
-   **Report issue...** Open issue-reporting workflow.
-   **Check for updates** Update check.
-   **Help Spyder...** Contribution/support page.
-   **About Spyder** Application about box.

------------------------------------------------------------------------

# U++ Construction Notes

## Separator Handling

Use:

``` cpp
bar.Separator();
```

## Check States

``` cpp
bar.Add("Wrap lines", [=]{ ToggleWrapLines(); }).Check(wrap_lines);
```

## Disabled Actions

``` cpp
bar.Add("Step into function or method", [=]{ StepInto(); })
   .Key(K_CTRL|K_F11)
   .Enable(debug_session_active);
```

## Submenus

``` cpp
bar.Sub("Open recent", [=](Bar& b){ OpenRecentMenu(b); });
```

------------------------------------------------------------------------

# Confidence Notes

This document is reliable for all entries directly visible in the
screenshots.

Possible uncaptured areas: - disabled submenu contents for some Source
diagnostics - any items not expanded on screen

Those should be treated as **not captured**, not invented.
