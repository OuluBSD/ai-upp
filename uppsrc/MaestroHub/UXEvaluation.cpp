#include "MaestroHub.h"

NAMESPACE_UPP

UXEvaluationFactory::UXEvaluationFactory() {
	Title("UX Evaluation Factory");
	Sizeable().Zoomable();
	
	test_list.AddColumn("Test Name");
	test_list.AddColumn("Status");
	test_list.WhenCursor = THISBACK(OnTestCursor);
	
	images.Add(baseline_view.LeftPos(10, 300).VSizePos(10, 10));
	images.Add(current_view.LeftPos(320, 300).VSizePos(10, 10));
	images.Add(diff_view.LeftPos(630, 300).VSizePos(10, 10));
	
	run_test.SetLabel("Run Test");
	run_test << THISBACK(OnRunTest);
	
	approve.SetLabel("Approve Difference");
	approve << THISBACK(OnApprove);
	
	split.Horz(test_list, images);
	split.SetPos(2500);
	
	Add(split.VSizePos(0, 40).HSizePos());
	Add(run_test.BottomPos(5, 30).LeftPos(5, 100));
	Add(approve.BottomPos(5, 30).RightPos(5, 150));
}

void UXEvaluationFactory::Load(const String& maestro_root) {
	root = maestro_root;
	test_list.Clear();
	// Stub load
	test_list.Add("Main Screen Layout", "Passed");
	test_list.Add("Settings Dialog", "Failed (Diff > 5%)");
	
	if(test_list.GetCount() > 0) test_list.SetCursor(0);
}

void UXEvaluationFactory::OnRunTest() {
	PromptOK("Test execution stubbed. In a real scenario, this would launch the app and capture screens.");
}

void UXEvaluationFactory::OnApprove() {
	if(!test_list.IsCursor()) return;
	PromptOK("Baseline updated.");
}

void UXEvaluationFactory::OnTestCursor() {
	// Stub image loading with standard icons for demo
	baseline_view.SetImage(CtrlImg::save()); 
	current_view.SetImage(CtrlImg::save_as());
	diff_view.SetImage(CtrlImg::exclamation()); 
}

END_UPP_NAMESPACE
