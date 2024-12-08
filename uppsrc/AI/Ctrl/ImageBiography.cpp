#include "Ctrl.h"

NAMESPACE_UPP


ImageBiographyCtrl::ImageBiographyCtrl() {
	Add(hsplit.VSizePos(0,20).HSizePos());
	Add(prog.BottomPos(0,20).HSizePos(300));
	Add(remaining.BottomPos(0,20).LeftPos(0,300));
	
	hsplit.Horz() << categories << vsplit;
	hsplit.SetPos(1500, 0);
	hsplit.SetPos(3000, 1);
	
	vsplit.Vert() << years << bsplit;
	vsplit.SetPos(3333);
	
	bsplit.Horz() << entries << year << img;
	bsplit.SetPos(1500,0);
	bsplit.SetPos(5000,1);
	
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
	categories.WhenCursor << THISBACK(OnCategoryCursor);
	
	
	years.AddColumn(t_("Year"));
	years.AddColumn(t_("Age"));
	years.AddColumn(t_("Class"));
	years.AddColumn(t_("Images"));
	years.AddColumn(t_(""));
	years.AddIndex("IDX");
	years.ColumnWidths("1 1 1 1 15");
	years.WhenCursor << THISBACK(DataYear);
	
	
	entries.AddColumn(t_("#"));
	entries.AddColumn(t_("Time"));
	entries.AddColumn(t_("Keywords"));
	entries.AddIndex("IDX");
	entries.ColumnWidths("1 2 2");
	entries.WhenCursor << THISBACK(DataEntry);
	entries.WhenBar << THISBACK(EntryListMenu);
	
	year.time <<= THISBACK(OnValueChange);
	year.native_text <<= THISBACK(OnValueChange);
	year.text <<= THISBACK(OnValueChange);
	year.keywords <<= THISBACK(OnValueChange);
	year.image_text <<= THISBACK(OnValueChange);
	year.image_keywords <<= THISBACK(OnValueChange);
	
}

void ImageBiographyCtrl::Data() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography || !mp.analysis) {
		for(int i = 0; i < categories.GetCount(); i++) {
			categories.Set(i, 0, 0);
			categories.Set(i, 2, 0);
		}
		return;
	}
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	
	Index<int> req_cats = mp.analysis->GetRequiredCategories();
	for(int i = 0; i < categories.GetCount(); i++) {
		int cat_i = categories.Get(i, "IDX");
		bool req = req_cats.Find(cat_i) >= 0;
		categories.Set(i, 0, req ? "X" : "");
		BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
		int c = bcat.GetFilledImagesCount();
		categories.Set(i, 2, c > 0 ? Value(c) : Value());
	}
	DataCategory();
}

void ImageBiographyCtrl::DataCategory() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography || !categories.IsCursor()) {
		years.Clear();
		return;
	}
	Owner& owner = *mp.owner;
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
		int c = by.images.GetCount();
		years.Set(i, 3, c > 0 ? Value(c) : Value());
		years.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR(years);
	years.SetSortColumn(0, false);
	years.SetCount(bcat.years.GetCount());
	if (years.GetCount() && !years.IsCursor())
		years.SetCursor(0);
	
	DataYear();
}

void ImageBiographyCtrl::DataYear() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography || !categories.IsCursor() || !years.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	BioYear& by = bcat.years[year_i];
	
	for(int i = 0; i < by.images.GetCount(); i++) {
		const BioImage& bimg = by.images[i];
		entries.Set(i, 0, i);
		entries.Set(i, "IDX", i);
		entries.Set(i, 1, bimg.time);
		entries.Set(i, 2, bimg.image_keywords);
	}
	INHIBIT_CURSOR(entries);
	entries.SetSortColumn(0, false);
	entries.SetCount(by.images.GetCount());
	if (entries.GetCount() && !entries.IsCursor())
		entries.SetCursor(0);
	
	
	DataEntry();
}

