#include "MaestroHub.h"

NAMESPACE_UPP

ConversionPane::ConversionPane() {
	CtrlLayout(*this);
	
	toolbar.MaxIconSize(Size(20, 20));
	toolbar.Set(THISBACK(OnToolbar));
	
	transformation_tree.AddColumn("Unit", 200);
	transformation_tree.AddColumn("Status", 100);
	transformation_tree.WhenCursor = THISBACK(OnTreeCursor);
	
	diff_pane.SetQTF("[* Select a transformation unit to view changes]");
	ai_rationale.SetQTF("[* AI planning details will appear here]");
	
	workspace_tabs.Add(diff_pane.SizePos(), "Source/Target Diff");
	workspace_tabs.Add(ai_rationale.SizePos(), "AI Rationale & Plan");
	
	center_split.Vert(workspace_tabs, progress_log);
	center_split.SetPos(7000);
	
	main_split.Horz(transformation_tree, center_split);
	main_split.SetPos(3000);
}

void ConversionPane::OnToolbar(Bar& bar) {
	bar.Add("Inventory", CtrlImg::Dir(), THISBACK(OnInventory)).Tip("Generate Conversion Inventory");
	bar.Add("Plan", CtrlImg::save(), THISBACK(OnPlan)).Tip("Generate Transformation Plan");
	bar.Separator();
	bar.Add("Start Factory", CtrlImg::go_forward(), THISBACK(OnRun)).Tip("Run Conversion Pipeline");
	bar.Separator();
	bar.Add("Validate", CtrlImg::reporticon(), THISBACK(OnValidate)).Tip("Run Semantic Integrity Checks");
}

void ConversionPane::Load(const String& maestro_root) {
	root = maestro_root;
	Log("Ready.", Blue());
	
	String plan_path = AppendFileName(root, ".maestro/convert/plan/plan.json");
	if(FileExists(plan_path)) {
		if(LoadFromJsonFile(plan, plan_path)) {
			Log("Loaded existing plan: " + plan.id, LtGreen());
			
			transformation_tree.Clear();
			int r = transformation_tree.Add(0, CtrlImg::Dir(), "Transformation Units");
			for(const auto& ph : plan.phases) {
				int p = transformation_tree.Add(r, CtrlImg::Dir(), ph.name);
				for(const auto& tk : ph.tasks) {
					int t = transformation_tree.Add(p, CtrlImg::File(), tk.title);
					transformation_tree.SetRowValue(t, 1, tk.status);
				}
			}
			transformation_tree.Open(r);
		}
	}
}

void ConversionPane::OnInventory() {
	Log("Scanning repository for conversion inventory...", Blue());
	// In reality, we'd prompt for source/target
	ConversionOrchestrator::Inventory(root, AppendFileName(root, "out"));
	Log("Inventory generated successfully.", LtGreen());
}

void ConversionPane::OnPlan() {
	Log("Generating transformation plan...", Blue());
	ConversionOrchestrator::Plan(root, AppendFileName(root, "out"));
	Load(root);
	Log("Plan generated and loaded.", LtGreen());
}

void ConversionPane::OnRun() {
	Log("Starting batch conversion...", LtRed());
	// This should be async in a real app
	ConversionOrchestrator::Run(root, AppendFileName(root, "out"), 0);
	Load(root);
	Log("Batch conversion finished.", LtGreen());
}

void ConversionPane::OnValidate() {
	Log("Running semantic integrity validation...", Blue());
	ConversionOrchestrator::Validate(root, AppendFileName(root, "out"));
	Log("Validation complete.", LtGreen());
}

void ConversionPane::OnTreeCursor() {
	// Update diff view and rationale based on selected task
}

void ConversionPane::Log(const String& txt, Color clr) {
	String entry;
	entry << "[ " << Format(GetSysTime()) << " ] " << "[C" << FormatInt(clr.GetRaw()) << " " << DeQtf(txt) << "]&\n";
	progress_log.SetQTF(BodyAsQTF(progress_log.Get()) + entry);
}

END_UPP_NAMESPACE
