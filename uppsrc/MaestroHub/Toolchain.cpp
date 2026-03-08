#include "MaestroHub.h"

NAMESPACE_UPP

void ToolchainManager::Load(const String& dir) {
	for(FindFile ff(AppendFileName(dir, "*.var")); ff; ff.Next()) {
		String name = GetFileTitle(ff.GetName());
		if(Find(name)) continue; // Already loaded
		
		Toolchain& tc = toolchains.Add();
		tc.name = name;
		tc.path = ff.GetPath();
		
		FileIn in(tc.path);
		while(!in.IsEof()) {
			String line = in.GetLine();
			int q = line.Find('=');
			if(q >= 0) {
				String key = TrimBoth(line.Left(q));
				String val = TrimBoth(line.Mid(q+1));
				tc.vars.Add(key, val);
			}
		}
	}
}

void ToolchainManager::ScanStandardLocations() {
	Load(GetHomeDirectory());
	Load(ConfigFile("")); 
	Load(GetConfigFolder());
	
#ifdef PLATFORM_POSIX
	Load(GetHomeDirectory() + "/.upp/build_methods");
	Load(GetHomeDirectory() + "/.config/u++/build_methods");
#endif
}

Toolchain* ToolchainManager::Find(const String& name) {
	for(int i=0; i<toolchains.GetCount(); i++) {
		if(toolchains[i].name == name) return &toolchains[i];
	}
	return NULL;
}

BuildMethodsDialog::BuildMethodsDialog() {
	Title("Build Methods");
	Sizeable().Zoomable();
	
	list.AddColumn("Method");
	vars.AddColumn("Variable");
	vars.AddColumn("Value");
	
	split.Horz(list, vars);
	split.SetPos(3000);
	
	Add(split.SizePos());
	
	list.WhenCursor = THISBACK(OnSelect);
	
	Load();
}

void BuildMethodsDialog::Load() {
	tm.ScanStandardLocations();
	list.Clear();
	for(const auto& tc : tm.toolchains) {
		list.Add(tc.name);
	}
	if(list.GetCount() > 0) list.SetCursor(0);
}

void BuildMethodsDialog::OnSelect() {
	vars.Clear();
	if(!list.IsCursor()) return;
	
	int idx = list.GetCursor();
	const Toolchain& tc = tm.toolchains[idx];
	
	for(int i=0; i<tc.vars.GetCount(); i++) {
		vars.Add(tc.vars.GetKey(i), tc.vars[i]);
	}
}

END_UPP_NAMESPACE