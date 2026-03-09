# Phase Audit: Inspection Panes Alignment

## Goal
Verify that the implementation of the Variable Explorer and Plots Pane meets or exceeds the requirements in `spyder/*.md`.

## Mandatory Checklist

### 1. Variable Explorer Alignment (spyder/GUI.md)
- [ ] **Widget**: Is it an `ArrayCtrl`?
- [ ] **Columns**: Does it have Name, Type, Size, and Value columns?
- [ ] **Functionality**: Can you edit variables? (Not a shortcut: must update VM state).
- [ ] **Sorting**: Do columns sort correctly?
- [ ] **Icons**: Are type-specific icons present?

### 2. Plots Pane Alignment (spyder/GUI.md)
- [ ] **Widget**: Is it an `ImageCtrl`?
- [ ] **Capability**: Does it display figures exported by the runtime?
- [ ] **Quality**: Does it handle resizing/aspect ratios correctly?

### 3. Architecture Alignment (spyder/APPLICATION_ARCHITECTURE.md)
- [ ] **Modular**: Are `VariableExplorer` and `PlotsPane` in separate files in `Panes/` directory (or similar clean structure)?
- [ ] **Event Flow**: Does the `VariableExplorer` refresh automatically upon `Execution finished` (as per `EVENT_FLOW.md`)?

### 4. Quality Standards
- [ ] **No Stubs**: Are all "Data Viewers" for lists/dicts fully implemented?
- [ ] **No Shortcuts**: Is the communication between VM and UI handled via proper callbacks/events, not global state?
- [ ] **Code Quality**: Are all UI elements using `SizePos` and following U++ layout conventions?

## Final Action
If any item above is NOT checked, the phase is **NOT COMPLETE**. Do not mark as done in `cookie.txt`.
