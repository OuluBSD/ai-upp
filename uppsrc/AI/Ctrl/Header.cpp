#include "Ctrl.h"
#define PREF(obj) auto& obj = p.obj;
#define REF(obj) auto& obj = p.header.obj;

NAMESPACE_UPP

#error TODO remove Platforms_Header_ etc.

void BiographyPlatformCtrl::Platforms_Header_Ctor() {
	REF(vsplit);
	REF(entries);
	REF(entry_split);
	REF(attr_keys);
	REF(attr_value);
	
	this->p.tabs.Add(vsplit.SizePos(), "Header");
	
	vsplit.Vert() << entries << entry_split;
	vsplit.SetPos(3333);
	
	entries.AddColumn(t_("Published"));
	entries.AddColumn(t_("Title"));
	entries.AddColumn(t_("Message"));
	entries.AddColumn(t_("Hashtags"));
	entries.AddColumn(t_("Comments"));
	entries.AddColumn(t_("Score"));
	entries.AddIndex("IDX");
	entries.ColumnWidths("3 5 10 4 1 1");
	entries.WhenBar << THISBACK(Platforms_Header_EntryListMenu);
	
	entry_split.Horz() << attr_keys << attr_value;
	entry_split.SetPos(2500);
	attr_keys.AddColumn(t_("Attribute key"));
	attr_keys.AddIndex("IDX");
	attr_keys.Add(t_("Original description"), -1);
	for(int j = 0; j < PLATDESC_MODE_COUNT; j++) {
		String mode = GetPlatformDescriptionModeKey(j);
		for(int i = 0; i < PLATDESC_LEN_COUNT; i++) {
			String len = GetPlatformDescriptionLengthKey(i);
			int idx = i * PLATDESC_MODE_COUNT + j;
			String t = mode + " " + len;
			attr_keys.Add(t, idx);
		}
	}
	attr_keys.SetCursor(0);
	attr_keys.WhenCursor << THISBACK(Platforms_Header_DataPlatform);
	attr_value.WhenAction << THISBACK(Platforms_Header_OnValueChange);
	
}

void BiographyPlatformCtrl::Platforms_Header_DataPlatform() {
	PREF(platforms)
	REF(vsplit);
	REF(entries);
	REF(entry_split);
	REF(attr_keys);
	REF(attr_value);
	
	DatasetPtrs mp = GetDataset();
	Biography& biography = *mp.biography;
	BiographyPlatform& analysis = *mp.analysis;
	
	if (!mp.profile || !platforms.IsCursor()) {
		attr_value.Clear();
		return;
	}
	
	int plat_i = platforms.GetCursor();
	
	const Platform& plat = GetPlatforms()[plat_i];
	if (plat_i >= analysis.platforms.GetCount()) {
		attr_value.Clear();
		return;
	}
	
	PlatformBiographyPlatform& plat_anal = analysis.platforms[plat_i];
	
	if (attr_keys.IsCursor()) {
		int key_i = attr_keys.Get("IDX");
		int total_desc = PLATDESC_LEN_COUNT * PLATDESC_MODE_COUNT;
		int len_i = key_i / PLATDESC_MODE_COUNT;
		int mode_i = key_i % PLATDESC_MODE_COUNT;
		if (key_i < 0)
			attr_value.SetData(plat_anal.profile_description_from_biography);
		else if (key_i < total_desc)
			attr_value.SetData(plat_anal.descriptions[len_i][mode_i]);
		else
			attr_value.SetData("");
	}
	else {
		attr_value.Clear();
	}
}

void BiographyPlatformCtrl::Platforms_Header_OnValueChange() {
	PREF(platforms)
	REF(vsplit);
	REF(entries);
	REF(entry_split);
	REF(attr_keys);
	REF(attr_value);
	
	DatasetPtrs mp = GetDataset();
	Biography& biography = *mp.biography;
	BiographyPlatform& analysis = *mp.analysis;
	
	if (!platforms.IsCursor() || !attr_keys.IsCursor())
		return;
	
	int plat_i = platforms.GetCursor();
	const Platform& plat = GetPlatforms()[plat_i];
	PlatformBiographyPlatform& plat_anal = analysis.platforms[plat_i];
	
	
	int key_i = attr_keys.Get("IDX");
	int total_desc = PLATDESC_LEN_COUNT * PLATDESC_MODE_COUNT;
	int len_i = key_i / PLATDESC_MODE_COUNT;
	int mode_i = key_i % PLATDESC_MODE_COUNT;
	if (key_i < 0)
		plat_anal.profile_description_from_biography = attr_value.GetData();
	else if (key_i < total_desc)
		plat_anal.descriptions[len_i][mode_i] = attr_value.GetData();
}

void BiographyPlatformCtrl::Platforms_Header_ToolMenu(Bar& bar) {
	bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
}

void BiographyPlatformCtrl::Platforms_Header_EntryListMenu(Bar& bar) {
	
}

void BiographyPlatformCtrl::Platforms_Header_Do(int fn) {
	PREF(platforms)
	REF(vsplit);
	REF(entries);
	REF(entry_split);
	REF(attr_keys);
	REF(attr_value);
	
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.release)
		return;
	SocialHeaderProcess& ss = SocialHeaderProcess::Get(mp);
	if (fn == 0) {
		ss.Start();
	}
	else if (fn == 1) {
		ss.Stop();
	}
}


END_UPP_NAMESPACE
#undef PREF
#undef REF
