#include "Ctrl.h"

NAMESPACE_UPP


PlatformProfileCtrl::PlatformProfileCtrl() {
	Add(tabs.VSizePos(0,20).HSizePos());
	Add(prog.BottomPos(0,20).HSizePos(300));
	Add(remaining.BottomPos(0,20).LeftPos(0,300));
	
	// Platform tab
	{
		tabs.Add(p.hsplit.SizePos(), t_("Platforms"));
		
		p.hsplit.Horz() << p.platforms << p.tabs;
		p.hsplit.SetPos(2000);
		
		p.tabs.Add(p.epk_photo_prompt_split.SizePos(), t_("EPK Photo prompts"));
		p.tabs.WhenSet << THISBACK(DataPlatform);
		
		p.epk_photo_prompt_split.Vert() << p.epk_photo_prompts << p.epk_photo_multi_image_split;
		p.epk_photo_multi_image_split.Horz();
		for(int i = 0; i < 4; i++)
			p.epk_photo_multi_image_split << p.epk_photo[i];
		
		p.epk_photo_prompts.AddColumn(t_("Type"));
		p.epk_photo_prompts.AddColumn(t_("Text"));
		p.epk_photo_prompts.AddIndex("IDX0");
		p.epk_photo_prompts.AddIndex("IDX1");
		p.epk_photo_prompts.ColumnWidths("1 3");
		p.epk_photo_prompts.WhenCursor << THISBACK(OnPhotoPrompt);
		p.epk_photo_prompts.WhenBar << THISBACK(PhotoPromptMenu);
		
		p.platforms.AddColumn(t_("Platform"));
		p.platforms.AddColumn(t_("Family Chosen By Me"));
		p.platforms.AddColumn(t_("Rights representatives"));
		p.platforms.AddColumn(t_("Military Rank"));
		p.platforms.AddColumn(t_("Sex"));
		p.platforms.AddIndex("IDX");
		p.platforms.ColumnWidths("2 1 1 1 1");
		for(int i = 0; i < PLATFORM_COUNT; i++) {
			const Platform& plat = GetPlatforms()[i];
			p.platforms.Set(i, 0, plat.name);
			//p.platforms.Set(i, 1, plat.group);
			p.platforms.Set(i, "IDX", i);
		}
		//p.platforms.SetSortColumn(1);
		p.platforms.SetCursor(0);
		p.platforms.WhenCursor << THISBACK(DataPlatform);
		p.platforms.WhenBar << THISBACK(PlatformMenu);
	}
	
	// Clustered prompts tab
	{
		tabs.Add(c.hsplit.SizePos(), t_("Prompt clusters"));
		
		c.hsplit.Horz() << c.image_types << c.vsplit;
		c.hsplit.SetPos(1200);
		
		c.vsplit.Vert() << c.prompts << c.final_prompt << c.bsplit;
		c.vsplit.SetPos(5000,0);
		
		c.image_types.AddColumn(t_("Image group"));
		c.image_types.AddColumn(t_("Prompt count"));
		c.image_types.AddIndex("IDX");
		c.image_types.WhenCursor << THISBACK(DataImageType);
		c.image_types.ColumnWidths("4 1");
		
		c.prompts.AddColumn(t_("Prompt"));
		
		c.final_prompt.SetEditable(false);
		
		c.bsplit.Horz();
		for(int i = 0; i < 4; i++)
			c.bsplit << c.epk_photo[i];
	}
	
	tabs.WhenSet << THISBACK(Data);
}

void PlatformProfileCtrl::Data() {
	int tab = tabs.Get();
	
	if (tab == 0)
		DataPlatforms();
	else if (tab == 1)
		DataClusters();
}

