# Task: Connect Console to ByteVM

## Goal
Wire the PythonConsole input to ByteVM execution and handle output.

## Changes needed in PythonIDE

In `PythonIDE.h`:
```cpp
class PythonIDE : public DockWindow {
    // ...
    void OnConsoleInput();
};
```

In `PythonIDE.cpp`:
```cpp
PythonIDE::PythonIDE()
{
    // ...
    python_console.WhenInput = [=] { OnConsoleInput(); };
}

void PythonIDE::OnConsoleInput()
{
    String cmd = python_console.GetInput();
    if(cmd.IsEmpty()) return;
    
    // Process input through ByteVM
    // Use existing vm member
    try {
        Tokenizer tk;
        tk.SkipComments();
        tk.SkipPythonComments();
        if(!tk.Process(cmd, "<stdin>")) return;
        tk.NewlineToEndStatement();
        tk.CombineTokens();

        PyCompiler compiler(tk.GetTokens(), "<stdin>");
        Vector<PyIR> ir;
        compiler.Compile(ir);

        vm.SetIR(ir);
        PyValue res = vm.Run();
        if(!res.IsNone())
            python_console.Write(res.Repr() + "
");
    }
    catch (Exc& e) {
        python_console.WriteError(e + "
");
    }
}
```

## Changes needed in PythonConsole

Add `WhenInput` callback and `GetInput()` method.

## Success Criteria
- Typing Python code in console (e.g. `1 + 1`) shows result (`2`)
- Errors are displayed in console
- VM state persists between commands
