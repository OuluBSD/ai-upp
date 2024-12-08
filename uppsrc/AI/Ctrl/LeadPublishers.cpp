#include "Ctrl.h"

NAMESPACE_UPP


LeadPublishers::LeadPublishers() {
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

void LeadPublishers::Data() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& p = MetaPtrs::Single();
	LeadDataTemplate& ldt = LeadDataTemplate::Single();
	
	
	for(int i = 0; i < ldt.publishers.GetCount(); i++) {
		LeadDataPublisher& ldp = ldt.publishers[i];
		list.Set(i, 0, ldp.name);
	}
	list.SetCount(ldt.publishers.GetCount());
	
}

void LeadPublishers::DataItem() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& p = MetaPtrs::Single();
	LeadDataTemplate& ldt = LeadDataTemplate::Single();
	
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

void LeadPublishers::ToolMenu(Bar& bar) {
	bar.Add(t_("Paste artist list"), AppImg::VioletRing(), THISBACK(PasteArtists));
}

void LeadPublishers::ListMenu(Bar& bar) {
	bar.Add(t_("Add"), AppImg::BlueRing(), THISBACK(AddPublisher)).Key(K_CTRL_N);
	if (list.IsCursor())
		bar.Add(t_("Remove"), AppImg::BlueRing(), THISBACK(RemovePublisher)).Key(K_CTRL|K_SHIFT|K_W);
}

void LeadPublishers::Do(int fn) {
	
}

void LeadPublishers::AddPublisher() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& p = MetaPtrs::Single();
	LeadDataTemplate& ldt = LeadDataTemplate::Single();
	
	LeadDataPublisher& ldp = ldt.publishers.Add();
	
	
	
	Data();
	list.SetCursor(list.GetCount()-1);
}

void LeadPublishers::RemovePublisher() {
	if (!list.IsCursor())
		return;
	
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& p = MetaPtrs::Single();
	LeadDataTemplate& ldt = LeadDataTemplate::Single();
	
	int idx = list.GetCursor();
	ldt.publishers.Remove(idx);
	
	Data();
}

void LeadPublishers::ValueChange() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& p = MetaPtrs::Single();
	LeadDataTemplate& ldt = LeadDataTemplate::Single();
	
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

void LeadPublishers::PasteArtists() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& p = MetaPtrs::Single();
	LeadDataTemplate& ldt = LeadDataTemplate::Single();
	
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


END_UPP_NAMESPACE
