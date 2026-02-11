#include <CtrlLib/CtrlLib.h>
#include <AI/Logic/TheoremProver.h>
#include <Ctrl/Automation/Automation.h>
#include <AI/Core/Core.h>

using namespace Upp;
using namespace TheoremProver;

struct TheoremProverCtrl : TopWindow {
	Splitter vsplit, hsplit;
	DocEdit  input;
	RichTextView output;
	TreeCtrl proof_tree;
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
			status.Set("Success: Formula proven");
		else
			status.Set("Failure: Formula unprovable");
		
		UpdateProofTree();
	}
	
	void UpdateProofTree() {
		proof_tree.Clear();
		proof_tree.SetRoot(0, "Proof Tree (Sequential)");
	}
	
	void AddAxiomGui() {
		String code = input.GetData();
		if (code.IsEmpty()) return;
		
		String err = AddAxiom(code);
		if (err.IsEmpty()) {
			status.Set("Axiom added successfully");
		} else {
			status.Set("Error adding axiom: " + err);
		}
	}
	
	void RefreshAxioms() {
		String ax = GetAxioms();
		output.SetQTF("[B Current Axioms:\n]" + DeQtf(ax));
	}
	
	void ClearAll() {
		ClearLogic();
		output.Clear();
		proof_tree.Clear();
		status.Set("Logic cleared");
	}
	
	void InsertText(String s) {
		input.Insert(input.GetCursor(), s);
		input.SetFocus();
	}
	
	void RunAutomation(String script_path) {
		String content = LoadFile(script_path);
		if(content.IsVoid()) return;

		Thread().Run([=] {
			try {
				PyVM vm;
				RegisterAutomationBindings(vm);
				
				Tokenizer tk;
				tk.SkipComments();
				tk.SkipPythonComments();
				if(!tk.Process(content, script_path)) return;
				tk.NewlineToEndStatement();
				tk.CombineTokens();

				PyCompiler compiler(tk.GetTokens());
				Vector<PyIR> ir;
				compiler.Compile(ir);

				vm.SetIR(ir);
				vm.Run();
			} catch (Exc& e) {
				LOG("Automation Error: " + e);
			}
		});
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
		proof_tree.LayoutId("ProofTree");
		toolbar.LayoutId("Toolbar");
		
		AddFrame(toolbar);
		AddFrame(status);
		toolbar.Set(THISBACK(MainMenu));
		
		hsplit.Horz(proof_tree, output);
		hsplit.SetPos(2000);
		
		vsplit.Vert(input, hsplit);
		vsplit.SetPos(3000);
		
		Add(vsplit.SizePos());
		
		input <<= "forall x. P(x) implies (Q(x) implies P(x))";
		
		WhenPrint = [this](String s) {
			output.SetQTF(output.GetQTF() + DeQtf(s) + "\n");
		};
	}
};

GUI_APP_MAIN {
	TheoremProverCtrl hub;
	if (CommandLine().GetCount() > 0) {
		hub.RunAutomation(CommandLine()[0]);
	}
	hub.Run();
}