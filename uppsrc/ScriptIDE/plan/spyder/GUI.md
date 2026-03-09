# Spyder-like IDE GUI Specification (U++ Implementation)

## Overview

This document specifies the graphical user interface layout inspired by
Spyder IDE, adapted for implementation using the Ultimate++ (U++)
framework.

The main window uses:

DockWindow

All panes inherit from:

DockableCtrl

The design follows a **dockable multi-pane development environment**
similar to MATLAB / Spyder.

------------------------------------------------------------------------

# Main Window

Class: IDEWindow : DockWindow

Components:

MenuBar ToolBar StatusBar Central Editor Area Dockable Panes

    IDEWindow
    ├── MenuBar
    ├── ToolBar
    ├── StatusBar
    ├── EditorTabs (CodeEditor)
    └── DockArea
        ├── FilesPane
        ├── VariableExplorerPane
        ├── DebuggerPane
        ├── ProfilerPane
        ├── PlotsPane
        ├── HelpPane
        ├── HistoryPane
        └── FindPane

------------------------------------------------------------------------

# Menu Structure

File - New file - Open file - Save - Save all - Close - Exit

Edit - Undo - Redo - Cut - Copy - Paste

Search - Find - Search text in files

Run - Run file - Run cell - Run selection - Debug file

Tools - PYTHONPATH manager - User environment variables - Remote
connections

View - Toggle panes

Help - Documentation - About

------------------------------------------------------------------------

# Toolbar

Uses:

ToolBar

Buttons:

New file Open file Save Save all Run file Run cell Run cell and advance
Run selection Debug file Profile file Preferences PythonPath manager
Working directory selector

------------------------------------------------------------------------

# Editor

Widget:

CodeEditor

Features:

syntax highlighting code folding line numbers breakpoints auto
indentation multi cursor

Tabs implemented using:

TabBar

------------------------------------------------------------------------

# Dockable Panes

Each pane:

inherits DockableCtrl

    class FilesPane : public DockableCtrl
    class VariableExplorerPane : public DockableCtrl
    class DebuggerPane : public DockableCtrl
    class ProfilerPane : public DockableCtrl
    class PlotsPane : public DockableCtrl
    class HelpPane : public DockableCtrl
    class HistoryPane : public DockableCtrl
    class FindPane : public DockableCtrl

------------------------------------------------------------------------

# Files Pane

Widgets:

TreeCtrl FileList

Features:

filesystem browsing open file context menu

------------------------------------------------------------------------

# Variable Explorer

Widget:

ArrayCtrl

Columns:

Name Type Size Value

Allows editing variables from Python runtime.

------------------------------------------------------------------------

# Debugger Pane

Widgets:

TreeCtrl ToolBar

Displays:

call stack frames

Debugger controls:

step continue step into step out stop

------------------------------------------------------------------------

# Profiler Pane

Widget:

ArrayCtrl

Columns:

Function Total time Local time Calls File:Line

------------------------------------------------------------------------

# Plots Pane

Widget:

ImageCtrl

Displays matplotlib figures exported by Python runtime.

------------------------------------------------------------------------

# Find Pane

Widgets:

EditString TreeCtrl ToggleButtons

Capabilities:

regex search case sensitive search exclude patterns directory scope

------------------------------------------------------------------------

# Dialog Windows

## PYTHONPATH Manager

ArrayCtrl listing paths.

Operations:

add remove reorder

------------------------------------------------------------------------

## Environment Variables Editor

ArrayCtrl

Columns:

Key Type Size Value

Edits Windows registry environment variables.

------------------------------------------------------------------------

## Remote Connections

Tabs:

SSH JupyterHub

Fields:

hostname port username password

Used for remote kernel execution.

------------------------------------------------------------------------

# Status Bar

Widgets:

memory usage CPU usage cursor position Python interpreter
