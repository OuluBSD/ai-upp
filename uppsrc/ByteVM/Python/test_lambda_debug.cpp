#include <ByteVM/ByteVM.h>
#include <Core/TextParsing/TextParsing.h>

using namespace Upp;

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
	Tokenizer tk;
	tk.SkipPythonComments();
	tk.HaveIdents();
	tk.CombineTokens();
	tk.NewlineToEndStatement();
	
	if(!tk.Process(src, "test_lambda.py")) {
		LOG("Tokenization failed");
		return;
	}
	
	LOG("Tokens:");
	for(int i = 0; i < tk.GetTokens().GetCount(); i++) {
		const Token& t = tk.GetTokens()[i];
		LOG("  " << i << ": type=" << t.type << " (" << Token::GetTypeStringStatic(t.type) << ") str='" << t.str_value << "' line=" << t.loc.line);
	}
	
	PyCompiler compiler(tk.GetTokens(), "test_lambda.py");
	Vector<Upp::PyIR> ir;
	try {
		compiler.Compile(ir);
		LOG("Compilation succeeded! IR count: " << ir.GetCount());
	} catch(Exc& e) {
		LOG("Compilation failed: " << e);
	}
}
