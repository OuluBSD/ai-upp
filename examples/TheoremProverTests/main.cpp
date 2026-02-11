#include <Core/Core.h>
#include <AI/Logic/TheoremProver.h>

using namespace Upp;
using namespace TheoremProver;

void Test(String code, bool expected) {
	ClearLogic();
	NodeVar n = Parse(code);
	if (!n.Is()) {
		Cout() << "[ERROR] Parse failed: " << code << "\n";
		return;
	}
	bool result = ProveFormula(Index<NodeVar>(), n);
	if (result == expected) {
		Cout() << "[OK] " << code << "\n";
	} else {
		Cout() << "[FAIL] " << code << " (expected " << (expected ? "True" : "False") << ")\n";
	}
}

CONSOLE_APP_MAIN {
	Cout() << "Running Theorem Prover Tests...\n";
	
	Test("P or not P", true);
	Test("P and not P", false);
	Test("exists x. (P(x) implies forall y. P(y))", true);
	Test("x = x", true);
	Test("x = y implies y = x", true); 
	Test("x = y and y = z implies x = z", true);
	
	Cout() << "Tests complete.\n";
}