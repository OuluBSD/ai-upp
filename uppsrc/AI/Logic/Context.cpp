#include "TheoremProver.h"

namespace TheoremProver {

Index<NodeVar> axioms;
ArrayMap<NodeVar, Index<NodeVar> > lemmas;
String* catch_print = NULL;
Event<String> WhenPrint;

void Print(String s) {
	if (catch_print) {*catch_print << s << '\n';}
	if(WhenPrint) WhenPrint(s);
	else LOG(s);
}

}
