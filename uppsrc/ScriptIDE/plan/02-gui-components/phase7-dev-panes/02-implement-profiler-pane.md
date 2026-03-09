# Task: Implement Profiler Pane

## Goal
Implement the `ProfilerPane` to display execution timing data, fulfilling the specification in `spyder/GUI.md`.

## U++ Widget Mapping
- **Main Container**: `DockableCtrl`.
- **Data Table**: `ArrayCtrl`.

## Interface Definition (ProfilerPane.h)

```cpp
class ProfilerPane : public DockableCtrl {
public:
	typedef ProfilerPane CLASSNAME;
	ProfilerPane();

	void SetData(const VectorMap<String, Value>& data); // Placeholder for actual profile data
	void Clear();

private:
	ArrayCtrl list;
};
```

## Implementation Steps

### 1. UI Layout
- Initialize `Title("Profiler")` and `Icon(CtrlImg::small_gear())`.
- Configure `ArrayCtrl` columns: "Function", "Total time", "Local time", "Calls", "File:Line".
- Enable sorting and even row colors.

### 2. Integration
- Register in `PythonIDE::DockInit`.
- Connect to a future `ProfilerEngine`.

## Success Criteria
- [ ] Pane is dockable and correctly titled.
- [ ] All specified columns are present.
- [ ] Table is sortable.
