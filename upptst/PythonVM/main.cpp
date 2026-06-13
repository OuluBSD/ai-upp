#include <ByteVM/ByteVM.h>

using namespace Upp;

static void RunCode(const char *code, bool expect_error = false, const char *expected_error = nullptr)
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
	try {
		vm.Run();
		if(expect_error)
			throw Exc("Expected runtime error, but script completed successfully");
	}
	catch(Exc& e) {
		if(!expect_error)
			throw;
		Cout() << "Caught expected runtime error: " << e << "\n";
		if(expected_error && *expected_error && String(e).Find(expected_error) < 0)
			throw Exc("Unexpected runtime error message: " + String(e));
	}
	Cout() << "========================================\n\n";
}

CONSOLE_APP_MAIN
{
	RunCode("x = 10\n"
	        "y = 20\n"
	        "z = x + y\n"
	        "print(z)\n");
	     
	RunCode("if True:\n"
	        "    print(\"Inside if\")\n"
	        "else:\n"
	        "    print(\"Inside else\")\n");
	     
	RunCode("i = 0\n"
	        "while i < 3:\n"
	        "    print(i)\n"
	        "    i = i + 1\n");

	RunCode("l = [1, 2, \"hello\"]\n"
	        "print(l)\n");
	     
	RunCode("d = {\"a\": 1, \"b\": 2}\n"
	        "print(d)\n");

	RunCode("l = [10, 20, 30]\n"
	        "print(l[1])\n");
	     
	RunCode("d = {\"name\": \"U++\", \"version\": 2026}\n"
	        "print(d[\"name\"])\n");

	RunCode("def add(a, b):\n"
	        "    return a + b\n"
	        "\n"
	        "res = add(5, 7)\n"
	        "print(res)\n");
	     
	RunCode("def fib(n):\n"
	        "    if n < 2:\n"
	        "        return n\n"
	        "    return fib(n-1) + fib(n-2)\n"
	        "\n"
	        "print(fib(7))\n");

	RunCode("for i in range(5):\n"
	        "    print(i)\n");

	RunCode("hand_state = {\"board\": {\"count\": 1, \"queen\": 2}}\n"
	        "assert \"count\" in hand_state[\"board\"]\n"
	        "print(\"assert passed\")\n");

	RunCode("hand_state = {\"board\": {\"count\": 1, \"queen\": 2}}\n"
	        "assert \"joker\" in hand_state[\"board\"]\n",
	        true, "AssertionError");

	RunCode("hand_state = {\"board\": {\"count\": 1, \"queen\": 2}}\n"
	        "assert \"joker\" in hand_state[\"board\"], \"board missing key\"\n",
	        true, "AssertionError: board missing key");
}
