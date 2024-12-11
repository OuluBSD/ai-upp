#include "Ctrl.h"

NAMESPACE_UPP


BiographyCtrl::BiographyCtrl() {
	Add(hsplit.VSizePos(0,20).HSizePos());
	Add(prog.BottomPos(0,20).HSizePos(300));
	Add(remaining.BottomPos(0,20).LeftPos(0,300));
	
	
	hsplit.Horz() << categories << vsplit;
	hsplit.SetPos(1500);
	
	vsplit.Vert() << years << year;
	vsplit.SetPos(3333);
	
	CtrlLayout(year);
	
	categories.AddColumn(t_("Required"));
	categories.AddColumn(t_("Category"));
	categories.AddColumn(t_("Entries"));
	categories.AddIndex("IDX");
	for(int i = 0; i < BIOCATEGORY_COUNT; i++) {
		categories.Set(i, 1, GetBiographyCategoryKey(i));
		categories.Set(i, "IDX", i);
	}
	categories.ColumnWidths("1 5 1");
	categories.SetSortColumn(0);
	categories.SetCursor(0);
	categories <<= THISBACK(DataCategory);
	
	
	years.AddColumn(t_("Year"));
	years.AddColumn(t_("Age"));
	years.AddColumn(t_("Class"));
	years.AddColumn(t_("Keywords"));
	years.AddColumn(t_("Text"));
	years.AddIndex("IDX");
	years.ColumnWidths("1 1 1 5 10");
	years.WhenCursor << THISBACK(DataYear);
	
	
	year.keywords <<= THISBACK(OnValueChange);
	year.native_text <<= THISBACK(OnValueChange);
	year.text <<= THISBACK(OnValueChange);
	
	year.elements.AddColumn("Key");
	year.elements.AddColumn("Value");
	year.elements.AddColumn("Score");
	year.elements.ColumnWidths("2 12 1");
	
}

void BiographyCtrl::Data() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.analysis) {
		for(int i = 0; i < categories.GetCount(); i++) {
			categories.Set(i, 0, 0);
			categories.Set(i, 2, 0);
		}
		return;
	}
	Owner& owner = *mp.owner;
	Profile& profile = *mp.profile;
	BiographyAnalysis& analysis = *mp.analysis;
	Biography& biography = *mp.biography;
	
	Index<int> req_cats = analysis.GetRequiredCategories();
	for(int i = 0; i < categories.GetCount(); i++) {
		int cat_i = categories.Get(i, "IDX");
		bool req = req_cats.Find(cat_i) >= 0;
		categories.Set(i, 0, req ? "X" : "");
		BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
		int c = bcat.GetFilledCount();
		categories.Set(i, 2, c > 0 ? Value(c) : Value());
	}
	DataCategory();
}

void BiographyCtrl::DataCategory() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.biography || !categories.IsCursor()) {
		years.Clear();
		return;
	}
	Owner& owner = *mp.owner;
	Profile& profile = *mp.profile;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	
	for(int i = 0; i < bcat.years.GetCount(); i++) {
		const BioYear& by = bcat.years[i];
		int age = by.year - owner.year_of_birth;
		int cls = age - 7;
		String cls_str;
		if (cls >= 0) {
			int round = cls / 12;
			cls = cls % 12;
			cls_str.Cat('A' + round);
			cls_str += " " + IntStr(1+cls);
		}
		years.Set(i, 0, by.year);
		years.Set(i, 1, age);
		years.Set(i, 2, cls_str);
		years.Set(i, 3, by.keywords);
		years.Set(i, 4, by.text);
		years.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR(years);
	years.SetSortColumn(0, false);
	years.SetCount(bcat.years.GetCount());
	if (years.GetCount() && !years.IsCursor())
		years.SetCursor(0);
	
	DataYear();
}

void BiographyCtrl::DataYear() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !categories.IsCursor() || !years.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Profile& profile = *mp.profile;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	BioYear& by = bcat.years[year_i];
	
	year.year.SetData(by.year);
	year.keywords.SetData(by.keywords);
	year.native_text.SetData(by.native_text);
	year.text.SetData(by.text);
	
	UpdateElements();
}

