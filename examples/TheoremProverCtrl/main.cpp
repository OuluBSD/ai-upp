#include <CtrlLib/CtrlLib.h>
#include <AI/Logic/TheoremProver.h>
#include <Ctrl/Automation/Automation.h>
#include <AI/Core/Core.h>
#include <AI/LogicGui/LogicGui.h>

using namespace Upp;
using namespace TheoremProver;

struct TheoremProverCtrl : TopWindow {
	Splitter vsplit, hsplit;
	DocEdit  input;
	RichTextView output;
	TreeCtrl proof_tree;
	ToolBar  toolbar;
	StatusBar status;
	String   captured_qtf;
	String   last_status;
	
	typedef TheoremProverCtrl CLASSNAME;
	
	void SetStatus(const String& s) {
		last_status = s;
		status.Set(s);
	}

	virtual bool Access(Visitor& v) override {
		if (auto *av = dynamic_cast<AutomationVisitor*>(&v)) {
			// Write mode
			if (av->write_mode) {
				if (av->target_path == "Main/input_val" || av->target_path == "input_val") {
					input.SetData(av->target_value);
					av->found = true;
				}
			}

			// Read mode
			AutomationElement& el_in = av->elements.Add();
			el_in.path = "Main/input_val";
			el_in.text = "input_val";
			el_in.value = input.GetData();
			el_in.visible = true;
			el_in.enabled = true;
			
			AutomationElement& el_out = av->elements.Add();
			el_out.path = "Main/output_val";
			el_out.text = "output_val";
			el_out.value = output.GetQTF();
			el_out.visible = true;
			el_out.enabled = true;

			AutomationElement& el_status = av->elements.Add();
			el_status.path = "Main/status_val";
			el_status.text = "status_val";
			el_status.value = last_status;
			el_status.visible = true;
			el_status.enabled = true;
		}
		return TopWindow::Access(v);
	}
	
	void RunProof() {
		String code = input.GetData();
		if (code.IsEmpty()) return;
		
		output.Clear();
		captured_qtf.Clear();
		SetStatus("Proving...");
		
		// Setup WhenPrint before calling ProveLogic
		WhenPrint = [this](String s) {
			LOG("WhenPrint callback: " << s);
			captured_qtf << DeQtf(s) << "&";
			output.SetQTF(captured_qtf);
		};
		
		String result = ProveLogic(code);
		LOG("RunProof result: " << result);
		
		if (result.Find("Formula proven") != -1)
			SetStatus("Success: Formula proven");
		else
			SetStatus("Failure: Formula unprovable");
		
		// If captured_qtf is still empty but result has something, use it
		if (captured_qtf.IsEmpty() && result.GetCount()) {
			captured_qtf = "[C " + DeQtf(result) + "]";
			output.SetQTF(captured_qtf);
		}
		
		Ctrl::CheckConstraints();
		
		UpdateProofTree();
	}
	
	void UpdateProofTree() {
		proof_tree.Clear();
		proof_tree.SetRoot(CtrlImg::plus(), "Proof Tree (Sequential)");
		for(const String& s : current_proof_steps) {
			proof_tree.Add(0, CtrlImg::File(), s);
		}
		proof_tree.OpenDeep(0);
	}
	
	void AddAxiomGui() {
		String code = input.GetData();
		if (code.IsEmpty()) return;
		
		String err = AddAxiom(code);
		if (err.IsEmpty()) {
			SetStatus("Axiom added successfully");
		} else {
			SetStatus("Error adding axiom: " + err);
		}
	}
	
	void RefreshAxioms() {
		LOG("Refreshing Axioms UI");
		output.Clear();
		captured_qtf.Clear();
		captured_qtf = "[B Current Axioms:&]";
		
		WhenPrint = [this](String s) {
			LOG("WhenPrint (Axioms): " << s);
			captured_qtf << DeQtf(s) << "&";
			output.SetQTF(captured_qtf);
		};
		
		GetAxioms();
		SetStatus("Axioms listed");
	}
	
	void ClearAll() {
		ClearLogic();
		output.Clear();
		proof_tree.Clear();
		SetStatus("Logic cleared");
	}
	
	void InsertText(String s) {
		input.Insert(input.GetCursor(), s);
		input.SetFocus();
	}
	
