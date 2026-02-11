#include <CtrlLib/CtrlLib.h>
#include <AI/Logic/TheoremProver.h>

using namespace Upp;
using namespace TheoremProver;

struct TheoremProverCtrl : TopWindow {
	Splitter vsplit;
	DocEdit  input;
	RichTextView output;
	ToolBar  toolbar;
	StatusBar status;
	
	typedef TheoremProverCtrl CLASSNAME;
	
	void RunProof() {
		String code = input.GetData();
		if (code.IsEmpty()) return;
		
		status.Set("Proving...");
		String result = ProveLogic(code);
		output.SetQTF("[C " + DeQtf(result) + "]");
		
		if (result.Find("Formula proven") != -1)
			status.Set("Success: Formula proven", Yellow());
		else
			status.Set("Failure: Formula unprovable", Red());
	}
	
	void AddAxiomGui() {
		String code = input.GetData();
		if (code.IsEmpty()) return;
		
		String err = AddAxiom(code);
		if (err.IsEmpty()) {
			status.Set("Axiom added successfully");
		} else {
			status.Set("Error adding axiom: " + err, Red());
		}
	}
	
	void RefreshAxioms() {
		String ax = GetAxioms();
		output.SetQTF("[B Current Axioms:\n]" + DeQtf(ax));
	}
	
	void ClearAll() {
		ClearLogic();
		output.Clear();
		status.Set("Logic cleared");
	}
	
	void InsertText(String s) {
		input.Insert(s);
		input.SetFocus();
	}
	
	void MainMenu(Bar& bar) {
		bar.Add("Prove", CtrlImg::save(), THISBACK(RunProof)).Key(K_F5).Help("Try to prove the formula in the input box");
		bar.Add("Add Axiom", CtrlImg::plus(), THISBACK(AddAxiomGui)).Key(K_F6).Help("Add the current formula as an axiom");
		bar.Add("List Axioms", CtrlImg::open(), THISBACK(RefreshAxioms)).Key(K_F7).Help("Show all current axioms");
		bar.Separator();
		
		bar.Add("∀", [this]{ InsertText("forall "); }).Help("Insert Universal Quantifier");
		bar.Add("∃", [this]{ InsertText("exists "); }).Help("Insert Existential Quantifier");
		bar.Add("¬", [this]{ InsertText("not "); }).Help("Insert Negation");
		bar.Add("∧", [this]{ InsertText(" and "); }).Help("Insert Conjunction");
		bar.Add("∨", [this]{ InsertText(" or "); }).Help("Insert Disjunction");
		bar.Add("→", [this]{ InsertText(" implies "); }).Help("Insert Implication");
		bar.Add("=", [this]{ InsertText(" = "); }).Help("Insert Equality");
		
		bar.Separator();
		bar.Add("Clear All", CtrlImg::remove(), THISBACK(ClearAll)).Key(K_F8).Help("Reset all axioms and lemmas");
	}

	TheoremProverCtrl() {
		Title("Theorem Prover GUI");
		Sizeable().Zoomable();
		
		LayoutId("Main");
		input.LayoutId("Input");
		output.LayoutId("Output");
		toolbar.LayoutId("Toolbar");
		
		AddFrame(toolbar);
		AddFrame(status);
		toolbar.Set(THISBACK(MainMenu));
		
		vsplit.Vert(input, output);
		vsplit.SetPos(3000);
		
		Add(vsplit.SizePos());
		
		input <<= "forall x. P(x) implies (Q(x) implies P(x))";
		
		WhenPrint = [this](String s) {
			output.SetQTF(output.GetQTF() + DeQtf(s) + "\n");
		};
	}
};

GUI_APP_MAIN {
	TheoremProverCtrl().Run();
}
