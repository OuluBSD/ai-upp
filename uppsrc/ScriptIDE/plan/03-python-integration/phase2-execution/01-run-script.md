# Task: Implement Run Script Action

## Goal
Enable running the code currently in the editor through the ByteVM interpreter.

## Implementation in PythonIDE.cpp

```cpp
void PythonIDE::OnRun()
{
	String code = code_editor.Get();
	if(code.IsEmpty()) return;
	
	python_console.Clear();
	python_console.Write("--- Running script ---
");
	
	try {
		Tokenizer tk;
		tk.SkipComments();
		tk.SkipPythonComments();
		// Use current file name if available, else <editor>
		String filename = "<editor>"; 
		if(!tk.Process(code, filename)) return;
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens(), filename);
		Vector<PyIR> ir;
		compiler.Compile(ir);

		vm.SetIR(ir);
		vm.Run();
		
		python_console.Write("--- Script finished ---
");
	}
	catch (Exc& e) {
		python_console.WriteError("Runtime error: " + e + "
");
	}
}
```

## Success Criteria
- Clicking "Run" (or pressing F5) in the menu executes the editor's code
- Output is displayed in the console
- Script finish message is shown
- Errors are caught and displayed
