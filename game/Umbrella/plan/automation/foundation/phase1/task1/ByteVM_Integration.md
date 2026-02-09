# Task: ByteVM Integration

## Overview
Add ByteVM and Ctrl/Automation package dependencies to Umbrella, implement command-line `--test` flag, and create basic script execution infrastructure.

## Objective
Enable Python script execution for automated testing following the AriaHub/MaestroHub pattern.

## Prerequisites
- Umbrella builds successfully with current configuration
- Familiarity with `uppsrc/AriaHub/main.cpp` (lines 166-204) for reference implementation
- Understanding of U++ package dependencies (`.upp` file format)

## Implementation Steps

### Step 1: Add Package Dependencies
**File**: `game/Umbrella/Umbrella.upp`

Add the following dependencies:
```
uses
	CtrlLib,
	Docking,
	plugin/jpg,
	plugin/png,
	ByteVM,                    # NEW: Python VM
	Ctrl/Automation;           # NEW: GUI automation bindings

file
	...existing files...
```

**Verification**: Build should succeed with `script/build.py -mc 1 -j 12 Umbrella`

### Step 2: Extend Command-Line Argument Parsing
**File**: `game/Umbrella/main.cpp`

Modify `UmbrellaArgs` struct to add test script path:
```cpp
struct UmbrellaArgs {
	bool testMode = false;
	bool editorMode = false;
	bool newGameMode = false;
	String levelPath;
	String testScript;  // NEW
	int worldNum = 1;
	int stageNum = 1;

	void Parse(const Vector<String>& cmdline);
};
```

Update `Parse()` method:
```cpp
void UmbrellaArgs::Parse(const Vector<String>& cmdline) {
	for(int i = 0; i < cmdline.GetCount(); i++) {
		String arg = cmdline[i];

		if(arg == "--test" && i + 1 < cmdline.GetCount()) {
			testMode = true;
			testScript = cmdline[++i];  // Get script path
		}
		// ...existing arg parsing...
	}
}
```

### Step 3: Implement Test Mode Execution
**File**: `game/Umbrella/main.cpp`

Add test execution branch in `GUI_APP_MAIN`:
```cpp
GUI_APP_MAIN {
	UmbrellaArgs args;
	args.Parse(CommandLine());

	if(args.testMode) {
		return RunAutomationTest(args.testScript);
	}

	// ...existing mode routing...
}
```

### Step 4: Create Test Execution Function
**File**: `game/Umbrella/main.cpp`

Implement `RunAutomationTest()`:
```cpp
int RunAutomationTest(const String& scriptPath) {
	// Verify script exists
	if(!FileExists(scriptPath)) {
		LOG("ERROR: Test script not found: " << scriptPath);
		return 1;
	}

	// Load script source
	String source = LoadFile(scriptPath);
	if(source.IsEmpty()) {
		LOG("ERROR: Failed to load test script: " << scriptPath);
		return 1;
	}

	// Initialize PyVM
	Upp::PyVM vm;

	// Register automation bindings (from Ctrl/Automation package)
	Upp::RegisterAutomationBindings(vm);

	try {
		// Tokenize Python source
		Upp::Tokenizer tokenizer;
		tokenizer.SkipPythonComments(true);
		tokenizer.Process(source, scriptPath);
		tokenizer.CombineTokens();

		// Compile to IR
		Upp::PyCompiler compiler(tokenizer.GetTokens());
		Upp::Vector<Upp::PyIR> ir;
		if(!compiler.Compile(ir)) {
			LOG("ERROR: Failed to compile Python script");
			return 1;
		}

		// Execute
		vm.SetIR(ir);
		Upp::PyValue result = vm.Run();

		LOG("Test script completed successfully");
		return 0;
	}
	catch(const Exc& e) {
		LOG("ERROR: Exception during test execution: " << e);
		return 1;
	}
}
```

### Step 5: Create Test Script Directory
**Shell command**:
```bash
mkdir -p game/Umbrella/tests/{editor,gameplay}
```

### Step 6: Create Hello World Test Script
**File**: `game/Umbrella/tests/hello_test.py`

```python
# Simple test to verify ByteVM integration
print("Hello from ByteVM!")
print("Python automation is working")

# Test basic Python features
x = 42
y = x * 2
print("Math works:", y)

# Test that we can access builtin functions
result = len("test")
print("Builtins work:", result)
```

## Testing

### Build Test
```bash
script/build.py -mc 1 -j 12 Umbrella
```
Expected: Build succeeds with no errors

### Execution Test
```bash
bin/Umbrella --test game/Umbrella/tests/hello_test.py
```
Expected output:
```
Hello from ByteVM!
Python automation is working
Math works: 84
Builtins work: 4
Test script completed successfully
```

### Error Handling Test
```bash
bin/Umbrella --test nonexistent.py
```
Expected: Error message about file not found, exit code 1

## Success Criteria
- [ ] Build succeeds with ByteVM and Ctrl/Automation dependencies
- [ ] `--test` flag recognized and parsed correctly
- [ ] Hello world script executes and prints output
- [ ] Error handling works (missing file, syntax errors)
- [ ] No memory leaks (verify with Valgrind if needed)

## Known Issues / Gotchas
1. **Tokenizer API**: Ensure `SkipPythonComments(true)` is called before `Process()`
2. **IR Compilation**: Check return value of `Compile()` before executing
3. **Exception Handling**: Wrap execution in try/catch for `Upp::Exc`
4. **Path Resolution**: Script paths may be relative to CWD, use absolute paths if needed

## Next Steps
After this task completes:
- **Phase 1 Task 2**: Create `GameScriptBridge` class
- **Phase 1 Task 3**: Register game constants and enums to Python globals

## References
- `uppsrc/AriaHub/main.cpp` lines 166-204: Complete integration example
- `uppsrc/ByteVM/PyVM.h`: PyVM API documentation
- `uppsrc/ByteVM/PyCompiler.h`: Compilation API
- `uppsrc/Ctrl/Automation/Automation.h`: RegisterAutomationBindings() declaration
