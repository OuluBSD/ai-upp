#include "TheoremProver.h"

namespace TheoremProver {

Index<NodeVar> axioms;
ArrayMap<NodeVar, Index<NodeVar> > lemmas;
Index<NodeVar> lemma_cache;
Vector<String> current_proof_steps;
bool flag_silent_prover = false;
String* catch_print = NULL;
Event<String> WhenPrint;

void ClearProofSteps() {
	current_proof_steps.Clear();
}

void Print(String s) {
	if (!flag_silent_prover)
		current_proof_steps.Add(s);
	if (catch_print) {*catch_print << s << '\n';}
	if(WhenPrint) WhenPrint(s);
	else LOG(s);
}

}