void BiographyCtrl::UpdateElements() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !categories.IsCursor() || !years.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Profile& profile = *mp.profile;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	BioYear& by = bcat.years[year_i];
	
	double score_sum = 0;
	for(int i = 0; i < by.elements.GetCount(); i++) {
		const auto& e = by.elements[i];
		double sc = e.GetAverageScore();
		year.elements.Set(i, 0, Capitalize(e.key));
		year.elements.Set(i, 1, e.value);
		year.elements.Set(i, 2, sc);
		score_sum += sc;
	}
	year.elements.SetCount(by.elements.GetCount());
	year.elements.SetSortColumn(2,true);
	
	if (by.elements.GetCount() > 0) {
		double score_av = score_sum / by.elements.GetCount();
		year.score.SetLabel(Format("Score: %.2n", score_av));
	}
	else year.score.SetLabel("");
	
}

void BiographyCtrl::UpdateElementHints() {
	for(int i = 0; i < element_hints.GetCount(); i++) {
		year.elements.Set(i, 0, Capitalize(element_hints.GetKey(i)));
		year.elements.Set(i, 1, element_hints[i]);
		year.elements.Set(i, 2, Value());
	}
	year.elements.SetCount(element_hints.GetCount());
}

void BiographyCtrl::OnValueChange() {
	DatasetPtrs mp = GetDataset();
	
	if (!mp.profile || !categories.IsCursor() || !years.IsCursor())
		return;
	
	TODO
	#if 0
	if (!mp.editable_biography)
		return;
	#endif
	
	Owner& owner = *mp.owner;
	Profile& profile = *mp.profile;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	
	BioYear& by = bcat.years[year_i];
	by.keywords = year.keywords.GetData();
	by.native_text = year.native_text.GetData();
	by.text = year.text.GetData();
	
	years.Set(year_i, 3, by.keywords);
	years.Set(year_i, 4, by.text);
}

void BiographyCtrl::MakeKeywords () {
	TODO
	#if 0
	if (!MetaPtrs::Single().editable_biography)
		return;
	TaskMgr& m = AiTaskManager();
	SocialArgs args;
	args.text = year.text.GetData();
	m.GetSocial(args, [this](String s) {PostCallback(THISBACK1(OnKeywords, s));});
	#endif
}

void BiographyCtrl::Translate() {
	TODO
	#if 0
	if (!MetaPtrs::Single().editable_biography)
		return;
	TaskMgr& m = AiTaskManager();
	
	String src = year.native_text.GetData();
	
	m.Translate("FI-FI", src, "EN-US", [this](String s) {PostCallback(THISBACK1(OnTranslate, s));});
	#endif
}

void BiographyCtrl::GetElements() {
	
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !categories.IsCursor() || !years.IsCursor())
		return;
	TODO
	#if 0
	if (!mp.editable_biography)
		return;
	
	Owner& owner = *mp.owner;
	Profile& profile = *mp.profile;
	Biography& biography = *mp.biography;
	int cat_enum = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_enum);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	
	
	BioYear& by = bcat.years[year_i];
	
	BiographyProcessArgs args;
	args.fn = 0;
	args.category = GetBiographyCategoryEnum(cat_enum);
	args.text = by.text;
	args.year = by.year;
	TaskMgr& m = AiTaskManager();
	
	auto* snap_ptr = mp.release;
	auto* by_ptr = &by;
	m.GetBiography(args, [this, snap_ptr, by_ptr](String result) {
		RemoveEmptyLines3(result);
		Vector<String> lines = Split(result, "\n");
		
		snap_ptr->last_modified = GetSysTime();
		
		by_ptr->elements.Clear();
		for (String& line : lines) {
			int a = line.Find(":");
			if (a < 0) continue;
			String key = ToLower(TrimBoth(line.Left(a)));
			String value = TrimBoth(line.Mid(a+1));
			RemoveQuotes(value);
			String lvalue = ToLower(value);
			int i = by_ptr->FindElement(key);
			if (lvalue.IsEmpty() || lvalue == "none." || lvalue == "none" || lvalue.Left(6) == "none (") {
				if (i >= 0)
					by_ptr->elements.Remove(i);
				continue;
			}
			if (i < 0) {
				i = by_ptr->elements.GetCount();
				by_ptr->elements.Add();
			}
			auto& el = by_ptr->elements[i];
			el.key = key;
			el.value = value;
		}
		
		PostCallback(THISBACK(UpdateElements));
	});
	#endif
}

