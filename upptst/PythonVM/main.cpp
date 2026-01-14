#include <ByteVM/ByteVM.h>

using namespace Upp;

void Test(const char *code)
{
	Cout() << "========================================\n";
	Cout() << "Testing code:\n" << code << "\n";
	Cout() << "----------------------------------------\n";
	Tokenizer tk;
	tk.SkipPythonComments();
	if(!tk.Process(code, "test.py")) {
		Cout() << "Tokenizing failed\n";
		return;
	}
	tk.CombineTokens();
	tk.NewlineToEndStatement();
	
	// Print tokens for debugging
	Cout() << "Tokens:\n";
	for(const auto& t : tk.GetTokens()) {
		Cout() << t.ToString() << "\n";
	}
	Cout() << "----------------------------------------\n";

	PyCompiler compiler(tk.GetTokens());
	Vector<PyIR> ir;
	try {
		compiler.Compile(ir);
	} catch (Exc& e) {
		Cout() << "Compilation failed: " << e << "\n";
		return;
	}
	
	PyVM vm;
	vm.SetIR(ir);
	vm.Run();
	Cout() << "========================================\n\n";
}

CONSOLE_APP_MAIN
{
	Test("x = 10\n"
	     "y = 20\n"
	     "z = x + y\n"
	     "print(z)\n");
	     
	Test("if True:\n"
	     "    print(\"Inside if\")\n"
	     "else:\n"
	     "    print(\"Inside else\")\n");
	     
	Test("i = 0\n"
	     "while i < 3:\n"
	     "    print(i)\n"
	     "    i = i + 1\n");

	Test("l = [1, 2, \"hello\"]\n"
	     "print(l)\n");
	     
	Test("d = {\"a\": 1, \"b\": 2}\n"
	     "print(d)\n");

	Test("l = [10, 20, 30]\n"
	     "print(l[1])\n");
	     
	Test("d = {\"name\": \"U++\", \"version\": 2026}\n"
	     "print(d[\"name\"])\n");

	Test("def add(a, b):\n"
	     "    return a + b\n"
	     "\n"
	     "res = add(5, 7)\n"
	     "print(res)\n");
	     
	Test("def fib(n):\n"
	     "    if n < 2:\n"
	     "        return n\n"
	     "    return fib(n-1) + fib(n-2)\n"
	     "\n"
	     "print(fib(7))\n");
}
