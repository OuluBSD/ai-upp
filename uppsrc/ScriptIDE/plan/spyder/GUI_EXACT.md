# Exact GUI Surface Specification (Captured)

This document complements `MENUBAR_SPEC.md` and records the exact
captured GUI surface: main toolbar, panes, pane toolbars, pane menus,
context menus, dialogs and quick-navigation popups.

It is intended for a faithful U++ implementation using `DockWindow` and
`DockableCtrl`.

------------------------------------------------------------------------

# Main Toolbar

``` text
MainToolbar
├─ New File
├─ Open File
├─ Save File
├─ Save All Files
├─ [separator]
├─ Create new cell at the current line
├─ [separator]
├─ Run file
├─ Run cell
├─ Run cell and advance
├─ Run current line or selection
├─ [separator]
├─ Debug file
├─ Debug cell
├─ Debug the current file or selection
├─ [separator]
├─ Profile file
├─ Profile cell
├─ Profile current line or selection
├─ [separator]
├─ Maximize current pane
├─ Preferences
├─ PYTHONPATH manager
├─ [separator]
├─ Recent projects (dropdown)
├─ Working directory path dropdown
├─ Browse working directory
└─ Change to parent directory
```

------------------------------------------------------------------------

# Files Pane

## Location Bar

``` text
FilesLocationBar
├─ PYTHONPATH manager button
├─ Active directory dropdown / path field
├─ Browse directory
└─ Parent directory
```

## Pane Toolbar

``` text
FilesPaneToolbar
├─ Previous
├─ Next
├─ Parent
├─ (align right) Filter filenames
└─ Pane menu
```

## Pane Menu

``` text
FilesPaneMenu
├─ Show hidden files
├─ Edit filter settings...
├─ [separator]
├─ Size
├─ Type
├─ Date modified                           [checked]
├─ [separator]
├─ Single click to open
├─ [separator]
├─ Move
├─ Undock
└─ Close
```

------------------------------------------------------------------------

# Variable Explorer Pane

## Toolbar

``` text
VariableExplorerToolbar
├─ Import data
├─ Save data
├─ Save data as
├─ Remove all variables
├─ [separator]
├─ Search variable names and types
├─ Filter variables
├─ Refresh variables
├─ [separator]
└─ Pane menu
```

## Columns

-   Name
-   Type
-   Size
-   Value

## Pane Menu

``` text
VariableExplorerPaneMenu
├─ Exclude private variables                [checked]
├─ Exclude all-uppercase variables
├─ Exclude capitalized variables
├─ Exclude unsupported data types
├─ Exclude callables and modules            [checked]
├─ Show arrays min/max
├─ [separator]
├─ Resize rows to contents
├─ Resize columns to contents
├─ [separator]
├─ Move
├─ Undock
└─ Close
```

------------------------------------------------------------------------

# Debugger Pane

## Toolbar

``` text
DebuggerToolbar
├─ Debug current line
├─ Execute until next breakpoint
├─ Step into function or method
├─ Execute until function returns
├─ Stop debugging
├─ [separator]
├─ Start debugging after last error
├─ Interrupt execution and start the debugger
├─ Inspect execution
├─ [separator]
├─ Show file/line in editor
├─ Search frames
├─ (align right) Show breakpoints
└─ Pane menu
```

## Pane Menu

``` text
DebuggerPaneMenu
├─ Exclude internal frames when inspecting execution   [checked]
├─ [separator]
├─ Move
├─ Undock
└─ Close
```

------------------------------------------------------------------------

# Plots Pane

## Toolbar

``` text
PlotsToolbar
├─ Save plot as...
├─ Save all plots...
├─ Copy plot to clipboard as image
├─ Remove plot
├─ Remove all plots
├─ [separator]
├─ Zoom percent dropdown
├─ Zoom out
├─ Zoom in
├─ Fit plot to pane size
├─ (align right) Previous plot
├─ Next plot
└─ Pane menu
```

## Pane Menu

``` text
PlotsPaneMenu
├─ Mute inline plotting                      [checked]
├─ Show plot outline
├─ Set maximum number of plots
├─ [separator]
├─ Move
├─ Undock
└─ Close
```

------------------------------------------------------------------------

# Help Pane

## Header Controls

``` text
HelpPaneHeader
├─ Source selector dropdown
├─ Object input field
├─ Home button
├─ Lock button
└─ Pane menu
```

## Pane Menu

``` text
HelpPaneMenu
├─ Rich Text                                (radio)
├─ Plain Text                               (radio)
├─ Show Source                              [checkbox]
├─ [separator]
├─ Automatic import                         [checkbox]
├─ [separator]
├─ Move
├─ Undock
└─ Close
```

------------------------------------------------------------------------

# Profiler Pane

## Toolbar

``` text
ProfilerToolbar
├─ Collapse one level up
├─ Expand one level down
├─ [separator]
├─ Show items with one large local time
├─ Hide calls to external libraries
├─ Show callers/callees
├─ Search
├─ [separator]
├─ Stop profiling
├─ (align right) Save profiling data
├─ Load profiling data comparison
├─ Clear comparison                         [disabled]
└─ Pane menu
```

## Pane Menu

``` text
ProfilerPaneMenu
├─ Move
├─ Undock
└─ Close
```

------------------------------------------------------------------------

# History Pane

## Pane Menu

``` text
HistoryPaneMenu
├─ Wrap lines                               [checked]
├─ Show line numbers
├─ [separator]
├─ Move
├─ Undock
└─ Close
```

------------------------------------------------------------------------

# Find Pane

## Toolbar