void ImageBiographyCtrl::DataEntry() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !categories.IsCursor() || !years.IsCursor() || !entries.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	BioYear& by = bcat.years[year_i];
	int entry_i = entries.Get("IDX");
	BioImage& bimg = by.images[entry_i];
	
	
	year.time.SetData(bimg.time);
	year.native_text.SetData(bimg.native_text);
	year.text.SetData(bimg.text);
	year.keywords.SetData(bimg.keywords);
	year.image_text.SetData(bimg.image_text);
	year.image_keywords.SetData(bimg.image_keywords);
	
	if (bimg.image_hash) {
		String path = CacheImageFile(bimg.image_hash);
		if (!FileExists(path))
			path = ThumbnailImageFile(bimg.image_hash);
		Image i = StreamRaster::LoadFileAny(path);
		this->img.SetImage(i);
	}
	else {
		this->img.Clear();
	}
}

void ImageBiographyCtrl::OnCategoryCursor() {
	DataCategory();
	for(int i = years.GetCount()-1; i >= 0; i--) {
		int count = years.Get(i, 3);
		if (count > 0) {
			years.SetCursor(i);
			return;
		}
	}
	img.Clear();
}

void ImageBiographyCtrl::OnValueChange() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography || !categories.IsCursor() || !years.IsCursor() || !entries.IsCursor())
		return;
	if (!mp.editable_biography)
		return;
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	mp.snap->last_modified = GetSysTime();
	
	BioYear& by = bcat.years[year_i];
	int entry_i = entries.Get("IDX");
	BioImage& bimg = by.images[entry_i];
	
	bimg.time = year.time.GetData();
	bimg.native_text = year.native_text.GetData();
	bimg.text = year.text.GetData();
	bimg.keywords = year.keywords.GetData();
	bimg.image_keywords = year.image_keywords.GetData();
	bimg.image_text = year.image_text.GetData();
	
	entries.Set(1, bimg.time);
	entries.Set(2, bimg.image_keywords);
}

void ImageBiographyCtrl::MakeKeywords(int fn) {
	TaskMgr& m = TaskMgr::Single();
	SocialArgs args;
	if (fn == 0)
		args.text = year.text.GetData();
	else
		args.text = year.image_text.GetData();
	m.GetSocial(args, [this,fn](String s) {PostCallback(THISBACK2(OnKeywords, fn, s));});
}

void ImageBiographyCtrl::Translate() {
	TaskMgr& m = TaskMgr::Single();
	
	String src = year.native_text.GetData();
	
	m.Translate("FI-FI", src, "EN-US", [this](String s) {PostCallback(THISBACK1(OnTranslate, s));});
}

void ImageBiographyCtrl::OnTranslate(String s) {
	year.text.SetData(s);
	OnValueChange();
}

void ImageBiographyCtrl::OnKeywords(int fn, String s) {
	RemoveEmptyLines2(s);
	Vector<String> parts = Split(s, "\n");
	s = Join(parts, ", ");
	if (fn == 0)
		year.keywords.SetData(s);
	else
		year.image_keywords.SetData(s);
	OnValueChange();
}

void ImageBiographyCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Start"), AppImg::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), AppImg::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	bar.Separator();
	bar.Add(t_("Translate"), AppImg::BlueRing(), THISBACK(Translate));
	bar.Add(t_("Make keywords"), AppImg::BlueRing(), THISBACK1(MakeKeywords, 0));
	bar.Add(t_("Make keywords (image)"), AppImg::BlueRing(), THISBACK1(MakeKeywords, 1));
	bar.Separator();
	bar.Add(t_("Paste Image path"), AppImg::BlueRing(), THISBACK(PasteImagePath)).Key(K_CTRL_V);
	bar.Separator();
	bar.Add(t_("Analyse image"), AppImg::RedRing(), THISBACK(AnalyseImage)).Key(K_F7);
}

void ImageBiographyCtrl::EntryListMenu(Bar& bar) {
	bar.Add(t_("Add Entry"), AppImg::BlueRing(), THISBACK(AddEntry)).Key(K_CTRL_T);
	bar.Add(t_("Remove Entry"), AppImg::BlueRing(), THISBACK(RemoveEntry)).Key(K_CTRL|K_SHIFT|K_W);
	
}

