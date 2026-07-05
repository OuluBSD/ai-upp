#include "Core.h"
#include <Core/Core.h>

void IdeCoreGatherProjectFiles(const Workspace& wspc, Index<String>& used)
{
	HdependClearDependencies();
	for(int i = 0; i < wspc.GetCount(); i++) {
		const Package& p = wspc.GetPackage(i);
		String pn = wspc[i];
		for(int j = 0; j < p.GetCount(); j++) {
			const Package::File& f = p[j];
			if(!f.separator) {
				String p = SourcePath(pn, f);
				used.FindAdd(p);
				Vector<String> d = HdependGetDependencies(p);
				for(int q = 0; q < d.GetCount(); q++)
					used.FindAdd(d[q]);
				for(int q = 0; q < f.depends.GetCount(); q++)
					used.FindAdd(SourcePath(pn, f.depends[q].text));
			}
		}
		used.FindAdd(SourcePath(pn, "init"));
	}
}

bool IdeCoreExportProjectFiles(const Workspace& wspc, const String& ep, Index<String>& used, bool all, const Vector<String>& upp, Gate<int> step)
{
	for(int i = 0; i < wspc.GetCount(); i++) {
		if(!step(i))
			return false;
		CopyFolder(AppendFileName(ep, wspc[i]), PackageDirectory(wspc[i]), used, all, true);
	}
	for(int i = 0; i < upp.GetCount(); i++) {
		if(!step(wspc.GetCount() + i))
			return false;
		String d = upp[i];
		FindFile ff(AppendFileName(d, "*"));
		while(ff) {
			if(ff.IsFile()) {
				String fn = ff.GetName();
				String path = AppendFileName(d, fn);
				if(all || used.Find(path) >= 0)
					CopyFile(AppendFileName(ep, fn), path, true);
			}
			ff.Next();
		}
	}
	return true;
}
