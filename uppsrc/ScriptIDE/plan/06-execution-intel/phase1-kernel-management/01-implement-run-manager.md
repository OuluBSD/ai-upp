# Task: Implement Run Manager

## Goal
Implement the `RunManager` service to centralize the lifecycle of Python script execution, fulfilling the specification in `spyder/APPLICATION_ARCHITECTURE.md` and `spyder/EVENT_FLOW.md`.

## Strategy
Move the execution logic from `PythonIDE` into a dedicated `RunManager` class. This class will handle compilation, VM state management, and event dispatching.

## Interface Definition (RunManager.h)

```cpp
class RunManager : public Moveable<RunManager> {
public:
	RunManager(PyVM& vm);

	void Run(const String& code, const String& filename);
	void RunSelection(const String& code);
	void Stop();

	Event<> WhenStarted;
	Event<> WhenFinished;
	Event<const String&> WhenError;

private:
	PyVM& vm;
	bool is_running = false;
};
```

## Implementation Steps

### 1. Create RunManager
- Move `OnRun` and `OnRunSelection` logic into `RunManager`.
- Ensure it handles `Tokenizer` and `PyCompiler` steps.

### 2. Integration with PythonIDE
- `PythonIDE` will own an instance of `RunManager`.
- Connect IDE toolbar/menu actions to `RunManager` methods.
- UI components (VariableExplorer, Console) will update based on `RunManager` events.

## Success Criteria
- [ ] Centralized execution logic.
- [ ] Clear separation between UI and runtime management.
- [ ] All execution types (Full, Selection) are supported.
- [ ] No stubs: full integration with `PyVM`.