void PlatformProfileCtrl::DataPlatforms() {
	p.platforms.Clear();
	for(int i = 0; i < PLATFORM_COUNT; i++) {
		const Platform& pl = GetPlatforms()[i];
		const PlatformAnalysis& pa = MetaDatabase::Single().GetAdd(pl);
		p.platforms.Set(i, 0, pl.name);
		p.platforms.Set(i, "IDX", i);
		p.platforms.Set(i, 1, pa.GetRoleScoreSumWeighted(SOCIETYROLE_SCORE_FAMILY_CHOSEN_BY_ME));
		p.platforms.Set(i, 2, pa.GetRoleScoreSumWeighted(SOCIETYROLE_SCORE_REPRESENTATIVE_FOR_RIGHTS_OF_SOMEONE));
		p.platforms.Set(i, 3, pa.GetRoleScoreSumWeighted(SOCIETYROLE_SCORE_MILTARY_RANK_RELATED));
		
		double female = pa.GetRoleScoreSumWeighted(SOCIETYROLE_SCORE_FEMALE);
		double male = pa.GetRoleScoreSumWeighted(SOCIETYROLE_SCORE_MALE);
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
	
	DataPlatform();
}

void PlatformProfileCtrl::DataPlatform() {
	DatasetPtrs mp = GetDataset();
	if (!p.platforms.IsCursor() || !mp.profile || !mp.analysis) {
		return;
	}
	BiographyAnalysis& analysis = *mp.analysis;
	int plat_i = p.platforms.Get("IDX");
	const Platform& pl = GetPlatforms()[plat_i];
	const PlatformAnalysis& pa = MetaDatabase::Single().GetAdd(pl);
	if (plat_i >= analysis.platforms.GetCount()) return;
	const PlatformBiographyAnalysis& pba = analysis.platforms[plat_i];
	
	
	int tab = p.tabs.Get();
	if (tab == 0) {
		int row = 0;
		for(int i = 0; i < pba.epk_photos.GetCount(); i++) {
			const PlatformAnalysisPhoto& pap = pba.epk_photos[i];
			String group = pba.epk_photos.GetKey(i);
			for(int j = 0; j < pap.prompts.GetCount(); j++) {
				const PhotoPrompt& pp = pap.prompts[j];
				p.epk_photo_prompts.Set(row, 0, group);
				p.epk_photo_prompts.Set(row, 1, pp.prompt);
				p.epk_photo_prompts.Set(row, "IDX0", i);
				p.epk_photo_prompts.Set(row, "IDX1", j);
				row++;
			}
		}
		p.epk_photo_prompts.SetCount(row);
		if (row && !p.epk_photo_prompts.IsCursor())
			p.epk_photo_prompts.SetCursor(0);
		
		OnPhotoPrompt();
	}
}

void PlatformProfileCtrl::DataClusters() {
	DatasetPtrs mp = GetDataset();
	BiographyAnalysis& analysis = *mp.analysis;
	
	analysis.RealizePromptImageTypes();
	
	for(int i = 0; i < analysis.image_types.GetCount(); i++) {
		String key = analysis.image_types.GetKey(i);
		const PhotoPromptGroupAnalysis& ppga = analysis.image_types[i];
		c.image_types.Set(i, 0, Capitalize(key));
		c.image_types.Set(i, 1, ppga.image_count);
		c.image_types.Set(i, "IDX", i);
	}
	c.image_types.SetSortColumn(1, true);
	INHIBIT_CURSOR(c.image_types);
	if (!c.image_types.IsCursor() && c.image_types.GetCount())
		c.image_types.SetCursor(0);
	
	DataImageType();
}

void PlatformProfileCtrl::DataImageType() {
	DatasetPtrs mp = GetDataset();
	BiographyAnalysis& analysis = *mp.analysis;
	
	if (!c.image_types.IsCursor())
		return;
	
	int img_i = c.image_types.Get("IDX");
	
	String group = analysis.image_types.GetKey(img_i);
	PhotoPromptGroupAnalysis& ppga = analysis.image_types[img_i];
	Vector<PhotoPromptLink> pps = analysis.GetImageTypePrompts(group);
	
	for(int i = 0; i < pps.GetCount(); i++) {
		PhotoPromptLink& ppl = pps[i];
		c.prompts.Set(i, 0, ppl.pp->prompt);
	}
	c.prompts.SetCount(pps.GetCount());
	
	c.final_prompt.SetData(ppga.prompt);
}

void PlatformProfileCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Start"), AppImg::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), AppImg::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	
}

