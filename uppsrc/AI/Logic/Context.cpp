#include "TheoremProver.h"
namespace TheoremProver {
Index<NodeVar> axioms;
ArrayMap<NodeVar, Index<NodeVar>> lemmas;
Index<NodeVar> lemma_cache;
Vector<String> current_proof_steps;
bool flag_silent_prover = false;
String* catch_print = NULL;
Event<String> WhenPrint;
void ClearProofSteps() { current_proof_steps.Clear(); }
void Log(LogCategory cat, LogLevel level, String s) {}
void Print(String s) {}
}
