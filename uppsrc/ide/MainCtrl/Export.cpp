#ifdef flagGUI
#include "MainCtrl.h"

void Ide::ExportMakefile(const String& ep)
{
	SaveMakeFile(AppendFileName(ep, "Makefile"), true);
}

void Ide::ExportProject(const String& ep, bool all, bool gui, bool deletedir)
{
	SaveFile(false);
	::Workspace wspc;
	wspc.Scan(main);
	Index<String> used;
	IdeCoreGatherProjectFiles(wspc, used);

	if(FileExists(ep)) {
		if(gui && !PromptYesNo(DeQtf(ep) + " is existing file.&"
		                "Do you want to delete it?")) return;
		FileDelete(ep);
	}
	if(deletedir && DirectoryExists(ep)) {
		if(gui && !PromptYesNo(DeQtf(ep) + " is existing directory.&"
		                "Do you want to replace it?")) return;
		DeleteFolderDeep(ep);
	}

	Progress pi("Exporting project");
	Vector<String> upp = GetUppDirs();
	pi.SetTotal(wspc.GetCount() + upp.GetCount());
	
	if(!IdeCoreExportProjectFiles(wspc, ep, used, all, upp, [&](int step) {
		return !(gui && pi.StepCanceled());
	})) {
		return;
	}
	ExportMakefile(ep);
}
#endif // flagGUI