void PlatformProfileCtrl::PhotoPromptMenu(Bar& bar) {
	bar.Add("Set as groups Top 1 prompt", [this]() {
		if (!p.platforms.IsCursor() || !p.epk_photo_prompts.IsCursor())
		return;
	
		DatasetPtrs mp = GetDataset();
		BiographyAnalysis& analysis = *mp.analysis;
		int plat_i = p.platforms.Get("IDX");
		const Platform& pl = GetPlatforms()[plat_i];
		const PlatformAnalysis& pa = MetaDatabase::Single().GetAdd(pl);
		if (plat_i >= analysis.platforms.GetCount()) return;
		PlatformBiographyAnalysis& pba = analysis.platforms[plat_i];
		
		int i = p.epk_photo_prompts.Get("IDX0");
		int j = p.epk_photo_prompts.Get("IDX1");
		PlatformAnalysisPhoto& pap = pba.epk_photos[i];
		if (j > 0) {
			PhotoPrompt& pp0 = pap.prompts[0];
			PhotoPrompt& pp1 = pap.prompts[j];
			Swap(pp0, pp1);
		}
		PostCallback(THISBACK(DataPlatform));
	});
	bar.Add("Edit prompt", [this]() {
		if (!p.platforms.IsCursor() || !p.epk_photo_prompts.IsCursor())
		return;
	
		DatasetPtrs mp = GetDataset();
		BiographyAnalysis& analysis = *mp.analysis;
		int plat_i = p.platforms.Get("IDX");
		const Platform& pl = GetPlatforms()[plat_i];
		const PlatformAnalysis& pa = MetaDatabase::Single().GetAdd(pl);
		if (plat_i >= analysis.platforms.GetCount()) return;
		PlatformBiographyAnalysis& pba = analysis.platforms[plat_i];
		
		int i = p.epk_photo_prompts.Get("IDX0");
		int j = p.epk_photo_prompts.Get("IDX1");
		PlatformAnalysisPhoto& pap = pba.epk_photos[i];
		PhotoPrompt& pp = pap.prompts[j];
		
		bool b = EditTextNotNull(
			pp.prompt,
			t_("Prompt text"),
			"",
			0
		);
		PostCallback(THISBACK(DataPlatform));
	});
}

void PlatformProfileCtrl::PlatformMenu(Bar& bar) {
	
}

void PlatformProfileCtrl::Do(int fn) {
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.snap)
		return;
	PlatformProfileProcess& ss = PlatformProfileProcess::Get(*mp.profile, *mp.snap);
	if (fn == 0) {
		ss.Start();
	}
	else if (fn == 1) {
		ss.Stop();
	}
}

void PlatformProfileCtrl::SetSorting(int col) {
	
}

void PlatformProfileCtrl::OnPhotoPrompt() {
	if (!p.platforms.IsCursor() || !p.epk_photo_prompts.IsCursor())
		return;
	
	DatasetPtrs mp = GetDataset();
	BiographyAnalysis& analysis = *mp.analysis;
	int plat_i = p.platforms.Get("IDX");
	const Platform& pl = GetPlatforms()[plat_i];
	const PlatformAnalysis& pa = MetaDatabase::Single().GetAdd(pl);
	if (plat_i >= analysis.platforms.GetCount()) return;
	const PlatformBiographyAnalysis& pba = analysis.platforms[plat_i];
	
	int i = p.epk_photo_prompts.Get("IDX0");
	int j = p.epk_photo_prompts.Get("IDX1");
	const PlatformAnalysisPhoto& pap = pba.epk_photos[i];
	const PhotoPrompt& pp = pap.prompts[j];
	
	for(int i = 0; i < 4; i++) {
		String path = pp.GetFilePath(i);
		if (FileExists(path)) {
			Image img = StreamRaster::LoadFileAny(path);
			p.epk_photo[i].SetImage(img);
		}
		else {
			p.epk_photo[i].Clear();
		}
	}
	
}


END_UPP_NAMESPACE
