#include "umake.h"

void Ide::ExportMakefile(const String& ep)
{
	SaveMakeFile(AppendFileName(ep, "Makefile"), true);
}

void Ide::ExportProject(const String& ep, bool all, bool deletedir)
{
	::Workspace wspc;
	wspc.Scan(main);
	Index<String> used;
	IdeCoreGatherProjectFiles(wspc, used);

	if(FileExists(ep))
		FileDelete(ep);
	if(deletedir && DirectoryExists(ep))
		DeleteFolderDeep(ep);

	Vector<String> upp = GetUppDirs();
	IdeCoreExportProjectFiles(wspc, ep, used, all, upp, [](int step) { return true; });
	ExportMakefile(ep);
}
