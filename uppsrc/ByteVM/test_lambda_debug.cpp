#include "ByteVM.h"
#include <Core/TextParsing/PythonTokenizer.h>

void TestLambdaCompilation()
{
	String src = R"(
d = {
    "a": lambda x: x + 1,
    "b": lambda: print("hello")
}
f = lambda x: x * 2
)";

	Vector<Token> tokens;
	PythonTokenizer tk;
	tk.Combine2();
	tk.Combine3();
	tk.NewlineToEndStatement();
	
	if(!tk.Process(src, "test_lambda.py")) {
		LOG("Tokenization failed");
		return;
	}
	
	LOG("Tokens:");
	for(int i = 0; i < tk.GetTokens().GetCount(); i++) {
		const Token& t = tk.GetTokens()[i];
		LOG(Format("  %d: type=%d (%s) str='%s' line=%d", i, t.type, Token::GetTypeStringStatic(t.type), t.str_value, t.loc.line));
	}
	
	PyCompiler compiler(tk.GetTokens(), "test_lambda.py");
	Vector<PyIR> ir;
	try {
		compiler.Compile(ir);
		LOG("Compilation succeeded! IR count: %d", ir.GetCount());
	} catch(Exc& e) {
		LOG("Compilation failed: %s", (const char*)e);
	}
}
