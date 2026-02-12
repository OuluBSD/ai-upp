#include "ConstraintVisitor.h"
#include <AI/Logic/TheoremProver.h>

namespace Upp {

using namespace TheoremProver;

Event<String, bool> WhenCheckConstraintsResult;

struct ViolationDisplay : TopWindow {
	Label msg;
	TimeCallback timer;
	
	ViolationDisplay() {
		SetRect(0, 0, 400, 40);
		Add(msg.SizePos());
		msg.SetAlign(ALIGN_CENTER);
		msg.SetFont(StdFont().Bold());
		msg.SetInk(White());
		Background(Red());
		TopMost();
		Title("UGUI Constraint Violation");
		Ignorable();
	}
	
	void ShowError(const String& s) {
		msg.SetText(s);
		if(!IsOpen()) {
			Rect r = GetWorkArea();
			SetRect(r.left + 10, r.top + 10, 400, 40);
			Open();
		}
		timer.KillSet(3000, [this] { Close(); });
	}
};

ViolationDisplay& GetViolationDisplay() {
	static ViolationDisplay d;
	return d;
}

void CheckUGUIConstraints()
{
	if(Ctrl::constraints.IsEmpty()) return;
	
	// Get all top windows
	Vector<Ctrl *> tops = Ctrl::GetTopWindows();
	if(tops.IsEmpty()) return;
	
	ConstraintVisitor v;
	for(Ctrl *top : tops)
		v.CollectFacts(*top);
	
	const Index<String>& facts = v.GetFacts();
	
	String title = GetExeTitle();
	String home = GetHomeDirectory();
	String log_dir = AppendFileName(home, ".local/state/u++/guilog");
	RealizeDirectory(log_dir);
	String log_path = AppendFileName(log_dir, title + ".log");
	FileAppend log(log_path);
	
	log << "[" << GetSysTime() << "] Constraint Check Started\n";
	log << "  Facts collected: " << facts.GetCount() << "\n";
	for(int i = 0; i < facts.GetCount(); i++) {
		log << "    " << facts[i] << "\n";
		Log(LOG_FACT, LL_TRACE, facts[i]);
	}
	
	Index<NodeVar> fact_axioms;
	for(int i = 0; i < facts.GetCount(); i++) {
		NodeVar n = Parse(facts[i]);
		if (n.Is()) fact_axioms.Add(n);
	}
	
	for(const String& c : Ctrl::constraints) {
		log << "  Checking constraint: " << c << "\n";
		Log(LOG_CONSTRAINT, LL_INFO, "Checking constraint: " + c);
		NodeVar formula = Parse(c);
		if (formula.Is()) {
			// Suppress printing during constraint checks
			auto saved_print = WhenPrint;
			String* saved_catch = catch_print;
			bool saved_silent = flag_silent_prover;
			
			WhenPrint.Clear();
			catch_print = nullptr;
			flag_silent_prover = true;
			
			bool proven = ProveFormula(fact_axioms, formula);
			
			WhenPrint = saved_print;
			catch_print = saved_catch;
			flag_silent_prover = saved_silent;
			
			if(WhenCheckConstraintsResult) WhenCheckConstraintsResult(c, proven);
			if(proven) {
				log << "    SUCCESS: Constraint satisfied.\n";
				Log(LOG_CONSTRAINT, LL_INFO, "SUCCESS: Constraint satisfied: " + c);
			}
			else {
				log << "    FAILURE: Constraint NOT satisfied!\n";
				Log(LOG_CONSTRAINT, LL_WARN, "FAILURE: Constraint NOT satisfied: " + c);
				GetViolationDisplay().ShowError("Constraint violated: " + c);
			}
		}
	}
	
	log << "Constraint Check Finished\n";
	log.Close();
}

void LinkLogicGui()
{
	Ctrl::WhenCheckConstraints = [] { CheckUGUIConstraints(); };
}

INITBLOCK
{
}

}