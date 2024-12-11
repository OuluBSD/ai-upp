#include "Ctrl.h"

NAMESPACE_UPP


BiographySummaryCtrl::BiographySummaryCtrl() {
	Add(hsplit.VSizePos(0,20).HSizePos());
	Add(prog.BottomPos(0,20).HSizePos(300));
	Add(remaining.BottomPos(0,20).LeftPos(0,300));
	
	hsplit.Horz() << categories << vsplit;
	hsplit.SetPos(1500, 0);
	hsplit.SetPos(3000, 1);
	
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
	
	block.elements.AddColumn("Key");
	block.elements.AddColumn("Value");
	block.elements.AddColumn("Score");
	block.elements.ColumnWidths("2 12 1");
	
}

void BiographySummaryCtrl::Data() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.owner || !mp.biography) {
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

void BiographySummaryCtrl::DataCategory() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.owner || !mp.biography || !categories.IsCursor()) {
		blocks.Clear();
		return;
	}
	Owner& owner = *mp.owner;
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

void BiographySummaryCtrl::DataYear() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.owner || !mp.biography || !categories.IsCursor() || !blocks.IsCursor())
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
	
	UpdateElements();
}

void BiographySummaryCtrl::UpdateElements() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.owner || !mp.biography || !categories.IsCursor() || !blocks.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int block_i = blocks.Get("IDX");
	if (block_i >= bcat.summaries.GetCount()) return;
	BioYear& by = bcat.summaries[block_i];
	
	for(int i = 0; i < by.elements.GetCount(); i++) {
		block.elements.Set(i, 0, Capitalize(by.elements[i].key));
		block.elements.Set(i, 1, by.elements[i].value);
		block.elements.Set(i, 2, by.elements[i].GetAverageScore());
	}
	block.elements.SetCount(by.elements.GetCount());
	
}

void BiographySummaryCtrl::OnValueChange() {
	DatasetPtrs mp = GetDataset();
	if (!mp.owner || !mp.biography ||!categories.IsCursor() || !blocks.IsCursor())
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

void BiographySummaryCtrl::MakeKeywords () {
	TaskMgr& m = AiTaskManager();
	SocialArgs args;
	args.text = block.text.GetData();
	m.GetSocial(args, [this](String s) {PostCallback(THISBACK1(OnKeywords, s));});
}

void BiographySummaryCtrl::Translate() {
	TaskMgr& m = AiTaskManager();
	
	String src = block.native_text.GetData();
	
	m.Translate("FI-FI", src, "EN-US", [this](String s) {PostCallback(THISBACK1(OnTranslate, s));});
}

void BiographySummaryCtrl::OnTranslate(String s) {
	block.text.SetData(s);
	OnValueChange();
}

void BiographySummaryCtrl::OnKeywords(String s) {
	RemoveEmptyLines2(s);
	Vector<String> parts = Split(s, "\n");
	s = Join(parts, ", ");
	block.keywords.SetData(s);
	OnValueChange();
}

void BiographySummaryCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	//bar.Add(t_("Translate"), TextImgs::BlueRing(), THISBACK(Translate)).Key(K_F5);
	//bar.Add(t_("Make keywords"), TextImgs::BlueRing(), THISBACK(MakeKeywords)).Key(K_F6);
}

void BiographySummaryCtrl::Do(int fn) {
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.release)
		return;
	if (mp.editable_biography) {
		PromptOK(t_("The latest (and editable) revision won't be processed. Select other than the latest revision."));
		return;
	}
	BiographySummaryProcess& sdi = BiographySummaryProcess::Get(*mp.profile, *mp.snap);
	prog.Attach(sdi);
	sdi.WhenRemaining << [this](String s) {PostCallback([this,s](){remaining.SetLabel(s);});};
	if (fn == 0)
		sdi.Start();
	else
		sdi.Stop();
}

void BiographySummaryCtrl::EntryListMenu(Bar& bar) {
	
}

















BiographySummaryProcess::BiographySummaryProcess() {
	
}

