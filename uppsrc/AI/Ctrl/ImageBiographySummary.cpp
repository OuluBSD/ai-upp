#include "Ctrl.h"

NAMESPACE_UPP


ImageBiographySummaryCtrl::ImageBiographySummaryCtrl() {
	Add(hsplit.VSizePos(0,20).HSizePos());
	Add(prog.BottomPos(0,20).HSizePos(300));
	Add(remaining.BottomPos(0,20).LeftPos(0,300));
	
	hsplit.Horz() << categories << vsplit;
	hsplit.SetPos(1500);
	
	vsplit.Vert() << blocks << block;
	vsplit.SetPos(3333);
	
	CtrlLayout(block);
	
	categories.AddColumn(t_("Category"));
	categories.AddColumn(t_("Entries"));
	categories.AddIndex("IDX");
	for(int i = 0; i < BIOCATEGORY_COUNT; i++) {
		categories.Set(i, 0, GetBiographyCategoryKey(i));
		categories.Set(i, "IDX", i);
	}
	categories.ColumnWidths("5 1");
	categories.SetSortColumn(0);
	categories.SetCursor(0);
	categories <<= THISBACK(DataCategory);
	
	
	blocks.AddColumn(t_("First Year"));
	blocks.AddColumn(t_("Last Year"));
	blocks.AddColumn(t_("Year count"));
	blocks.AddColumn(t_("Age"));
	blocks.AddColumn(t_("Class"));
	blocks.AddColumn(t_("Keywords"));
	blocks.AddColumn(t_("Text"));
	blocks.AddIndex("IDX");
	blocks.ColumnWidths("1 1 1 1 1 5 10");
	blocks.WhenCursor << THISBACK(DataYear);
	
	
	block.keywords <<= THISBACK(OnValueChange);
	block.native_text <<= THISBACK(OnValueChange);
	block.text <<= THISBACK(OnValueChange);
	
}

void ImageBiographySummaryCtrl::Data() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.owner) {
		for(int i = 0; i < categories.GetCount(); i++)
			categories.Set(i, 1, 0);
		return;
	}
	Human& owner = *mp.owner;
	Biography& biography = *mp.biography;
	
	for(int i = 0; i < categories.GetCount(); i++) {
		int cat_i = categories.Get(i, "IDX");
		BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
		//categories.Set(i, 1, bcat.GetFilledCount());
	}
	DataCategory();
}

void ImageBiographySummaryCtrl::DataCategory() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.owner || !categories.IsCursor()) {
		blocks.Clear();
		return;
	}
	Human& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	bcat.RealizeSummaries();
	
	for(int i = 0; i < bcat.summaries.GetCount(); i++) {
		const auto& range = bcat.summaries.GetKey(i);
		const BioYear& by = bcat.summaries[i];
		int age = by.year - owner.year_of_birth;
		int cls = age - 7;
		String cls_str;
		if (cls >= 0) {
			int round = cls / 12;
			cls = cls % 12;
			cls_str.Cat('A' + round);
			cls_str += " " + IntStr(1+cls);
		}
		blocks.Set(i, 0, range.off);
		blocks.Set(i, 1, range.off + range.len - 1);
		blocks.Set(i, 2, range.len);
		blocks.Set(i, 3, age);
		blocks.Set(i, 4, cls_str);
		blocks.Set(i, 5, by.keywords);
		blocks.Set(i, 6, by.text);
		blocks.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR(blocks);
	//blocks.SetSortColumn(0, false);
	blocks.SetCount(bcat.summaries.GetCount());
	if (blocks.GetCount() && !blocks.IsCursor())
		blocks.SetCursor(0);
	
	DataYear();
}

void ImageBiographySummaryCtrl::DataYear() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.owner || !categories.IsCursor() || !blocks.IsCursor())
		return;
	Human& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int block_i = blocks.Get("IDX");
	if (block_i >= bcat.summaries.GetCount()) return;
	BioYear& by = bcat.summaries[block_i];
	
	block.year.SetData(by.year);
	block.keywords.SetData(by.keywords);
	block.native_text.SetData(by.native_text);
	block.text.SetData(by.text);
}

void ImageBiographySummaryCtrl::OnValueChange() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.owner || !categories.IsCursor() || !blocks.IsCursor())
		return;
	if (!mp.editable_biography)
		return;
	mp.snap->last_modified = GetSysTime();
	
	Human& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int block_i = blocks.Get("IDX");
	BioYear& by = bcat.summaries[block_i];
	
	by.keywords = block.keywords.GetData();
	by.native_text = block.native_text.GetData();
	by.text = block.text.GetData();
	
	blocks.Set(5, by.keywords);
	blocks.Set(6, by.text);
}

void ImageBiographySummaryCtrl::MakeKeywords () {
	TaskMgr& m = TaskMgr::Single();
	SocialArgs args;
	args.text = block.text.GetData();
	m.GetSocial(args, [this](String s) {PostCallback(THISBACK1(OnKeywords, s));});
}

void ImageBiographySummaryCtrl::Translate() {
	TaskMgr& m = TaskMgr::Single();
	
	String src = block.native_text.GetData();
	
	m.Translate("FI-FI", src, "EN-US", [this](String s) {PostCallback(THISBACK1(OnTranslate, s));});
}

void ImageBiographySummaryCtrl::OnTranslate(String s) {
	block.text.SetData(s);
	OnValueChange();
}

void ImageBiographySummaryCtrl::OnKeywords(String s) {
	RemoveEmptyLines2(s);
	Vector<String> parts = Split(s, "\n");
	s = Join(parts, ", ");
	block.keywords.SetData(s);
	OnValueChange();
}

void ImageBiographySummaryCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Start"), AppImg::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), AppImg::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	//bar.Add(t_("Translate"), AppImg::BlueRing(), THISBACK(Translate)).Key(K_F5);
	//bar.Add(t_("Make keywords"), AppImg::BlueRing(), THISBACK(MakeKeywords)).Key(K_F6);
}

void ImageBiographySummaryCtrl::Do(int fn) {
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.snap)
		return;
	if (mp.editable_biography) {
		PromptOK(t_("The latest (and editable) revision can't be processed. Select older than latest revision."));
		return;
	}
	
	ImageBiographySummaryProcess& ss = ImageBiographySummaryProcess::Get(*mp.profile, *mp.snap);
	if (fn == 0) {
		ss.Start();
	}
	else if (fn == 1) {
		ss.Stop();
	}
}

void ImageBiographySummaryCtrl::EntryListMenu(Bar& bar) {
	
}


END_UPP_NAMESPACE