	void RunAutomation(String script_path) {
		String content = LoadFile(script_path);
		if(content.IsVoid()) return;

		TheoremProverCtrl *hub_ptr = this;
		Thread().Run([=] {
			try {
				Sleep(2000); // Wait for GUI thread to pump
				LOG("Starting Automation Script: " + script_path);
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
				LOG("Automation script finished successfully");
				_exit(0);
			} catch (Exc& e) {
				if(e.StartsWith("EXIT:")) {
					int code = atoi(e.Mid(5));
					LOG("Automation script exited with code: " + AsString(code));
					_exit(code);
				} else {
					LOG("Automation Error: " + e);
					_exit(1);
				}
			}
		});
	}
	
	void ExitApp() {
		Break();
	}
	
	void MainMenu(Bar& bar) {
		bar.Add("Prove", CtrlImg::save(), THISBACK(RunProof)).Key(K_F5).Help("Try to prove the formula in the input box").LayoutId("Prove");
		bar.Add("Add Axiom", CtrlImg::plus(), THISBACK(AddAxiomGui)).Key(K_F6).Help("Add the current formula as an axiom").LayoutId("Add Axiom");
		bar.Add("List Axioms", CtrlImg::open(), THISBACK(RefreshAxioms)).Key(K_F7).Help("Show all current axioms").LayoutId("List Axioms");
		bar.Add("Check Constraints", CtrlImg::save_as(), [] { Ctrl::CheckConstraints(); }).Help("Trigger manual UGUI constraint check").LayoutId("Check Constraints");
		bar.Separator();
		
		bar.Add("∀", [this]{ InsertText("forall "); }).Help("Insert Universal Quantifier").LayoutId("Forall");
		bar.Add("∃", [this]{ InsertText("exists "); }).Help("Insert Existential Quantifier").LayoutId("Exists");
		bar.Add("¬", [this]{ InsertText("not "); }).Help("Insert Negation").LayoutId("Not");
		bar.Add("∧", [this]{ InsertText(" and "); }).Help("Insert Conjunction").LayoutId("And");
		bar.Add("∨", [this]{ InsertText(" or "); }).Help("Insert Disjunction").LayoutId("Or");
		bar.Add("→", [this]{ InsertText(" implies "); }).Help("Insert Implication").LayoutId("Implies");
		bar.Add("=", [this]{ InsertText(" = "); }).Help("Insert Equality").LayoutId("Equal");
		
		bar.Separator();
		bar.Add("Clear All", CtrlImg::remove(), THISBACK(ClearAll)).Key(K_F8).Help("Reset all axioms and lemmas").LayoutId("Clear All");
	}

	TheoremProverCtrl() {
		Title("Theorem Prover GUI");
		Sizeable().Zoomable();
		
		WhenPrint = [this](String s) {
			LOG("WhenPrint: " << s);
			captured_qtf << DeQtf(s) << "&";
			output.SetQTF(captured_qtf);
		};
		
		WhenCheckConstraintsResult = [this](String c, bool proven) {
			if(!proven) SetStatus("Constraint FAILED: " + c);
		};

		LayoutId("Main");
		input.LayoutId("Input");
		output.LayoutId("Output");
		proof_tree.LayoutId("ProofTree");
		toolbar.LayoutId("Toolbar");
		status.LayoutId("Status");
		
		AddFrame(toolbar);
		AddFrame(status);
		toolbar.Set(THISBACK(MainMenu));
		
		hsplit.Horz(proof_tree, output);
		hsplit.SetPos(2000);
		hsplit.LayoutId("HSplit");
		
		vsplit.Vert(input, hsplit);
		vsplit.SetPos(3000);
		vsplit.LayoutId("VSplit");
		
		Add(vsplit.SizePos());
		
		input <<= "forall x. P(x) implies (Q(x) implies P(x))";
		
		PostCallback([] { 
			Ctrl::CheckConstraints(); 
		});
	}
};

GUI_APP_MAIN {

	if (CommandLine().GetCount() > 0) {

		InstallPanicMessageBox([](const char *title, const char *text) {

			LOG("PANIC: " << title << ": " << text);

			_exit(1);

		});

	}



	

	LinkLogicGui();

	TheoremProverCtrl hub;

	if (CommandLine().GetCount() > 0) {

		hub.RunAutomation(CommandLine()[0]);

	}

	hub.Run();

}
