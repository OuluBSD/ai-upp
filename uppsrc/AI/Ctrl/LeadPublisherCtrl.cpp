#include "Ctrl.h"

NAMESPACE_UPP


LeadPublisherCtrl::LeadPublisherCtrl() {
	Add(hsplit.SizePos());
	
	hsplit.Horz() << list << vsplit;
	hsplit.SetPos(1500);
	
	vsplit.Vert() << info << artists;
	
	CtrlLayout(info);
	
	list.AddColumn(t_("Name"));
	list.WhenBar << THISBACK(ListMenu);
	list.WhenCursor << THISBACK(DataItem);
	
	info.name <<= THISBACK(ValueChange);
	info.info <<= THISBACK(ValueChange);
	info.genres <<= THISBACK(ValueChange);
	info.url <<= THISBACK(ValueChange);
	
	artists.AddColumn(t_("Artist"));
}

void LeadPublisherCtrl::Data() {
	DatasetPtrs p = GetDataset();
	LeadDataTemplate& ldt = *p.lead_tmpl;
	
	for(int i = 0; i < ldt.publishers.GetCount(); i++) {
		LeadDataPublisher& ldp = ldt.publishers[i];
		list.Set(i, 0, ldp.name);
	}
	list.SetCount(ldt.publishers.GetCount());
	
}

void LeadPublisherCtrl::DataItem() {
	DatasetPtrs p = GetDataset();
	LeadDataTemplate& ldt = *p.lead_tmpl;
	
	if (!list.IsCursor())
		return;
	
	int idx = list.GetCursor();
	LeadDataPublisher& ldp = ldt.publishers[idx];
	
	info.name.SetData(ldp.name);
	info.info.SetData(ldp.info);
	info.genres.SetData(ldp.genres);
	info.url.SetData(ldp.url);
	
	for(int i = 0; i < ldp.artists.GetCount(); i++) {
		artists.Set(i, 0, ldp.artists[i]);
	}
	artists.SetCount(ldp.artists.GetCount());
	
}

void LeadPublisherCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Paste artist list"), TextImgs::VioletRing(), THISBACK(PasteArtists));
}

void LeadPublisherCtrl::ListMenu(Bar& bar) {
	bar.Add(t_("Add"), TextImgs::BlueRing(), THISBACK(AddPublisher)).Key(K_CTRL_N);
	if (list.IsCursor())
		bar.Add(t_("Remove"), TextImgs::BlueRing(), THISBACK(RemovePublisher)).Key(K_CTRL|K_SHIFT|K_W);
}

void LeadPublisherCtrl::Do(int fn) {
	
}

void LeadPublisherCtrl::AddPublisher() {
	DatasetPtrs p = GetDataset();
	LeadDataTemplate& ldt = *p.lead_tmpl;
	
	LeadDataPublisher& ldp = ldt.publishers.Add();
	
	
	
	Data();
	list.SetCursor(list.GetCount()-1);
}

void LeadPublisherCtrl::RemovePublisher() {
	if (!list.IsCursor())
		return;
	
	
	DatasetPtrs p = GetDataset();
	LeadDataTemplate& ldt = *p.lead_tmpl;
	
	int idx = list.GetCursor();
	ldt.publishers.Remove(idx);
	
	Data();
}

void LeadPublisherCtrl::ValueChange() {
	DatasetPtrs p = GetDataset();
	LeadDataTemplate& ldt = *p.lead_tmpl;
	
	if (!list.IsCursor())
		return;
	
	int idx = list.GetCursor();
	LeadDataPublisher& ldp = ldt.publishers[idx];
	
	ldp.name = info.name.GetData();
	ldp.info = info.info.GetData();
	ldp.genres = info.genres.GetData();
	ldp.url = info.url.GetData();
	
	list.Set(idx, 0, ldp.name);
}

void LeadPublisherCtrl::PasteArtists() {
	DatasetPtrs p = GetDataset();
	LeadDataTemplate& ldt = *p.lead_tmpl;
	
	if (!list.IsCursor())
		return;
	
	int idx = list.GetCursor();
	LeadDataPublisher& ldp = ldt.publishers[idx];
	ldp.artists.Clear();
	Vector<String> artists = Split(ReadClipboardText(), "\n");
	for (String& s : artists)
		ldp.artists.Add(TrimBoth(s));
	
	DataItem();
}

INITIALIZER_COMPONENT(LeadPublisher);
INITIALIZER_COMPONENT_CTRL(LeadPublisher, LeadPublisherCtrl)


END_UPP_NAMESPACE
