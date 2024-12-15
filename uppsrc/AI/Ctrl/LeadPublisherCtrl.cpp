#include "Ctrl.h"

NAMESPACE_UPP


LeadPublisherCtrl::LeadPublisherCtrl() {
	Add(hsplit.SizePos());
	
	hsplit.Horz() << vsplit;
	hsplit.SetPos(1500);
	
	vsplit.Vert() << info << artists;
	
	CtrlLayout(info);
	
	info.name <<= THISBACK(ValueChange);
	info.info <<= THISBACK(ValueChange);
	info.genres <<= THISBACK(ValueChange);
	info.url <<= THISBACK(ValueChange);
	
	artists.AddColumn(t_("Artist"));
}

void LeadPublisherCtrl::Data() {
	LeadDataPublisher& ldp = GetExt<LeadDataPublisher>();
	
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
	bar.Separator();
	bar.Add(t_("Import Json"), THISBACK(ImportJson));
}

void LeadPublisherCtrl::Do(int fn) {
	
}

void LeadPublisherCtrl::ImportJson() {
	DatasetPtrs p = GetDataset();
	if (!p.entity)
		return;
	Entity& ent = *p.entity;
	if (!ent.node.owner)
		return;
	MetaNode& ent_owner = *ent.node.owner;
	
	FileSelNative filesel;
	filesel.ActiveDir(GetHomeDirFile("export"));
	
	if (filesel.ExecuteSelectDir("Select directory containing json files for importing")) {
		String dir = filesel.Get();
		FindFile ff;
		if (ff.Search(AppendFileName(dir, "*.json"))) do {
			String path = ff.GetPath();
			String title = GetFileTitle(path);
			MetaNode& publisher = ent_owner.GetAdd(title, "", METAKIND_ECS_ENTITY);
			MetaNode& pub_comp = publisher.GetAdd("","",METAKIND_ECS_COMPONENT_LEAD_PUBLISHER);
			LeadDataPublisher& pub = pub_comp.GetExt<LeadDataPublisher>();
			LoadFromJsonFile_VisitorNode(pub, path);
		}
		while (ff.Next());
		WhenEditorChange();
	}
}

void LeadPublisherCtrl::ValueChange() {
	DatasetPtrs p = GetDataset();
	LeadDataPublisher& ldp = GetExt<LeadDataPublisher>();
	
	ldp.name = info.name.GetData();
	ldp.info = info.info.GetData();
	ldp.genres = info.genres.GetData();
	ldp.url = info.url.GetData();
}

void LeadPublisherCtrl::PasteArtists() {
	LeadDataPublisher& ldp = GetExt<LeadDataPublisher>();
	ldp.artists.Clear();
	Vector<String> artists = Split(ReadClipboardText(), "\n");
	for (String& s : artists)
		ldp.artists.Add(TrimBoth(s));
	
	PostCallback(THISBACK(Data));
}

INITIALIZER_COMPONENT_CTRL(LeadDataPublisher, LeadPublisherCtrl)


END_UPP_NAMESPACE
