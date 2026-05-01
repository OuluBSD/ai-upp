#include "DomBindings.h"
#include "JsCompiler.h"

using namespace Upp;

void TestEcmaScriptVM()
{
	String src = R"(
let x = 10;
let y = 20;
if (x < y) {
    console.log("x is less than y");
    document.getElementById("status").innerHTML = "Updated";
} else {
    console.log("x is not less than y");
}
)";

	Tokenizer tk;
	tk.SkipPythonComments(); // JS comments are similar enough for this mock
	tk.HaveIdents();
	tk.CombineTokens();
	tk.NewlineToEndStatement();
	
	if(!tk.Process(src, "test.js")) {
		RLOG("Tokenization failed");
		return;
	}
	
	JsCompiler compiler(tk.GetTokens(), "test.js");
	Vector<Upp::PyIR> ir;
	try {
		compiler.Compile(ir);
		RLOG("Compilation succeeded! IR count: " << ir.GetCount());
		
		JsVM vm;
		InitDomBindings(vm);
		vm.SetIR(ir);
		vm.Run();
		
		RLOG("VM Run finished.");
		JsValue doc = vm.GetGlobals().GetItem(JsValue("document"));
		// JsValue el = doc.GetItem(JsValue("getElementById")).Call({JsValue("status")}); // Placeholder Call
		// Since Call is placeholder, manually check globals or effects if any
	} catch(Exc& e) {
		RLOG("Error: " << e);
	}
}
