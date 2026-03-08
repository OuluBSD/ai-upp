#include "MaestroHub.h"

NAMESPACE_UPP

InitDialog::InitDialog() {
	CtrlLayoutOKCancel(*this, "Initialize Maestro Project");
	
	btn_browse << THISBACK(OnBrowse);
}

void InitDialog::OnBrowse() {
	String path = SelectDirectory();
	if(!path.IsEmpty())
		dir.SetData(path);
}

END_UPP_NAMESPACE