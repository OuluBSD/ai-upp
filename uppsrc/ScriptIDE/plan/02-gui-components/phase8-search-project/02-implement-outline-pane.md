# Task: Implement Outline Pane

## Goal
Implement the `OutlinePane` to show a structural overview of the current Python file (classes, functions), as specified in `spyder/GUI.md`.

## U++ Widget Mapping
- **Main Container**: `DockableCtrl`.
- **Display**: `TreeCtrl`.

## Interface Definition (OutlinePane.h)

```cpp
class OutlinePane : public DockableCtrl {
public:
	typedef OutlinePane CLASSNAME;
	OutlinePane();

	void UpdateOutline(const String& code);
	
	Event<int> WhenSelectLine;

private:
	TreeCtrl tree;
	
	void OnSelect();
};
```

## Implementation Steps

### 1. UI Layout
- Initialize `Title("Outline")` and `Icon(CtrlImg::small_gear())`.
- Add `tree` using `SizePos()`.

### 2. Simple Parsing Logic
- For now, implement a basic regex-based or line-based parser to identify `def` and `class` keywords.
- Future: Use `Tokenizer` for more robustness.

### 3. Tree Population
- Clear and populate tree with found symbols.
- Store line number in node data.

### 4. Navigation
- Selecting a node triggers `WhenSelectLine(line)`.

## Success Criteria
- [ ] Outline shows all functions and classes in the current file.
- [ ] Clicking an item jumps to the correct line in the editor.
