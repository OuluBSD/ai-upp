#include "QwenManager.h"

NAMESPACE_UPP

QwenProjectView::Entry::Entry() {
	
}

void QwenProjectView::Entry::SetDocText(bool view_only) {
	Add(doc.SizePos());
	doc.SetEditable(!view_only);
}

QwenProjectView::QwenProjectView() {
	Splitter::Horz() << page << term;
	
}

void QwenProjectView::Data() {
	if (!prj) return;
	
	int c = 0; // TODO get entries in qwen session
	
	for(int i = page.GetCount(); i < c; i++) {
		
		// TODO update new entries
		/*
		Entry& e = entries.Add();
		page.Add(e, "<title>");
		*/
	}
	
}

END_UPP_NAMESPACE
