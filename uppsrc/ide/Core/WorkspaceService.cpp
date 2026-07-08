#include "Core.h"

bool IdeCoreWorkspace::LoadMainPackage(const String& package)
{
	if(IsNull(package))
		return false;
	main_package = package;
	workspace.Scan(package);
	return true;
}

Index<String> IdeCoreWorkspace::GetNests(bool refresh) const
{
	return GetAllNests(refresh);
}

VectorMap<String, String> IdeCoreWorkspace::ScanAllNests()
{
	VectorMap<String, String> result;
	Index<String> nests = GetNests();
	for(int i = 0; i < nests.GetCount(); i++) {
		String nest = nests[i];
		for(FindFile ff(AppendFileName(nest, "*")); ff; ff.Next()) {
			if(ff.IsFolder()) {
				String folder = ff.GetPath();
				String name = ff.GetName();
				String upp = AppendFileName(folder, name + ".upp");
				String xupp = AppendFileName(folder, name + ".xupp");
				if(FileExists(upp)) {
					result.FindAdd(name, upp);
				}
				else if(FileExists(xupp)) {
					result.FindAdd(name, xupp);
				}
			}
		}
	}
	return result;
}