``` text
FindToolbar
├─ Search text field
├─ Search button
├─ Regex toggle
├─ Case sensitive toggle
├─ Advanced search toggle
└─ Pane menu
```

## Advanced Search Controls

``` text
FindAdvancedControls
├─ Exclude pattern field
└─ Search in scope dropdown
```

## Pane Menu

``` text
FindPaneMenu
├─ Set maximum number of results
├─ [separator]
├─ Move
├─ Undock
└─ Close
```

------------------------------------------------------------------------

# IPython Console Pane

## Header

``` text
IPythonConsoleTabBar
├─ Console list button
├─ Console tabs
├─ (align right) Clear console
├─ Interrupt kernel
└─ Pane menu
```

## Pane Menu

``` text
IPythonConsolePaneMenu
├─ Interrupt kernel
├─ Restart kernel                           Ctrl+.
├─ Remove all variables                     Ctrl+Alt+R
├─ Rename tab
├─ [separator]
├─ Show environment variables
├─ Show sys.path contents
├─ Show elapsed time                        [checkbox]
├─ [separator]
├─ Switch to next console                   Alt+Shift+Right
├─ Switch to previous console               Alt+Shift+Left
├─ [separator]
├─ Move
├─ Undock
└─ Close
```

## Console Text Area Context Menu

``` text
ConsoleEditorContextMenu
├─ Cut                                      Ctrl+X
├─ Copy                                     Ctrl+C
├─ Copy (raw text)
├─ Paste                                    Ctrl+V
├─ Select all
├─ [separator]
├─ Inspect current object                   Ctrl+I
├─ Enter array table                        Ctrl+M
├─ Enter array inline                       Ctrl+Alt+M
├─ [separator]
├─ Save as html...
├─ Print...
├─ [separator]
├─ Clear console                            Ctrl+L
├─ Clear line or block                      Shift+Esc
├─ [separator]
└─ Quit
```

------------------------------------------------------------------------

# Project Tree Menus

## Folder Context Menu

``` text
ProjectFolderContextMenu
├─ New >
│  ├─ File...
│  ├─ Folder...
│  ├─ [separator]
│  ├─ Python file...
│  └─ Python package...
├─ Delete...
├─ Rename...
├─ [separator]
├─ Copy                                     Ctrl+C
├─ Paste                                    Ctrl+V
├─ Copy absolute path                       Alt+Shift+C
├─ Copy relative path                       Alt+Shift+D
├─ [separator]
├─ Show in folder
└─ Open IPython console here
```

## File Context Menu

``` text
ProjectFileContextMenu
├─ New >
│  ├─ File...
│  ├─ Folder...
│  ├─ [separator]
│  ├─ Python file...
│  └─ Python package...
├─ Run
├─ Open in Spyder
├─ Open externally
├─ Delete...
├─ Rename...
├─ Move...
├─ [separator]
├─ Copy                                     Ctrl+C
├─ Paste                                    Ctrl+V
├─ Copy absolute path                       Alt+Shift+C
├─ Copy relative path                       Alt+Shift+D
├─ [separator]
└─ Show in folder
```

------------------------------------------------------------------------

# Editor Menus

## Editor Tab Menu

``` text
EditorTabMenu
├─ Go to line...                            Ctrl+L
├─ Set console working directory
├─ Show in external file explorer
├─ [separator]
├─ File switcher...                         Ctrl+P
├─ Symbol finder...                         Ctrl+Alt+P
├─ Copy absolute path
├─ Copy relative path
├─ [separator]
├─ Close all to the right
├─ Close all to the left
├─ Close all but this
├─ Sort tabs alphabetically
├─ [separator]
├─ Split vertically
├─ Split horizontally
├─ Close this panel                         [disabled]
├─ [separator]
├─ New window
├─ Move
├─ Undock
└─ Close
```

## Editor Text Context Menu

``` text
EditorContextMenu
├─ Run cell                                 Ctrl+Return
├─ Run cell and advance                     Shift+Return
├─ Re-run last cell                         Alt+Return
├─ Run current line/selection               F9
├─ [separator]
├─ Go to definition
├─ Inspect current object
├─ [separator]
├─ Undo
├─ Redo
├─ [separator]
├─ Cut
├─ Copy
├─ Paste
├─ Select All
├─ [separator]
├─ Zoom in
├─ Zoom out
├─ Zoom reset
├─ [separator]
├─ Comment/Uncomment
├─ Generate docstring
└─ Format file or selection with Autopep8
```

------------------------------------------------------------------------

# Quick Navigation Popups

## Symbol Finder

``` text
SymbolFinderPopup
├─ Search input
└─ Result list
   ├─ symbol kind icon
   └─ symbol name
```

## File Switcher

``` text
FileFinderPopup
├─ Search input
│   placeholder: Search files by name (add @ for symbols)
└─ Result list
   ├─ file name
   ├─ file path
   └─ scope label (Editor / Project)
```

------------------------------------------------------------------------

# Tools Dialogs

## PYTHONPATH Manager

``` text
PythonPathManagerDialog
├─ Path list with enable checkboxes
├─ Add path
├─ Remove path
├─ Move up
├─ Move down
├─ Move to top
├─ Move to bottom
└─ OK / Cancel
```

## Environment Variables Editor

``` text
EnvironmentVariablesEditor
├─ Table columns: Key / Type / Size / Value
├─ Toolbar
└─ Save and Close / Close
```

## Remote Connections

``` text
RemoteConnectionsDialog
├─ Connection list
├─ Tabs: SSH / JupyterHub
└─ Save / Clear / Connect / Cancel
```

------------------------------------------------------------------------

# Notes

This file is deliberately **exact but bounded**: it reflects the
captured GUI surface and does not invent unseen controls.
