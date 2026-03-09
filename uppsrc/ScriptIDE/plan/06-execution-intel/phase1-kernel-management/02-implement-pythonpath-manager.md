# Task: Implement PYTHONPATH Manager

## Goal
Implement a manager and dialog for configuring the search paths for the Python runtime, as specified in `spyder/GUI.md`.

## U++ Widget Mapping
- **Manager**: Internal storage of paths.
- **Dialog**: `WithPYTHONPATHLayout<TopWindow>` with an `ArrayCtrl` for the list of paths.

## Interface Definition (PathManager.h)

```cpp
class PathManager {
public:
	void AddPath(const String& path);
	void RemovePath(int index);
	const Vector<String>& GetPaths() const { return paths; }

	void SyncToVM(PyVM& vm);

private:
	Vector<String> paths;
};
```

## Implementation Steps

### 1. Logic
- Implement `PathManager` to store and persist paths (e.g., in `settings.bin`).
- Implement `SyncToVM` to update `sys.path` in the `PyVM` globals.

### 2. UI
- Create a dialog to add/remove/move paths.
- Add "PYTHONPATH Manager" to the "Tools" menu.

## Success Criteria
- [ ] User can add custom directories to the search path.
- [ ] `import` statements in scripts respect the configured paths.
- [ ] Paths are saved across sessions.
