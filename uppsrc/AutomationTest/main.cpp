#include <Core/Core.h>
#include <AI/Logic/TheoremProver.h>

using namespace Upp;

CONSOLE_APP_MAIN {
	StdLogSetup(LOG_FILE);
	
	Index<TheoremProver::NodeVar> axioms;
	TheoremProver::NodeVar n = TheoremProver::Parse("P implies P");
	
	if (TheoremProver::ProveFormula(axioms, n)) {
		LOG("[CTRL] SUCCESS: P implies P");
	} else {
		LOG("[CTRL] FAILURE: P implies P");
	}
	
	Cout() << "Automation Test Complete.\n";
}