int BiographySummaryProcess::GetPhaseCount() const {
	return PHASE_COUNT;
}

int BiographySummaryProcess::GetBatchCount(int phase) const {
	switch (phase) {
		case PHASE_FIX_SUMMARY_HASHES:					return 1;
		case PHASE_SUMMARIZE_USING_EXISTING:			return BIOCATEGORY_COUNT;
		case PHASE_SUMMARIZE:							return BIOCATEGORY_COUNT;
		case PHASE_SUMMARIZE_ELEMENTS_USING_EXISTING:	return BIOCATEGORY_COUNT;
		case PHASE_SUMMARIZE_ELEMENTS:					return BIOCATEGORY_COUNT;
		default: return 1;
	}
}

int BiographySummaryProcess::GetSubBatchCount(int phase, int batch) const {
	int bcat_i = batch;
	BiographyCategory& bcat = snap->data.GetAdd(*p.owner, bcat_i);
	bcat.RealizeSummaries();
	switch (phase) {
		case PHASE_FIX_SUMMARY_HASHES:					return 1;
		case PHASE_SUMMARIZE_USING_EXISTING:			return bcat.summaries.GetCount();
		case PHASE_SUMMARIZE:							return bcat.summaries.GetCount();
		case PHASE_SUMMARIZE_ELEMENTS_USING_EXISTING:	return bcat.summaries.GetCount();
		case PHASE_SUMMARIZE_ELEMENTS:					return bcat.summaries.GetCount();
		default: return 1;
	}
}

void BiographySummaryProcess::DoPhase() {
	switch (phase) {
		case PHASE_FIX_SUMMARY_HASHES:					FixSummaryHashes(); return;
		case PHASE_SUMMARIZE_USING_EXISTING:			SummarizeUsingExisting(); return;
		case PHASE_SUMMARIZE:							Summarize(); return;
		case PHASE_SUMMARIZE_ELEMENTS_USING_EXISTING:	SummarizeElementsUsingExisting(); return;
		case PHASE_SUMMARIZE_ELEMENTS:					SummarizeElements(); return;
		default: return;
	}
}

BiographySummaryProcess& BiographySummaryProcess::Get(Profile& p, BiographySnapshot& snap) {
	static ArrayMap<String, BiographySummaryProcess> arr;
	
	TODO
	#if 0
	String key = "PROFILE(" + p.name + "), REVISION(" + IntStr(snap.revision) + ")";
	BiographySummaryProcess& ts = arr.GetAdd(key);
	ts.owner = p.owner;
	ts.profile = &p;
	ts.snap = &snap;
	ASSERT(ts.owner);
	return ts;
	#endif
	return Single<BiographySummaryProcess>();
}

void BiographySummaryProcess::FixSummaryHashes() {
	TODO
	#if 0
	// no latest snapsphot
	for(int i = 0; i < profile->snapshots.GetCount()-1; i++) {
		auto& snap = profile->snapshots[i];
		
		auto& cats = snap.data.AllCategories();
		for(int j = 0; j < cats.GetCount(); j++) {
			auto& bcat = cats[j];
			
			for(int k = 0; k < bcat.summaries.GetCount(); k++) {
				const BioRange& range = bcat.summaries.GetKey(k);
				auto& sum = bcat.summaries[k];
				if (sum.source_hash != 0)
					continue;
				CombineHash ch;
				if (range.len == 2) {
					int begin = range.off;
					int end = range.off + range.len;
					ASSERT(begin < end && end - begin < 100);
					for(int i = begin; i < end; i++) {
						BioYear& by = bcat.GetAdd(i);
						ch.Do(by.text);
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
						ch.Do(by.text);
					}
				}
				sum.source_hash = ch;
				ASSERT(sum.source_hash != 0);
			}
		}
	}
	#endif
	
	NextPhase();
}

