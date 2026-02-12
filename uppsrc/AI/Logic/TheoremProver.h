#ifndef _TheoremProver_TheoremProver_h
#define _TheoremProver_TheoremProver_h

#include <Core/Core.h>

using namespace Upp;

/*
	TheoremProver
	------------------------------------------------------------------------
	
	TheoremProver was forked from theorem_prover, which has:
		Copyright: Stephan Boyer, boyers@github
		License:   New BSD
		Source:    https://github.com/boyers/theorem_prover

	TheoremProver was inspired by following:
		 http://codereview.stackexchange.com/questions/11154/c11-propositional-logic-proposition-evaluator
		
	
	------------------------------------------------------------------------

*/

#include "Language.h"

namespace TheoremProver {

class InvalidInputError : public Exc {

public:
	InvalidInputError ( String msg ) : Exc ( msg ) {}
};

extern Index<NodeVar> axioms;
extern ArrayMap<NodeVar, Index<NodeVar> > lemmas;
extern Index<NodeVar> lemma_cache;
extern Vector<String> current_proof_steps;
extern bool flag_silent_prover;
extern String* catch_print;
extern Event<String> WhenPrint;

void Print(String s);
void ClearProofSteps();
bool ProveFormula(const Index<NodeVar>& axioms, const NodeVar& formula);
void RemoveRef(ArrayMap<NodeVar, int>& ind, const NodeVar& ref);


Vector<String> Lex(const String& inp);
NodeVar Parse(Vector<String>& tokens);
NodeVar Parse(String str);
NodeVar UnsafeParse(String str);
String EvaluateLogicNode(NodeVar ref);
String EvaluateLogic(String str);
String ProveLogicNode(NodeVar formula);
String ProveLogic(String str);
String AddAxiom(String str);
String GetAxioms();
String ProveLemmaNode(NodeVar formula);
String ProveLemma(String str);
String GetLemmas();
void ClearLogic();
NodeVar GetTruthTableDNF(NodeVar n);
NodeVar GetTruthTableCNF(NodeVar n);

void TypecheckTerm(Node& term);
void TypecheckFormula(Node& formula);
void CheckFormula(Node& formula);


}



#endif
