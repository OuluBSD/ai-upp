#include "InitDialog.h"

NAMESPACE_UPP

InitDialog::InitDialog() {
	CtrlLayout(*this, "Initialize Maestro Project");
	
	btn_browse << THISBACK(OnBrowse);
	ok << [=] { Break(IDOK); };
	cancel << [=] { Break(IDCANCEL); };
}

void InitDialog::OnBrowse() {
	String path = SelectDirectory();
	if(!path.IsEmpty())
		dir.SetData(path);
}

END_UPP_NAMESPACE
