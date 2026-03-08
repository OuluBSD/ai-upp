# Task: Implement Run Selection Action

## Goal
Enable running only the currently selected text in the editor through ByteVM.

## Implementation in PythonIDE.cpp

```cpp
void PythonIDE::OnRunSelection()
{
	String code;
	if(code_editor.IsSelection())
		code = code_editor.GetSelection();
	else
		code = code_editor.GetLine(code_editor.GetCursorLine());
		
	if(code.IsEmpty()) return;
	
	python_console.Write("--- Running selection ---
");
	
	try {
		Tokenizer tk;
		tk.SkipComments();
		tk.SkipPythonComments();
		if(!tk.Process(code, "<selection>")) return;
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens(), "<selection>");
		Vector<PyIR> ir;
		compiler.Compile(ir);

		vm.SetIR(ir);
		vm.Run();
	}
	catch (Exc& e) {
		python_console.WriteError("Runtime error: " + e + "
");
	}
}
```

## Success Criteria
- Selecting text and clicking "Run Selection" (or F9) executes only that text
- If no selection, the current line is executed
- VM state persists
