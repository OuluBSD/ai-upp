#ifndef _CtrlLib_Mlui_h_
#define _CtrlLib_Mlui_h_

struct MluiSnapshotOptions {
	bool include_hidden = false;
	bool include_layout = true;
	bool include_visible_text = true;
	bool include_visible_text_ratio = true;
};

Value  BuildMluiSnapshot(const MluiSnapshotOptions& options = MluiSnapshotOptions());
String HandleMluiJsonRequest(const String& request_json);
void   StartMluiRuntime();
void   RegisterMluiRuntimeStarter();

#endif
