#include "ConstraintVisitor.h"
#include <AI/Logic/TheoremProver.h>
#include <CtrlLib/CtrlLib.h>
#include <ByteVM/ByteVM.h>

namespace Upp {

using namespace TheoremProver;

Event<String, bool> WhenCheckConstraintsResult;

void CheckUGUIConstraints();

static PyValue builtin_check_constraints(const Vector<PyValue>& args, void*) {
	CheckUGUIConstraints();
	return PyValue::None();
}

void RegisterConstraintBindings(PyVM& vm) {
	auto& globals = vm.GetGlobals();
	globals.GetAdd(PyValue("check_constraints")) = PyValue::Function("check_constraints", builtin_check_constraints);
}

struct ViolationDisplay : TopWindow {
	StaticRect back;
	Label msg;
	TimeCallback timer;
	
	ViolationDisplay() {
		this->SetRect(0, 0, 400, 40);
		back.Color(Red());
		this->Add(back.SizePos());
		back.Add(msg.SizePos());
		msg.SetAlign(ALIGN_CENTER);
		msg.SetFont(StdFont().Bold());
		msg.SetInk(White());
		this->TopMost();
		this->Title("UGUI Constraint Violation");
	}
	
	void ShowError(const String& s) {
		msg.SetText(s);
		if(!this->IsOpen()) {
			Rect r = GetWorkArea();
			this->SetRect(r.left + 10, r.top + 10, 400, 40);
			this->Open();
		}
		timer.KillSet(3000, [this] { this->Close(); });
	}
};

ViolationDisplay& GetViolationDisplay() {
	static ViolationDisplay d;
	return d;
}

void CheckUGUIConstraints()
{
	GuiLock __;
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

void RegisterAutomationBindings(PyVM& vm);

void LinkAutomationScript()
{
	const Vector<String>& cmd = CommandLine();
	for(int i = 0; i < cmd.GetCount(); i++) {
		if (cmd[i] == "--script" && i + 1 < cmd.GetCount()) {
			String path = cmd[i + 1];
			String source = LoadFile(path);
			if (!source.IsEmpty()) {
				Thread().Run([source, path] {
					Cout() << "Automation: Loading script " << path << "\n";
					
					try {
						PyVM vm;
						RegisterAutomationBindings(vm);
						RegisterConstraintBindings(vm);
						
						Tokenizer tokenizer;
						tokenizer.SkipPythonComments(true);
						if (!tokenizer.Process(source, path)) {
							Cerr() << "Automation Error: Tokenization failed\n";
							return;
						}
						tokenizer.CombineTokens();
						
						PyCompiler compiler(tokenizer.GetTokens());
						Vector<PyIR> ir;
						compiler.Compile(ir);
						
						vm.SetIR(ir);
						vm.Run();
					} catch (const Exc& e) {
						if (e.Find("EXIT:0") < 0)
							Cerr() << "Automation script error: " << e << "\n";
					}
					
					Cout() << "Automation script finished.\n";
				});
				return;
			}
		}
	}
}

void LinkLogicGui()
{
	InstallPanicMessageBox([](const char* title, const char* text) {
		LOG("[PANIC] " << title << ": " << text);
	});
	Ctrl::WhenCheckConstraints = [] { CheckUGUIConstraints(); };
	LinkAutomationScript();
}

INITBLOCK
{
}

}