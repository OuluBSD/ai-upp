# Phase Audit: Dev Panes Alignment

## Goal
Verify that the implementation of the Debugger and Profiler panes meets or exceeds the requirements in `spyder/*.md`.

## Mandatory Checklist

### 1. Debugger Pane Alignment (spyder/GUI.md)
- [ ] **Widget**: Is it a `TreeCtrl` for stack and `ToolBar` for controls?
- [ ] **Controls**: Does it have Continue, Step Over, Step Into, Step Out, and Stop?
- [ ] **Functionality**: Does selecting a frame update the variable explorer or editor?

### 2. Profiler Pane Alignment (spyder/GUI.md)
- [ ] **Widget**: Is it an `ArrayCtrl`?
- [ ] **Columns**: Does it have Function, Total time, Local time, Calls, and File:Line?

### 3. Quality Standards
- [ ] **No Stubs**: Are all debugging actions fully connected to the `PyVM`?
- [ ] **Direct Inheritance**: Do both panes inherit directly from `DockableCtrl`?
- [ ] **Visuals**: Are appropriate icons and titles initialized?

## Final Action
If any item above is NOT checked, the phase is **NOT COMPLETE**.
