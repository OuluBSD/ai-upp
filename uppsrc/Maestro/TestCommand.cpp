#include "Maestro.h"

#ifdef flagGUI
#include <Ctrl/Automation/Automation.h>

namespace Upp {

void TestCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI test <script.py>\n";
}

void TestCommand::Execute(const Vector<String>& args) {
	if(args.GetCount() < 1) { ShowHelp(); return; }
	
	String script_path = args[0];
	String content = LoadFile(script_path);
	if(content.IsVoid()) {
		Cerr() << "Error: Could not read script file: " << script_path << "\n";
		return;
	}

	Cout() << "Starting UI Automation Test: " << script_path << "\n";

	Thread().Run([=] {
		try {
			SetAddMockFn([](const String& r, const String& s) { MockMaestroEngine::Get().AddMock(r, s); });
			
			PyVM vm;
			RegisterAutomationBindings(vm);
			SetCurrentVM(&vm);
			
			Tokenizer tk;
			tk.SkipComments();
			tk.SkipPythonComments();
			if(!tk.Process(content, script_path)) return;
			tk.NewlineToEndStatement();
			tk.CombineTokens();

			PyCompiler compiler(tk.GetTokens());
			Vector<PyIR> ir;
			compiler.Compile(ir);

			vm.SetIR(ir);
			vm.Run();
			
			SetCurrentVM(nullptr);
			
			Cout() << "✓ Test completed successfully.\n";
		} catch (Exc& e) {
			Cerr() << "✗ Test failed:\n" << e << "\n";
		}
	});
}

}
#endif
