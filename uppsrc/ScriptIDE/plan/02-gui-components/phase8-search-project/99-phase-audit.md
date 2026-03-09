# Phase Audit: Search & Project Panes Alignment

## Goal
Verify that the implementation of the Find in Files and Outline panes meets or exceeds the requirements in `spyder/*.md`.

## Mandatory Checklist

### 1. Find in Files Alignment (spyder/GUI.md)
- [ ] **Widget**: Search input and results table?
- [ ] **Functionality**: Recursive search in project directory?
- [ ] **Navigation**: Jumps to file/line on double-click?

### 2. Outline Pane Alignment (spyder/GUI.md)
- [ ] **Widget**: Tree view?
- [ ] **Symbols**: Shows classes and functions?
- [ ] **Navigation**: Jumps to code on selection?

### 3. Quality Standards
- [ ] **No Stubs**: Search logic is fully functional.
- [ ] **Direct Inheritance**: Both panes inherit from `DockableCtrl`.
- [ ] **Performance**: Search does not block UI (consider using `CoWork` or async logic if slow).

## Final Action
If any item above is NOT checked, the phase is **NOT COMPLETE**.