void ImageBiographyCtrl::Do(int fn) {
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.profile || !mp.snap)
		return;
	if (mp.editable_biography) {
		PromptOK(t_("The latest (and editable) revision can't be processed. Select older than latest revision."));
		return;
	}
	
	ImageBiographyProcess& ss = ImageBiographyProcess::Get(*mp.profile, *mp.snap);
	if (fn == 0) {
		ss.Start();
	}
	else if (fn == 1) {
		ss.Stop();
	}
}

void ImageBiographyCtrl::AddEntry() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography || !categories.IsCursor() || !years.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	BioYear& by = bcat.years[year_i];
	
	by.images.Add();
	DataYear();
	if (entries.GetCount())
		entries.SetCursor(entries.GetCount()-1);
}

void ImageBiographyCtrl::RemoveEntry() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography || !categories.IsCursor() || !years.IsCursor() || !entries.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	BioYear& by = bcat.years[year_i];
	int entry_i = entries.Get("IDX");
	if (entry_i >= 0 && entry_i < by.images.GetCount())
		by.images.Remove(entry_i);
	DataYear();
}

void ImageBiographyCtrl::PasteImagePath() {
	Image img = ReadClipboardImage();
	if (!img.IsEmpty()) {
		SetCurrentImage(img);
	}
	else {
		String path = ReadClipboardText();
		if (path.Left(7) == "file://") {
			path = path.Mid(7);
			img = StreamRaster::LoadFileAny(path);
			if (!img.IsEmpty())
				SetCurrentImage(img);
		}
	}
}

void ImageBiographyCtrl::SetCurrentImage(Image img) {
	this->img.SetImage(img);
	
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography || !categories.IsCursor() || !years.IsCursor() || !entries.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	BioYear& by = bcat.years[year_i];
	int entry_i = entries.Get("IDX");
	BioImage& bimg = by.images[entry_i];
	
	hash_t h = img.GetHashValue();
	String cache_path = CacheImageFile(h);
	String thumb_path = ThumbnailImageFile(h);
	String full_path = FullImageFile(h);
	RealizeDirectory(GetFileDirectory(cache_path));
	RealizeDirectory(GetFileDirectory(thumb_path));
	RealizeDirectory(GetFileDirectory(full_path));
	
	if (!FileExists(cache_path)) {
		Image small_img = RescaleToFit(img, 1024);
		JPGEncoder enc(98);
		enc.SaveFile(cache_path, small_img);
	}
	
	if (!FileExists(thumb_path)) {
		Image thumb_img = RescaleToFit(img, 128);
		JPGEncoder enc(98);
		enc.SaveFile(thumb_path, thumb_img);
	}
	
	if (!FileExists(full_path)) {
		JPGEncoder enc(100);
		enc.SaveFile(full_path, img);
	}
	
	bimg.image_hash = h;
}

void ImageBiographyCtrl::AnalyseImage() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography || !categories.IsCursor() || !years.IsCursor() || !entries.IsCursor())
		return;
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	int year_i = years.Get("IDX");
	if (year_i >= bcat.years.GetCount()) return;
	BioYear& by = bcat.years[year_i];
	int entry_i = entries.Get("IDX");
	BioImage& bimg = by.images[entry_i];
	
	if (!bimg.image_hash)
		return;
	
	String path = CacheImageFile(bimg.image_hash);
	if (!FileExists(path))
		path = ThumbnailImageFile(bimg.image_hash);
	/*Image i = StreamRaster::LoadFileAny(path);
	if (i.IsEmpty())
		return;*/
	String jpeg = LoadFile(path);
	if (jpeg.IsEmpty())
		return;
	
	TaskMgr& m = TaskMgr::Single();
	
	VisionArgs args;
	args.fn = 0;
	//args.img = i;
	
	m.GetVision(jpeg, args, [this](String s_) {
		PostCallback([this,s_](){
			String s = TrimBoth(s_);
			if (s.Left(1) == "\"") s = s.Mid(1);
			if (s.Right(1) == "\"") s = s.Left(s.GetCount()-1);
			year.image_text.SetData(s);
			OnValueChange();
		});
	});
}


END_UPP_NAMESPACE
