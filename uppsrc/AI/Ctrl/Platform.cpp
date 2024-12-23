#include "Ctrl.h"
#define PREF(obj) auto& obj = p.obj;
#define REF(tab, obj) auto& obj = p.tab.obj;

NAMESPACE_UPP


void BiographyPlatformCtrl::Platforms_Ctor() {
	PREF(hsplit);
	PREF(platforms);
	PREF(tabs);
	
	this->tabs.Add(hsplit.SizePos(), t_("Platforms"));
	
	hsplit.Horz() << platforms << tabs;
	hsplit.SetPos(2000);
	
	platforms.AddColumn(t_("Platform"));
	platforms.AddColumn(t_("Family Chosen By Me"));
	platforms.AddColumn(t_("Rights representatives"));
	platforms.AddColumn(t_("Military Rank"));
	platforms.AddColumn(t_("Sex"));
	platforms.AddIndex("IDX");
	platforms.ColumnWidths("2 1 1 1 1");
	for(int i = 0; i < PLATFORM_COUNT; i++) {
		const Platform& plat = GetPlatforms()[i];
		platforms.Set(i, 0, plat.name);
		//platforms.Set(i, 1, plat.group);
		platforms.Set(i, "IDX", i);
	}
	//platforms.SetSortColumn(1);
	platforms.SetCursor(0);
	platforms.WhenCursor << THISBACK(Platforms_DataPlatform);
	platforms.WhenBar << THISBACK(Platforms_Menu);
	
	
	Platforms_EpkPhoto_Ctor();
}

void BiographyPlatformCtrl::Platforms_EpkPhoto_Ctor() {
	PREF(tabs);
	REF(epk_photo, epk_photo_prompt_split);
	REF(epk_photo, epk_photo_prompts);
	REF(epk_photo, epk_photo_multi_image_split);
	REF(epk_photo, epk_photo);
	
	tabs.Add(epk_photo_prompt_split.SizePos(), t_("EPK Photo prompts"));
	tabs.WhenSet << THISBACK(Platforms_DataPlatform);
	
	epk_photo_prompt_split.Vert() << epk_photo_prompts << epk_photo_multi_image_split;
	epk_photo_multi_image_split.Horz();
	for(int i = 0; i < 4; i++)
		epk_photo_multi_image_split << epk_photo[i];
	
	epk_photo_prompts.AddColumn(t_("Type"));
	epk_photo_prompts.AddColumn(t_("Text"));
	epk_photo_prompts.AddIndex("IDX0");
	epk_photo_prompts.AddIndex("IDX1");
	epk_photo_prompts.ColumnWidths("1 3");
	epk_photo_prompts.WhenCursor << THISBACK(Platforms_OnPhotoPrompt);
	epk_photo_prompts.WhenBar << THISBACK(Platforms_PhotoPromptMenu);
	
}

void BiographyPlatformCtrl::Platforms_ToolMenu(Bar& bar) {
	/*bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	bar.Separator();
	bar.Add(t_("Import Json"), TextImgs::BlueRing(), THISBACK(ImportJson));*/
}

void BiographyPlatformCtrl::Platforms_Menu(Bar& bar) {
	
}

void BiographyPlatformCtrl::Platforms_Data() {
	DatasetPtrs ptrs = GetDataset();
	if (!ptrs.platmgr) {
		PromptOK("No PlatformManager component found");
		return;
	}
	PlatformManager& plat = *ptrs.platmgr;
	p.platforms.Clear();
	for(int i = 0; i < PLATFORM_COUNT; i++) {
		const Platform& pl = GetPlatforms()[i];
		const PlatformAnalysis& pa = ptrs.platmgr->GetPlatform(i);
		p.platforms.Set(i, 0, pl.name);
		p.platforms.Set(i, "IDX", i);
		p.platforms.Set(i, 1, pa.GetRoleScoreSumWeighted(plat, SOCIETYROLE_SCORE_FAMILY_CHOSEN_BY_ME));
		p.platforms.Set(i, 2, pa.GetRoleScoreSumWeighted(plat, SOCIETYROLE_SCORE_REPRESENTATIVE_FOR_RIGHTS_OF_SOMEONE));
		p.platforms.Set(i, 3, pa.GetRoleScoreSumWeighted(plat, SOCIETYROLE_SCORE_MILTARY_RANK_RELATED));
		
		double female = pa.GetRoleScoreSumWeighted(plat, SOCIETYROLE_SCORE_FEMALE);
		double male = pa.GetRoleScoreSumWeighted(plat, SOCIETYROLE_SCORE_MALE);
		double sum = female + male;
		if (sum == 0)
			p.platforms.Set(i, 4, 5);
		else
			p.platforms.Set(i, 4, female / sum * 10);
	}
	p.platforms.SetSortColumn(3, true);
	INHIBIT_CURSOR(p.platforms);
	if (!p.platforms.IsCursor() && p.platforms.GetCount())
		p.platforms.SetCursor(0);
	
	Platforms_DataPlatform();
}

void BiographyPlatformCtrl::Platforms_DataPlatform() {
	if (!p.platforms.IsCursor())
		return;
	
	int tab = p.tabs.Get();
	if (tab == 0)	Platforms_DataPlatform_Epk();
}

