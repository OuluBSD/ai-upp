# Task: Implement Find in Files Pane

## Goal
Implement the `FindInFilesPane` to provide global project-wide search functionality, as specified in `spyder/GUI.md`.

## U++ Widget Mapping
- **Main Container**: `DockableCtrl`.
- **Search Bar**: `EditString` for search pattern, `Button` to trigger.
- **Results**: `ArrayCtrl` to list occurrences (File, Line, Text).

## Interface Definition (FindInFilesPane.h)

```cpp
class FindInFilesPane : public DockableCtrl {
public:
	typedef FindInFilesPane CLASSNAME;
	FindInFilesPane();

	void SetRoot(const String& path) { root_path = path; }
	
	Event<const String&, int> WhenOpenMatch;

private:
	EditString search_pattern;
	Button     search_btn;
	ArrayCtrl  results;
	String     root_path;

	void OnSearch();
	void OnResultOpen();
};
```

## Implementation Steps

### 1. UI Layout
- Initialize `Title("Find in Files")` and `Icon(CtrlImg::search())`.
- Layout: Top row with `search_pattern` and `search_btn`.
- Remaining area: `results` `ArrayCtrl`.

### 2. Search Logic
- Recursively scan `root_path` for `.py` files.
- For each file, read lines and check for `search_pattern`.
- Add matches to `results` list: `File`, `Line`, `Text`.

### 3. Navigation
- Double-clicking a result triggers `WhenOpenMatch(file, line)`.

## Success Criteria
- [ ] Search pattern input and button work.
- [ ] Results show correct file paths and line numbers.
- [ ] Double-clicking a result notifies the IDE to open the file at the correct line.
- [ ] No stubs: actual filesystem scanning must be implemented.
