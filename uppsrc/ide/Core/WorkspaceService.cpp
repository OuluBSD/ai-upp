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
