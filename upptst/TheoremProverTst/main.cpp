#include <Core/Core.h>
#include <AI/Logic/TheoremProver.h>
#include <AI/Algo/FastSearch.h>

using namespace Upp;
using namespace TheoremProver;

struct IntNode : Moveable<IntNode> {
	int id;
	bool operator==(const IntNode& o) const {return id == o.id;}
	hash_t GetHashValue() const {return id;}
};

struct IntGen : FastSearchGenerator<IntNode> {
	void Generate(const IntNode& n, Vector<IntNode>& out) override {
		if (n.id < 10) {
			IntNode next;
			next.id = n.id + 1;
			out.Add(next);
		}
	}
	bool IsGoal(const IntNode& n) override { return n.id == 5; }
};

void TestSearch() {
	IntGen gen;
	IntNode start;
	start.id = 0;
	bool res = FastSearch<IntNode>::BFS(start, gen);
	if(res) Cout() << "[OK] BFS Test\n";
	else Cout() << "[FAIL] BFS Test\n";
}

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
	
	TestSearch();
	
	Test("P or not P", true);
	Test("P and not P", false);
	Test("exists x. (P(x) implies forall y. P(y))", true);
	Test("x = x", true);
	Test("x = y implies y = x", true); 
	Test("x = y and y = z implies x = z", true);
	
	Cout() << "Tests complete.\n";
}