#include "MaestroHub.h"

NAMESPACE_UPP

DebugWorkspace::DebugWorkspace() {
	CtrlLayout(*this);
	
	toolbar.MaxIconSize(Size(20, 20));
	toolbar.Set(THISBACK(OnToolbar));
	
	target_device.Add("Local Machine");
	target_device.Add("ADB: emulator-5554");
	target_device.Add("SSH: remote-builder");
	target_device.SetIndex(0);
	
	target_binary.NullText("Enter binary path...");
	target_binary.SetData("./crash");
	target_binary.LayoutId("TargetBinary");
	
	call_stack.LayoutId("CallStack");
	
	left_pane.Add(target_device.TopPos(0, 24).HSizePos());
	left_pane.Add(target_binary.TopPos(24, 24).HSizePos());
	left_pane.Add(call_stack.VSizePos(48, 0).HSizePos());
	
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
	bar.Add("Continue", CtrlImg::go_forward(), [=]{ if(dbg) dbg->Cont(); }).Tip("Continue Execution (F5)");
	bar.Add("Stop", CtrlImg::cross(), THISBACK(OnStop)).Tip("Terminate Execution");
	bar.Separator();
	bar.Add("Step Into", THISBACK(OnStep)).Tip("Step Into (F11)");
}

void DebugWorkspace::Load(const String& maestro_root) {
	source_code.SetQTF("[* Debugging MegaFileUtil...]&Select a target and click Run.");
}

void DebugWorkspace::OnRun() {
	if(!dbg) {
		dbg.Create<GdbService>();
		
		dbg->WhenOutput = [=](String s) {
			// Ideally log to a console, here we just show it works
			if(WhenLog) WhenLog(s);
		};
		
		dbg->WhenStack = [=](const Vector<StackFrame>& s) {
			call_stack.Clear();
			int root = call_stack.Add(0, CtrlImg::Dir(), "Call Stack");
			for(const auto& f : s) {
				call_stack.Add(root, CtrlImg::Dir(), f.ToString());
			}
			call_stack.OpenDeep(0);
		};
		
		dbg->WhenLocals = [=](const Vector<Variable>& v) {
			locals.Clear();
			for(const auto& var : v) {
				locals.Add(var.name, var.value, var.type);
			}
		};
	}
	
	dbg->Run(target_binary.GetData().ToString(), "");
}

void DebugWorkspace::OnStop() {
	if(dbg) dbg->Stop();
}

void DebugWorkspace::OnStep() {
	if(dbg) dbg->Step();
}

END_UPP_NAMESPACE