void BiographySummaryProcess::SummarizeUsingExisting() {
	Biography& biography = snap->data;
	
	TODO
	#if 0
	// Source data hash must be updated in earlier phase, and it won't be done to the latest
	ASSERT(&biography != &profile->snapshots.Top().data);
	
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
	
	// Apparently there is no input text
	if (sum.source_hash == 0) {
		NextSubBatch();
		return;
	}
	
	
	// Loop other snapshots to search for older summary with the same hash
	hash_t cmp = sum.source_hash;
	bool found = false;
	for(int i = 0; i < profile->snapshots.GetCount(); i++) {
		const auto& snap = profile->snapshots[i];
		BiographyCategory& bcat1 = biography.GetAdd(*owner, bcat_i);
		for(int k = 0; k < bcat1.summaries.GetCount(); k++) {
			const auto& by = bcat1.summaries[k];
			
			if (cmp == by.source_hash && by.text.GetCount()) {
				sum.text = by.text;
				NextSubBatch();
				return;
			}
		}
	}
	#endif
	
	NextSubBatch();
}

bool BiographySummaryProcess::SummarizeBase(int fn, BiographySummaryProcessArgs& args) {
	Biography& biography = snap->data;
	
	if (batch >= BIOCATEGORY_COUNT) {
		NextPhase();
		return false;
	}
	int bcat_i = batch;
	
	BiographyCategory& bcat = biography.GetAdd(*p.owner, bcat_i);
	bcat.RealizeSummaries();
	if (sub_batch >= bcat.summaries.GetCount()) {
		NextBatch();
		return false;
	}
	
	const BioRange& range = bcat.summaries.GetKey(sub_batch);
	BioYear& sum = bcat.summaries[sub_batch];
	
	if (fn == 0 && skip_ready && sum.text.GetCount()) {
		NextSubBatch();
		return false;
	}
	else if (fn == 1 && skip_ready && sum.elements.GetCount()) {
		NextSubBatch();
		return false;
	}
	
	args.fn = fn;
	
	if (range.len == 2) {
		int begin = range.off;
		int end = range.off + range.len;
		ASSERT(begin < end && end - begin < 100);
		for(int i = begin; i < end; i++) {
			BioYear& by = bcat.GetAdd(i);
			String title = GetBiographyCategoryKey(bcat_i) +
				", year " + IntStr(by.year) +
				", age " + IntStr(by.year - p.owner->year_of_birth);
			if (fn == 0 && by.text.GetCount())
				args.parts.Add(title, by.text);
			else if (fn == 1 && by.elements.GetCount())
				args.parts.Add(title, by.JoinElementMap(": ", "\n"));
		}
		if (args.parts.IsEmpty()) {
			NextSubBatch();
			return false;
		}
		else if (args.parts.GetCount() == 1) {
			if (fn == 0)
				OnProcessSummarize("(" + args.parts.GetKey(0) + ") " + args.parts[0]);
			else if (fn == 1)
				OnProcessSummarizeElements(args.parts[0]);
			return false;
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
			if (fn == 0 && by.text.GetCount())
				args.parts.Add(title, by.text);
			else if (fn == 1 && by.elements.GetCount())
				args.parts.Add(title, by.JoinElementMap(": ", "\n"));
		}
		if (args.parts.IsEmpty()) {
			NextSubBatch();
			return false;
		}
		else if (args.parts.GetCount() == 1) {
			if (fn == 0)
				OnProcessSummarize("(" + args.parts.GetKey(0) + ") " + args.parts[0]);
			else if (fn == 1)
				OnProcessSummarizeElements(args.parts[0]);
			return false;
		}
	}
	return true;
}


void BiographySummaryProcess::Summarize() {
	BiographySummaryProcessArgs args; // 0
	
	if (!SummarizeBase(0, args))
		return;
	
	SetWaiting(1);
	TaskMgr& m = AiTaskManager();
	m.GetBiographySummary(args, THISBACK(OnProcessSummarize));
}

