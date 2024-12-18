#include "Ctrl.h"

NAMESPACE_UPP


ImageBiographySummaryCtrl::ImageBiographySummaryCtrl() {
	Add(hsplit.VSizePos(0,20).HSizePos());
	
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
	Owner& owner = *mp.owner;
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
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	bcat.RealizeSummaries();
	
	Date today = GetSysDate();
	for(int i = 0; i < bcat.summaries.GetCount(); i++) {
		const auto& range = bcat.summaries.GetKey(i);
		const BioYear& by = bcat.summaries[i];
		Date by_date(by.year, today.month, today.day);
		int age = (by_date - owner.born) / 365;
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
	Owner& owner = *mp.owner;
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
	
	Owner& owner = *mp.owner;
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
	TaskMgr& m = AiTaskManager();
	SocialArgs args;
	args.text = block.text.GetData();
	m.GetSocial(args, [this](String s) {PostCallback(THISBACK1(OnKeywords, s));});
}

void ImageBiographySummaryCtrl::Translate() {
	TaskMgr& m = AiTaskManager();
	
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
	bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	//bar.Add(t_("Translate"), TextImgs::BlueRing(), THISBACK(Translate)).Key(K_F5);
	//bar.Add(t_("Make keywords"), TextImgs::BlueRing(), THISBACK(MakeKeywords)).Key(K_F6);
}

void ImageBiographySummaryCtrl::Do(int fn) {
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.release)
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





















ImageBiographySummaryProcess::ImageBiographySummaryProcess() {
	
}

int ImageBiographySummaryProcess::GetPhaseCount() const {
	return PHASE_COUNT;
}

int ImageBiographySummaryProcess::GetBatchCount(int phase) const {
	switch (phase) {
		case PHASE_SUMMARIZE_IMAGE_CATEGORY_YEAR:	return max(1, imgsum_tasks.GetCount());
		case PHASE_SUMMARIZE_IMAGE_BIOGRAPHY:		return BIOCATEGORY_COUNT;
		default: return 1;
	}
}

int ImageBiographySummaryProcess::GetSubBatchCount(int phase, int batch) const {
	TODO
	#if 0
	if (phase == PHASE_SUMMARIZE_IMAGE_BIOGRAPHY) {
		BiographyCategory& bcat = p.snap->data.GetAdd(*p.owner, batch);
		return bcat.summaries.GetCount();
	}
	#endif
	return 1;
}

void ImageBiographySummaryProcess::DoPhase() {
	switch (phase) {
		case PHASE_SUMMARIZE_IMAGE_CATEGORY_YEAR:	ProcessSummarizeImageCategoryYear(); return;
		case PHASE_SUMMARIZE_IMAGE_BIOGRAPHY:		ProcessSummarizeImageBiography(); return;
		default: return;
	}
}

ImageBiographySummaryProcess& ImageBiographySummaryProcess::Get(Profile& prof, BiographyPerspectives& snap) {
	static ArrayMap<String, ImageBiographySummaryProcess> arr;
	
	TODO
	#if 0
	String key = "PROFILE(" + p.name + "), REVISION(" + IntStr(snap.revision) + ")";
	ImageBiographySummaryProcess& ts = arr.GetAdd(key);
	ts.owner = prof.owner;
	ts.profile = &p;
	ts.snap = &snap;
	ASSERT(ts.owner);
	return ts;
	#endif
	return Single<ImageBiographySummaryProcess>();
}

void ImageBiographySummaryProcess::TraverseImageSummaryTasks() {
	TODO
	#if 0
	Biography& biography = p.snap->data;
	for(int i = 0; i < BIOCATEGORY_COUNT; i++) {
		BiographyCategory& bcat = biography.GetAdd(*p.owner, i);
		int bcat_i = i;
		for(int j = 0; j < bcat.years.GetCount(); j++) {
			BioYear& by = bcat.years[j];
			by.RealizeImageSummaries();
			for(int k = 0; k < by.image_summaries.GetCount(); k++) {
				const BioRange& range = by.image_summaries.GetKey(k);
				BioImage& bimg = by.image_summaries[k];
				if (phase == PHASE_SUMMARIZE_IMAGE_CATEGORY_YEAR && bimg.image_text.IsEmpty()) {
					ImageSummaryTask& t = imgsum_tasks.Add();
					t.bcat = &bcat;
					t.by = &by;
					t.range = range;
					t.summary = &bimg;
					t.bcat_i = bcat_i;
				}
			}
		}
	}
	#endif
}

void ImageBiographySummaryProcess::ProcessSummarizeImageCategoryYear() {
	
	if (batch == 0) {
		imgsum_tasks.Clear();
		TraverseImageSummaryTasks();
	}
	
	if (batch >= imgsum_tasks.GetCount()) {
		NextPhase();
		return;
	}
	
	const ImageSummaryTask& t = imgsum_tasks[batch];
	const BioRange& range = t.range;
	const BiographyCategory& bcat = *t.bcat;
	BioYear& by = *t.by;
	
	SocialArgs args;
	args.fn = 10;
	
	Date today = GetSysDate();
	if (range.len == 2) {
		int begin = range.off;
		int end = range.off + range.len;
		ASSERT(begin < end && end - begin < 100);
		for(int i = begin; i < end && i < by.images.GetCount(); i++) {
			BioImage& bi = by.images[i];
			Date by_date(by.year, today.month, today.day);
			String title = GetBiographyCategoryKey(t.bcat_i) +
				", year " + IntStr(by.year) +
				", age " + IntStr((by_date - p.owner->born) / 365) +
				", image #" + IntStr(i);
			if (bi.image_text.GetCount())
				args.parts.Add(title, bi.image_text);
		}
		if (args.parts.IsEmpty()) {
			NextBatch();
			return;
		}
		else if (args.parts.GetCount() == 1) {
			OnProcessSummarizeImageCategoryYear("(" + args.parts.GetKey(0) + ") " + args.parts[0]);
			return;
		}
	}
	else {
		int step = range.len / 2;
		int begin = range.off;
		int end = range.off + range.len;
		for(int i = begin; i < end; i+=step) {
			BioRange sub_range;
			sub_range.off = i;
			sub_range.len = range.len >> 1;
			int j = by.image_summaries.Find(sub_range);
			ASSERT(j >= 0);
			BioImage& bi = by.image_summaries[j];
			Date by_date(by.year, today.month, today.day);
			int from = sub_range.off;
			int to = sub_range.off + sub_range.len - 1;
			String title =
				GetBiographyCategoryKey(t.bcat_i) +
				", year " + IntStr(by.year) +
				", age " + IntStr((by_date - p.owner->born) / 365) +
				", images from #" + IntStr(from) + " to #" + IntStr(to);
				;
			if (bi.image_text.GetCount())
				args.parts.Add(title, bi.image_text);
		}
		if (args.parts.IsEmpty()) {
			NextBatch();
			return;
		}
		else if (args.parts.GetCount() == 1) {
			OnProcessSummarizeImageCategoryYear("(" + args.parts.GetKey(0) + ") " + args.parts[0]);
			return;
		}
	}
	
	SetWaiting(1);
	TaskMgr& m = AiTaskManager();
	m.GetSocial(args, THISBACK(OnProcessSummarizeImageCategoryYear));
}

void ImageBiographySummaryProcess::OnProcessSummarizeImageCategoryYear(String res) {
	const ImageSummaryTask& t = imgsum_tasks[batch];
	
	int j = t.by->image_summaries.Find(t.range);
	ASSERT(j >= 0);
	BioImage& bi = t.by->image_summaries[j];
	String& s = bi.image_text;
	
	s = TrimBoth(res);
	if (s.Left(1) == "\"") s = s.Mid(1);
	if (s.Right(1) == "\"") s = s.Left(s.GetCount()-1);
	
	if (j == t.by->image_summaries.GetCount()-1)
		t.by->image_text = s;
	
	NextBatch();
	SetWaiting(0);
}

void ImageBiographySummaryProcess::ProcessSummarizeImageBiography() {
	TODO
	#if 0
	Biography& biography = p.snap->data;
	
	if (batch >= BIOCATEGORY_COUNT) {
		NextPhase();
		return;
	}
	
	int bcat_i = batch;
	BiographyCategory& bcat = biography.GetAdd(*p.owner, bcat_i);
	bcat.RealizeSummaries();
	if (sub_batch >= bcat.summaries.GetCount()) {
		NextBatch();
		return;
	}
	
	const BioRange& range = bcat.summaries.GetKey(sub_batch);
	BioYear& sum = bcat.summaries[sub_batch];
	
	if (skip_ready && sum.text.GetCount()) {
		NextSubBatch();
		return;
	}
	
	SocialArgs args;
	args.fn = 10;
	
	if (range.len == 2) {
		int begin = range.off;
		int end = range.off + range.len;
		ASSERT(begin < end && end - begin < 100);
		for(int i = begin; i < end; i++) {
			BioYear& by = bcat.GetAdd(i);
			String title = GetBiographyCategoryKey(bcat_i) +
				", year " + IntStr(by.year) +
				", age " + IntStr(by.year - p.owner->year_of_birth);
			if (by.image_text.GetCount())
				args.parts.Add(title, by.image_text);
		}
		if (args.parts.IsEmpty()) {
			NextSubBatch();
			return;
		}
		else if (args.parts.GetCount() == 1) {
			TODO //OnProcessSummarize("(" + args.parts.GetKey(0) + ") " + args.parts[0]);
			return;
		}
	}
	else {
		int step = range.len / 2;
		int begin = range.off;
		int end = range.off + range.len;
		for(int i = begin; i < end; i+=step) {
			BioRange sub_range;
			sub_range.off = i;
			sub_range.len = range.len >> 1;
			int j = bcat.summaries.Find(sub_range);
			ASSERT(j >= 0);
			BioYear& by = bcat.summaries[j];
			int from = sub_range.off;
			int to = sub_range.off + sub_range.len - 1;
			String title =
				GetBiographyCategoryKey(bcat_i) +
				", from year " + IntStr(from) +
				" to year " + IntStr(to) +
				", age " + IntStr(from - p.owner->year_of_birth) + " - " + IntStr(to - p.owner->year_of_birth)
				;
			if (by.image_text.GetCount())
				args.parts.Add(title, by.image_text);
		}
		if (args.parts.IsEmpty()) {
			NextSubBatch();
			return;
		}
		else if (args.parts.GetCount() == 1) {
			TODO //OnProcessSummarize("(" + args.parts.GetKey(0) + ") " + args.parts[0]);
			return;
		}
	}
	
	SetWaiting(1);
	TaskMgr& m = AiTaskManager();
	m.GetSocial(args, THISBACK(OnProcessSummarizeImageBiography));
	#endif
}

void ImageBiographySummaryProcess::OnProcessSummarizeImageBiography(String res) {
	TODO
	#if 0
	Biography& biography = p.snap->data;
	int bcat_i = batch;
	BiographyCategory& bcat = biography.GetAdd(*p.owner, bcat_i);
	const BioRange& range = bcat.summaries.GetKey(sub_batch);
	BioYear& sum = bcat.summaries[sub_batch];
	
	sum.image_text = TrimBoth(res);
	
	
	NextSubBatch();
	SetWaiting(0);
	#endif
}


INITIALIZER_COMPONENT_CTRL(ImageBiographySummary, ImageBiographySummaryCtrl)

END_UPP_NAMESPACE
