#include "Ctrl.h"

NAMESPACE_UPP


BiographyPlatformCtrl::BiographyPlatformCtrl() {
	Add(tabs.VSizePos(0,20).HSizePos());
	
	Platforms_Ctor();
	Clusters_Ctor();
	Audience_Ctor();
	
	tabs.WhenSet << THISBACK(Data);
}

void BiographyPlatformCtrl::Data() {
	int tab = tabs.Get();
	
	if      (tab == 0)	Platforms_Data();
	else if (tab == 1)	Clusters_Data();
	else if (tab == 2)	Audience_Data();
}

void BiographyPlatformCtrl::ToolMenu(Bar& bar) {
	int tab = tabs.Get();
	
	if      (tab == 0)	Platforms_ToolMenu(bar);
	else if (tab == 1)	Clusters_ToolMenu(bar);
	else if (tab == 2)	Audience_ToolMenu(bar);
}

void BiographyPlatformCtrl::Do(int fn) {
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.release)
		return;
	String dir = GetFileDirectory(GetFilePath());
	PlatformProfileProcess& ss = PlatformProfileProcess::Get(mp, dir);
	if (fn == 0) {
		ss.Start();
	}
	else if (fn == 1) {
		ss.Stop();
	}
}

void BiographyPlatformCtrl::SetSorting(int col) {
	
}

void BiographyPlatformCtrl::ImportJson() {
	DatasetPtrs p = GetDataset();
	BiographyPlatform& data = GetExt<BiographyPlatform>();
	if (LoadFromJsonFile_VisitorNodePrompt(data)) {
		PostCallback(THISBACK(Data));
	}
}


INITIALIZER_COMPONENT_CTRL(BiographyPlatform, BiographyPlatformCtrl)

END_UPP_NAMESPACE
