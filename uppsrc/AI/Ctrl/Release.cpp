#include "Ctrl.h"

NAMESPACE_UPP


SnapInfoCtrl::SnapInfoCtrl() {
	CtrlLayout(*this);
	
	native_album_title <<= THISBACK(OnValueChange);
	english_album_title <<= THISBACK(OnValueChange);
	album_date <<= THISBACK(OnValueChange);
	year_of_content <<= THISBACK(OnValueChange);
	
}

void SnapInfoCtrl::Clear() {
	this->native_album_title		.Clear();
	this->english_album_title		.Clear();
	this->album_date				.Clear();
	this->year_of_content			.Clear();
}

void SnapInfoCtrl::Data() {
	DatasetPtrs p = GetDataset();
	
	lbl_snapshot.SetLabel("Release");
	
	Clear();
	
	if (p.release) {
		Snapshot& r = *p.release;
		
		native_album_title.SetData(r.native_title);
		english_album_title.SetData(r.english_title);
		album_date.SetData(r.date);
		year_of_content.SetData(r.year_of_content);
	}
	
}

void SnapInfoCtrl::OnValueChange() {
	DatasetPtrs p = GetDataset();
	
	TODO
	#if 0
	if (p.release && p.editor && p.editor->snaps.IsCursor()) {
		Snapshot& r = *p.release;
		
		r.native_title = native_album_title.GetData();
		r.english_title = english_album_title.GetData();
		r.date = album_date.GetData();
		r.year_of_content = year_of_content.GetData();
		
		int c = p.editor->snaps.GetCursor();
		p.editor->snaps.Set(c, 0, r.native_title);
	}
	#endif
}


END_UPP_NAMESPACE
