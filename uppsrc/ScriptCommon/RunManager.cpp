#include "ScriptCommon.h"

namespace Upp {

RunManager::RunManager(PyVM& vm) : vm(vm)
{
}

void RunManager::Run(const String& code, const String& filename)
{
	WhenStarted();
	try {
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
		
		WhenFinished();
	}
	catch (Exc& e) {
		WhenError(e);
	}
}

void RunManager::RunSelection(const String& code)
{
	WhenStarted();
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
		
		WhenFinished();
	}
	catch (Exc& e) {
		WhenError(e);
	}
}

void RunManager::Stop()
{
	vm.Reset();
	WhenFinished();
}

}