void BiographyPlatformCtrl::Platforms_DataPlatform_Epk() {
	REF(epk_photo, epk_photo_prompts);
	
	DatasetPtrs mp = GetDataset();
	BiographyPlatform& analysis = GetExt<BiographyPlatform>();
	int plat_i = p.platforms.Get("IDX");
	const Platform& pl = GetPlatforms()[plat_i];
	const PlatformAnalysis& pa = mp.platmgr->GetPlatform(plat_i);
	if (plat_i >= analysis.platforms.GetCount()) return;
	const PlatformBiographyPlatform& pba = analysis.platforms[plat_i];
	
	
	int row = 0;
	for(int i = 0; i < pba.epk_photos.GetCount(); i++) {
		const PlatformAnalysisPhoto& pap = pba.epk_photos[i];
		String group = pba.epk_photos.GetKey(i);
		for(int j = 0; j < pap.prompts.GetCount(); j++) {
			const PhotoPrompt& pp = pap.prompts[j];
			epk_photo_prompts.Set(row, 0, group);
			epk_photo_prompts.Set(row, 1, pp.prompt);
			epk_photo_prompts.Set(row, "IDX0", i);
			epk_photo_prompts.Set(row, "IDX1", j);
			row++;
		}
	}
	epk_photo_prompts.SetCount(row);
	if (row && !epk_photo_prompts.IsCursor())
		epk_photo_prompts.SetCursor(0);
	
	Platforms_OnPhotoPrompt();
}

void BiographyPlatformCtrl::Platforms_PhotoPromptMenu(Bar& bar) {
	
	bar.Add("Set as groups Top 1 prompt", [this]() {
		REF(epk_photo, epk_photo_prompts);
		if (!p.platforms.IsCursor() || !epk_photo_prompts.IsCursor())
		return;
	
		DatasetPtrs mp = GetDataset();
		BiographyPlatform& analysis = *mp.analysis;
		int plat_i = p.platforms.Get("IDX");
		const Platform& pl = GetPlatforms()[plat_i];
		const PlatformAnalysis& pa = mp.platmgr->GetPlatform(plat_i);
		if (plat_i >= analysis.platforms.GetCount()) return;
		PlatformBiographyPlatform& pba = analysis.platforms[plat_i];
		
		int i = epk_photo_prompts.Get("IDX0");
		int j = epk_photo_prompts.Get("IDX1");
		PlatformAnalysisPhoto& pap = pba.epk_photos[i];
		if (j > 0) {
			PhotoPrompt& pp0 = pap.prompts[0];
			PhotoPrompt& pp1 = pap.prompts[j];
			Swap(pp0, pp1);
		}
		PostCallback(THISBACK(Platforms_DataPlatform));
	});
	bar.Add("Edit prompt", [this]() {
		REF(epk_photo, epk_photo_prompts);
		if (!p.platforms.IsCursor() || !epk_photo_prompts.IsCursor())
		return;
	
		DatasetPtrs mp = GetDataset();
		BiographyPlatform& analysis = *mp.analysis;
		int plat_i = p.platforms.Get("IDX");
		const Platform& pl = GetPlatforms()[plat_i];
		const PlatformAnalysis& pa = mp.platmgr->GetPlatform(plat_i);
		if (plat_i >= analysis.platforms.GetCount()) return;
		PlatformBiographyPlatform& pba = analysis.platforms[plat_i];
		
		int i = epk_photo_prompts.Get("IDX0");
		int j = epk_photo_prompts.Get("IDX1");
		PlatformAnalysisPhoto& pap = pba.epk_photos[i];
		PhotoPrompt& pp = pap.prompts[j];
		
		bool b = EditTextNotNull(
			pp.prompt,
			t_("Prompt text"),
			"",
			0
		);
		PostCallback(THISBACK(Platforms_Data));
	});
}

void BiographyPlatformCtrl::Platforms_OnPhotoPrompt() {
	PREF(platforms);
	REF(epk_photo, epk_photo_prompts);
	REF(epk_photo, epk_photo);
	
	if (!platforms.IsCursor() || !epk_photo_prompts.IsCursor())
		return;
	
	DatasetPtrs mp = GetDataset();
	String dir = GetFileDirectory(GetFilePath());
	
	BiographyPlatform& analysis = *mp.analysis;
	int plat_i = p.platforms.Get("IDX");
	const Platform& pl = GetPlatforms()[plat_i];
	const PlatformAnalysis& pa = mp.platmgr->GetPlatform(plat_i);
	if (plat_i >= analysis.platforms.GetCount()) return;
	const PlatformBiographyPlatform& pba = analysis.platforms[plat_i];
	
	int i = epk_photo_prompts.Get("IDX0");
	int j = epk_photo_prompts.Get("IDX1");
	const PlatformAnalysisPhoto& pap = pba.epk_photos[i];
	const PhotoPrompt& pp = pap.prompts[j];
	
	for(int i = 0; i < 4; i++) {
		String path = pp.GetFilePath(dir, i);
		if (FileExists(path)) {
			Image img = StreamRaster::LoadFileAny(path);
			epk_photo[i].SetImage(img);
		}
		else {
			epk_photo[i].Clear();
		}
	}
}


END_UPP_NAMESPACE
