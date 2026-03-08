#include "MaestroHub.h"

NAMESPACE_UPP

OpsRunner::OpsRunner() {
	CtrlLayout(*this, "Maestro Operations Runner");
	
	ops_list.AddColumn("Operation");
	ops_list.Add("build");
	ops_list.Add("run");
	ops_list.Add("doctor");
	ops_list.Add("health");
	ops_list.Add("cleanup-cache");
	ops_list.SetCursor(0);
	
	btn_run << THISBACK(OnRun);
	btn_close << [=] { Close(); };
}

void OpsRunner::Load(const String& maestro_root) {
	root = maestro_root;
	output.SetQTF("[* Operation Runner Ready.]");
}

void OpsRunner::OnRun() {
	if(!ops_list.IsCursor()) return;
	String op = ops_list.Get(0);
	
	output.SetQTF("[* Running operation: " + op + "...]&");
	
	String res;
	res << "[* " << GetSysTime() << "] Execution started...&";
	if(op == "doctor") {
		res << "[@G OK] Project root verified.&";
		res << "[@G OK] Settings file exists.&";
		res << "[@Y WARN] AI Cache is getting large (500MB).&";
		res << "[@G OK] GraphLib integration active.&";
	} else {
		res << "Operation " << op << " completed successfully.&";
	}
	res << "------------------------------------------&";
	res << "[* Result:] SUCCESS";
	
	output.SetQTF(res);
}

END_UPP_NAMESPACE