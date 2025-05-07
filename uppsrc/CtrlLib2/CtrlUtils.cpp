#include "CtrlLib.h"

NAMESPACE_UPP
#if IS_UPP_CORE
FileSel& GFileSel();
#else
FileSel& GFileSel() {static FileSel s; return s;}
#endif
END_UPP_NAMESPACE


NAMESPACE_UPP

void SetFileDialogDirectory(String path) {
	FileSel& fs = GFileSel();
	LoadFromGlobal(fs, "GlobalFileSelector");
	fs.ActiveDir(path);
	StoreToGlobal(fs, "GlobalFileSelector");
}

END_UPP_NAMESPACE
