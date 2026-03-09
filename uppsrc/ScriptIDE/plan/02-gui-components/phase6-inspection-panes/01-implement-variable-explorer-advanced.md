# Task: Implement Advanced Variable Explorer

## Goal
Transform the current basic `VariableExplorer` into a full-featured inspection tool as specified in `spyder/GUI.md` and `spyder/UPP_INTERNALS.md`. It must support sorting, type-specific icons, and live value editing.

## U++ Widget Mapping
- **Main Container**: `DockableCtrl` (already implemented as `var_dock`).
- **Data Table**: `ArrayCtrl` (with `.Sorting().ColumnCaseInsensitive()`).
- **Icons**: `CtrlImg` for types (Numeric, String, List, Dict, Function).

## Interface Definition (VariableExplorer.h)

```cpp
class VariableExplorer : public ParentCtrl {
public:
    typedef VariableExplorer CLASSNAME;
    VariableExplorer();

    void SetVariables(const VectorMap<String, PyValue>& vars);
    void Clear();

private:
    ArrayCtrl list;
    
    void OnLeftDouble();
    void OnContextMenu(Bar& bar);
    void RemoveSelected();
    void InspectSelected(); // Opens detailed viewer dialog
    
    // Helper to get type string and icon
    String GetTypeString(const PyValue& v);
    Image  GetTypeIcon(const PyValue& v);
};
```

## Implementation Steps

### 1. Configure ArrayCtrl
In the constructor, set up columns:
- `Name`: `AddColumn("Name", 30)`
- `Type`: `AddColumn("Type", 15)`
- `Size`: `AddColumn("Size", 10)`
- `Value`: `AddColumn("Value", 45)`
- Enable `.Sorting()` and `.SetLineCy(20)`.

### 2. Implement SetVariables
- Iterate through the `VectorMap`.
- For each `PyValue`:
    - Calculate "Size" (length of string, length of list, count of dict keys).
    - Get "Type" string (e.g., `<class 'int'>`).
    - Get truncated "Value" representation (using `PyValue::Repr()`).
- Use `list.Add(icon, name, type, size, value)`.

### 3. Implement specialized Data Viewers (No Stubs!)
- Create a `DataViewerDialog` (inherits `TopWindow`).
- If a list is double-clicked, show a `ColumnList` or `ArrayCtrl` of its contents.
- If a dict is double-clicked, show a two-column `ArrayCtrl` of keys/values.

### 4. Integration with PythonIDE
- Ensure `PythonIDE::UpdateVariableExplorer` is called whenever the VM pauses or finishes execution.

## Success Criteria
- [ ] Variables appear automatically after a script runs.
- [ ] Table columns are sortable by clicking headers.
- [ ] Correct icons appear for `int`, `str`, `list`, and `dict`.
- [ ] Double-clicking a variable opens a detailed inspection window (not a stub).
- [ ] Context menu allows removing a variable from the display.
