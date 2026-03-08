#include <Core/Core.h>
#include <AI/Logic/TheoremProver.h>

using namespace Upp;
using namespace TheoremProver;

void Test(const char *formula, bool expected = true)
{
	Cout() << "Testing: " << formula << " ... ";
	NodeVar n = Parse(formula);
	if (!n.Is()) {
		Cout() << "PARSE ERROR\n";
		exit(1);
	}
	
	bool result = ProveFormula(Index<NodeVar>(), n);
	if (result == expected) {
		Cout() << "OK\n";
	}
	else {
		Cout() << "FAILED (expected " << (expected ? "true" : "false") << ")\n";
		exit(1);
	}
}

void TestEquality()
{
	Cout() << "Testing Equality ... ";
	ClearLogic();
	Index<NodeVar> axioms;
	axioms.Add(Parse("a = b"));
	axioms.Add(Parse("P(a)"));
	NodeVar n = Parse("P(b)");
	
	if (ProveFormula(axioms, n)) {
		Cout() << "OK\n";
	}
	else {
		Cout() << "FAILED\n";
		exit(1);
	}
}

void TestLemma()
{
	Cout() << "Testing Lemma Caching ... ";
	ClearLogic();
	NodeVar n = Parse("P implies P");
	ProveFormula(Index<NodeVar>(), n);
	
	// Should be in cache now
	if (lemma_cache.Find(n) != -1) {
		Cout() << "OK\n";
	}
	else {
		Cout() << "FAILED (not in cache)\n";
		exit(1);
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	
	Test("P implies P");
	Test("P or not P");
	Test("not (P and not P)");
	Test("(P implies Q) iff (not Q implies not P)");
	Test("forall x. P(x) implies P(a)");
	Test("exists x. P(x) iff not forall x. not P(x)");
	Test("(forall x. P(x) implies Q(x)) implies (forall x. P(x) implies forall x. Q(x))");
	
	TestEquality();
	TestLemma();
	
	Cout() << "All tests passed!\n";
}