void BiographyCtrl::GetElementHints() {
	TODO
	#if 0
	
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !categories.IsCursor() || !years.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Profile& profile = *mp.profile;
	Biography& biography = *mp.biography;
	int cat_enum = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_enum);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	
	BioYear& by = bcat.years[year_i];
	
	BiographyProcessArgs args;
	args.fn = 1;
	args.category = GetBiographyCategoryEnum(cat_enum);
	args.text = by.text;
	args.year = by.year;
	TaskMgr& m = AiTaskManager();
	
	auto* snap_ptr = mp.release;
	auto* by_ptr = &by;
	m.GetBiography(args, [this, snap_ptr, by_ptr](String result) {
		RemoveEmptyLines3(result);
		Vector<String> lines = Split(result, "\n");
		
		snap_ptr->last_modified = GetSysTime();
		
		element_hints.Clear();
		for (String& line : lines) {
			int a = line.Find(":");
			if (a < 0) continue;
			String key = ToLower(TrimBoth(line.Left(a)));
			String value = TrimBoth(line.Mid(a+1));
			RemoveQuotes(value);
			String lvalue = ToLower(value);
			if (lvalue.IsEmpty() || lvalue == "none." || lvalue == "none" || lvalue.Left(6) == "none (" || lvalue == "ready." || lvalue == "ready" || lvalue.Left(6) == "ready (" || lvalue == "n/a" || lvalue == "none mentioned.")
				continue;
			element_hints.GetAdd(key) = value;
		}
		
		PostCallback(THISBACK(UpdateElementHints));
	});
	#endif
}

void BiographyCtrl::GetElementScores() {
	TODO
	#if 0
	
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !categories.IsCursor() || !years.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Profile& profile = *mp.profile;
	Biography& biography = *mp.biography;
	int cat_enum = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_enum);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	
	BioYear& by = bcat.years[year_i];
	
	if (by.elements.IsEmpty())
		return;
	
	BiographyProcessArgs args;
	args.fn = 2;
	args.category = GetBiographyCategoryEnum(cat_enum);
	args.text = by.JoinElementMap(": ", "\n");
	args.year = by.year;
	TaskMgr& m = AiTaskManager();
	
	auto* snap_ptr = mp.release;
	auto* by_ptr = &by;
	m.GetBiography(args, [this, snap_ptr, by_ptr](String result) {
		RemoveEmptyLines3(result);
		Vector<String> lines = Split(result, "\n");
		
		snap_ptr->last_modified = GetSysTime();
		
		for (String& line : lines) {
			int a = line.Find(":");
			if (a < 0) continue;
			String key = ToLower(TrimBoth(line.Left(a)));
			String value = TrimBoth(line.Mid(a+1));
			int i = by_ptr->FindElement(key);
			if (i < 0)
				continue;
			auto& el = by_ptr->elements[i];
			
			Vector<String> scores = Split(value, ",");
			int j = -1;
			el.ResetScore();
			for (String& s : scores) {
				j++;
				Vector<String> parts = Split(s, ":");
				if (parts.GetCount() != 2)
					continue;
				int sc = ScanInt(TrimLeft(parts[1]));
				el.scores[j] = sc;
			}
		}
		
		PostCallback(THISBACK(UpdateElements));
	});
	#endif
}

void BiographyCtrl::OnTranslate(String s) {
	year.text.SetData(s);
	OnValueChange();
}

void BiographyCtrl::OnKeywords(String s) {
	RemoveEmptyLines2(s);
	Vector<String> parts = Split(s, "\n");
	s = Join(parts, ", ");
	year.keywords.SetData(s);
	OnValueChange();
}

void BiographyCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Translate"), TextImgs::BlueRing(), THISBACK(Translate)).Key(K_F5);
	bar.Add(t_("Make keywords"), TextImgs::BlueRing(), THISBACK(MakeKeywords)).Key(K_F6);
	bar.Add(t_("Get elements"), TextImgs::BlueRing(), THISBACK(GetElements)).Key(K_F7);
	bar.Add(t_("Get element hints"), TextImgs::BlueRing(), THISBACK(GetElementHints)).Key(K_F8);
	bar.Add(t_("Get element scores"), TextImgs::BlueRing(), THISBACK(GetElementScores)).Key(K_F9);
	bar.Separator();
	bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
}

void BiographyCtrl::Do(int fn) {
	TODO
	#if 0
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.release)
		return;
	if (!mp.editable_biography) {
		PromptOK(t_("Only the latest (and editable) revision can be processed. Select the latest revision."));
		return;
	}
	BiographyProcess& sdi = BiographyProcess::Get(*mp.profile, *mp.snap);
	prog.Attach(sdi);
	sdi.WhenRemaining << [this](String s) {PostCallback([this,s](){remaining.SetLabel(s);});};
	if (fn == 0)
		sdi.Start();
	else
		sdi.Stop();
	#endif
}

void BiographyCtrl::EntryListMenu(Bar& bar) {
	
}


END_UPP_NAMESPACE
