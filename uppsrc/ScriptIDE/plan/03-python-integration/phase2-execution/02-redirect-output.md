# Task: Redirect ByteVM Output to Console

## Goal
Intercept `print()` calls and other output from ByteVM and display them in the PythonConsole.

## Implementation Details

ByteVM uses a `builtin_print` function that currently writes to `Cout()`. We need to modify this to use a callback or a configurable output stream.

### Changes in ByteVM (PyVM)

Add an output callback to `PyVM`:
```cpp
class PyVM {
    // ...
public:
    Event<const String&> WhenPrint;
};
```

Update `builtin_print` in `PyVM.cpp` to use this callback if set.

### Changes in PythonIDE

Connect `vm.WhenPrint` to `python_console.Write`.

## Files Modified
- `uppsrc/ByteVM/PyVM.h`
- `uppsrc/ByteVM/PyVM.cpp`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Success Criteria
- Running a script with `print("hello")` shows "hello" in the IDE console
- Sequential prints appear on new lines or as specified by `print()` arguments
