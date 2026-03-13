#include "ScriptCommon.h"

namespace Upp {

RunManager::RunManager(PyVM& vm) : vm(vm)
{
}

void RunManager::Run(const String& code, const String& filename)
{
	WhenStarted();
	try {
		vm.EnableBreakpoints(mode != RUN_NORMAL);
		Tokenizer tk;
		tk.SkipComments();
		tk.SkipPythonComments();
		if(!tk.Process(code, filename)) return;
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens(), filename);
		Vector<PyIR> ir;
		compiler.Compile(ir);

		vm.SetIR(ir);
		vm.Run();
		
		mode = RUN_NORMAL;
		WhenFinished();
	}
	catch (Exc& e) {
		mode = RUN_NORMAL;
		WhenError(e);
	}
}

void RunManager::RunSelection(const String& code)
{
	WhenStarted();
	try {
		vm.EnableBreakpoints(mode != RUN_NORMAL);
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
		
		mode = RUN_NORMAL;
		WhenFinished();
	}
	catch (Exc& e) {
		mode = RUN_NORMAL;
		WhenError(e);
	}
}

void RunManager::Stop()
{
	vm.Reset();
	mode = RUN_NORMAL;
	WhenFinished();
}

}
