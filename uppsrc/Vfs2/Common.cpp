#include "Vfs.h"

NAMESPACE_UPP

String GetRelSrcPath(const String& rel_path) {
	String ext = GetFileExt(rel_path);
	
	if (ext == ".ecs") return rel_path;
	if (ext == ".env") return rel_path;
	if (ext == ".db-src") return rel_path;
	
	return META_FILENAME;
}

END_UPP_NAMESPACE
