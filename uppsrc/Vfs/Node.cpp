#include "Vfs.h"

NAMESPACE_UPP

String GetAccessString(CodeAccess acc) {
	switch (acc) {
		case ACC_PRIVATE:	return "private";
		case ACC_PROTECTED:	return "protected";
		case ACC_PUBLIC:	return "public";
		default:			return "invalid";
	}
}


bool NodeBase::IsContained(const CodeArgs2& args) const {
	String pkg = GetHint(HINT_PKG);
	String file = GetHint(HINT_FILE);
	if (pkg.IsEmpty() || file.IsEmpty())
		return false;
	
	return args.pkg->name == pkg && args.file->name == file;
}

bool NodeBase::IsFwdContained(const CodeArgs2& args) const {
	String pkg = GetHint(HINT_FWD_DECL_PKG);
	String file = GetHint(HINT_FWD_DECL_FILE);
	if (pkg.IsEmpty() || file.IsEmpty())
		return false;
	
	return args.pkg->name == pkg && args.file->name == file;
}


END_UPP_NAMESPACE
