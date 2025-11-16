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

	int c = prj->session_ids.GetCount(); // Get the count of entries in qwen session

	// Add new entries if needed
	for(int i = page.GetCount(); i < c; i++) {
		Entry& e = entries.Add();
		page.Add(e, "Session " + IntStr(i+1)); // Set a default title
		e.SetDocText();
	}

	// Update existing entries
	for(int i = 0; i < min(c, page.GetCount()); i++) {
		page.GetItem(i).Text(prj->session_ids[i]); // Update title with session ID
	}
}

END_UPP_NAMESPACE