void BiographySummaryProcess::OnProcessSummarize(String res) {
	Biography& biography = snap->data;
	
	int bcat_i = batch;
	BiographyCategory& bcat = biography.GetAdd(*p.owner, bcat_i);
	const BioRange& range = bcat.summaries.GetKey(sub_batch);
	BioYear& sum = bcat.summaries[sub_batch];
	
	sum.text = TrimBoth(res);
	
	NextSubBatch();
	SetWaiting(0);
}

void BiographySummaryProcess::SummarizeElementsUsingExisting() {
	Biography& biography = snap->data;
	
	TODO
	#if 0
	// Source data hash must be updated in earlier phase, and it won't be done to the latest
	ASSERT(&biography != &profile->snapshots.Top().data);
	
	if (batch >= BIOCATEGORY_COUNT) {
		NextPhase();
		return;
	}
	int bcat_i = batch;
	
	BiographyCategory& bcat = biography.GetAdd(*owner, bcat_i);
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
	
	// Apparently there is no input text
	if (sum.source_hash == 0) {
		NextSubBatch();
		return;
	}
	
	
	// Loop other snapshots to search for older summary with the same hash
	hash_t cmp = sum.source_hash;
	bool found = false;
	for(int i = 0; i < profile->snapshots.GetCount(); i++) {
		const auto& snap = profile->snapshots[i];
		const BiographyCategory* bcat1 = snap.data.Find(*owner, bcat_i);
		if (!bcat1) continue;
		for(int k = 0; k < bcat1->summaries.GetCount(); k++) {
			const auto& by = bcat1->summaries[k];
			
			if (cmp == by.source_hash && by.elements.GetCount()) {
				sum.elements <<= by.elements;
				NextSubBatch();
				return;
			}
		}
	}
	#endif
	
	NextSubBatch();
}

void BiographySummaryProcess::SummarizeElements() {
	BiographySummaryProcessArgs args; // 1
	
	if (!SummarizeBase(1, args))
		return;
	
	SetWaiting(1);
	TaskMgr& m = AiTaskManager();
	m.GetBiographySummary(args, THISBACK(OnProcessSummarizeElements));
}

void BiographySummaryProcess::OnProcessSummarizeElements(String result) {
	Biography& biography = snap->data;
	
	TODO
	#if 0
	int bcat_i = batch;
	BiographyCategory& bcat = biography.GetAdd(*owner, bcat_i);
	const BioRange& range = bcat.summaries.GetKey(sub_batch);
	BioYear& sum = bcat.summaries[sub_batch];
	
	RemoveEmptyLines3(result);
	Vector<String> lines = Split(result, "\n");
	
	sum.elements.Clear();
	for (String& line : lines) {
		int a = line.Find(":");
		if (a < 0) continue;
		String key = ToLower(TrimBoth(line.Left(a)));
		String value = TrimBoth(line.Mid(a+1));
		RemoveQuotes(value);
		String lvalue = ToLower(value);
		int i = sum.FindElement(key);
		if (lvalue.IsEmpty() || lvalue == "none." || lvalue == "none" || lvalue.Left(6) == "none (" || lvalue == "ready." || lvalue == "ready" || lvalue.Left(6) == "ready (" || lvalue == "n/a" || lvalue == "n/a." || lvalue == "n/a. n/a." ||  lvalue == "n/a; n/a" || lvalue == "n/a, n/a" || lvalue == "none mentioned.")
			continue;
		if (lvalue.Right(5) == " n/a.")
			value = value.Left(value.GetCount() - 5);
		if (lvalue.Left(4) == "n/a.")
			value = TrimBoth(value.Mid(4));
		else if (lvalue.Left(4) == "n/a,")
			value = TrimBoth(value.Mid(4));
		else if (lvalue.Left(3) == "n/a")
			value = TrimBoth(value.Mid(3));
		
		if (i < 0) {
			i = sum.elements.GetCount();
			sum.elements.Add();
		}
		auto& el = sum.elements[i];
		el.key = key;
		el.value = value;
		el.ResetScore();
	}
	#endif
	
	NextSubBatch();
	SetWaiting(0);
}


END_UPP_NAMESPACE
