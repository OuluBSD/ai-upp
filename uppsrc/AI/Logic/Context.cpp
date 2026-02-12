#include "TheoremProver.h"

namespace TheoremProver {

Index<NodeVar> axioms;
ArrayMap<NodeVar, Index<NodeVar> > lemmas;
Index<NodeVar> lemma_cache;
Vector<String> current_proof_steps;
bool flag_silent_prover = false;
String* catch_print = NULL;
Event<String> WhenPrint;
LogLevel current_log_level = LL_INFO;

void ClearProofSteps() {
	current_proof_steps.Clear();
}

void Log(LogCategory cat, LogLevel level, String s) {
	if (level > current_log_level) return;
	
	String prefix;
	switch(cat) {
		case LOG_GENERAL:     prefix = "[GEN] "; break;
		case LOG_SEARCH:      prefix = "[SRCH] "; break;
		case LOG_UNIFICATION: prefix = "[UNIF] "; break;
		case LOG_CONSTRAINT:  prefix = "[CTRL] "; break;
		case LOG_FACT:        prefix = "[FACT] "; break;
	}
	
	String msg = prefix + s;
	
	if (!flag_silent_prover)
		current_proof_steps.Add(msg);
	
	if (catch_print) {*catch_print << msg << '\n';}
	
	if(WhenPrint) WhenPrint(msg);
	else {
		switch(level) {
			case LL_ERROR: RLOG("ERROR: " << msg); break;
			case LL_WARN:  RLOG("WARN: " << msg); break;
			case LL_INFO:  RLOG(msg); break;
			case LL_DEBUG: DLOG(msg); break;
			case LL_TRACE: DLOG("TRACE: " << msg); break;
		}
	}
}

void Print(String s) {
	Log(LOG_GENERAL, LL_INFO, s);
}

}
