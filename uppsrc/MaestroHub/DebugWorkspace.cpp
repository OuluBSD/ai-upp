#include "MaestroHub.h"

NAMESPACE_UPP

DebugWorkspace::DebugWorkspace() {
	CtrlLayout(*this);
	
	toolbar.Set(THISBACK(OnToolbar));
	
	target_device.Add("Local Machine");
	target_device.Add("ADB: emulator-5554");
	target_device.Add("SSH: remote-builder");
	target_device.SetIndex(0);
	
	left_pane.Add(target_device.TopPos(0, 24).HSizePos());
	left_pane.Add(call_stack.VSizePos(24, 0).HSizePos());
	
	call_stack.Add(0, CtrlImg::Dir(), "Call Stack");
	
	locals.AddColumn("Name");
	locals.AddColumn("Value");
	locals.AddColumn("Type");
	
	bottom_pane.Add(locals.SizePos());
	center_pane.Add(source_code.SizePos());
	
	vsplit.Vert(center_pane, bottom_pane);
	hsplit.Horz(left_pane, vsplit);
	hsplit.SetPos(2500);
	
	main_split.Add(hsplit.SizePos());
}

void DebugWorkspace::OnToolbar(Bar& bar) {
	bar.Add("Run", CtrlImg::go_forward(), THISBACK(OnRun)).Tip("Execute Target");
	bar.Add("Stop", CtrlImg::cross(), THISBACK(OnStop)).Tip("Terminate Execution");
	bar.Separator();
	bar.Add("Step Into", THISBACK(OnStep)).Tip("Step Into (F11)");
}

void DebugWorkspace::Load(const String& maestro_root) {
	source_code.SetQTF("[* Debugging MegaFileUtil...]&Select a target and click Run.");
}

void DebugWorkspace::OnRun() {
	PromptOK("Starting execution on " + target_device.GetData().ToString());
}

void DebugWorkspace::OnStop() {
	PromptOK("Execution stopped.");
}

void DebugWorkspace::OnStep() {
	// Stub
}

END_UPP_NAMESPACE