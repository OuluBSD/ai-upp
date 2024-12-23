#include "Ctrl.h"

NAMESPACE_UPP

void BiographyPlatformCtrl::Clusters_Ctor() {
	this->tabs.Add(c.hsplit.SizePos(), t_("Prompt clusters"));
	
	c.hsplit.Horz() << c.image_types << c.vsplit;
	c.hsplit.SetPos(1200);
	
	c.vsplit.Vert() << c.prompts << c.final_prompt << c.bsplit;
	c.vsplit.SetPos(5000,0);
	
	c.image_types.AddColumn(t_("Image group"));
	c.image_types.AddColumn(t_("Prompt count"));
	c.image_types.AddIndex("IDX");
	c.image_types.WhenCursor << THISBACK(Clusters_DataImageType);
	c.image_types.ColumnWidths("4 1");
	
	c.prompts.AddColumn(t_("Prompt"));
	
	c.final_prompt.SetEditable(false);
	
	c.bsplit.Horz();
	for(int i = 0; i < 4; i++)
		c.bsplit << c.epk_photo[i];
}

void BiographyPlatformCtrl::Clusters_Data() {
	DatasetPtrs mp = GetDataset();
	if (!mp.analysis) {
		PromptOK("No PlatformManager component found");
		return;
	}
	BiographyPlatform& analysis = GetExt<BiographyPlatform>();
	
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
	
	Clusters_DataImageType();
}

void BiographyPlatformCtrl::Clusters_ToolMenu(Bar& bar) {
	/*bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	bar.Separator();
	bar.Add(t_("Import Json"), TextImgs::BlueRing(), THISBACK(ImportJson));*/
}

void BiographyPlatformCtrl::Clusters_DataImageType() {
	DatasetPtrs mp = GetDataset();
	if (!mp.analysis) {
		PromptOK("No PlatformManager component found");
		return;
	}
	BiographyPlatform& analysis = *mp.analysis;
	
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


END_UPP_NAMESPACE
