#include "Ctrl.h"

NAMESPACE_UPP


ImageBiographyCtrl::ImageBiographyCtrl() {
	Add(hsplit.VSizePos(0,20).HSizePos());
	
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
	
	DatasetPtrs mp = GetDataset();
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
	
	DatasetPtrs mp = GetDataset();
	if (!mp.owner || !mp.biography || !categories.IsCursor()) {
		years.Clear();
		return;
	}
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
	
	Date today = GetSysDate();
	for(int i = 0; i < bcat.years.GetCount(); i++) {
		const BioYear& by = bcat.years[i];
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
	
	DatasetPtrs mp = GetDataset();
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
	
	DatasetPtrs mp = GetDataset();
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
	
	TODO
	#if 0
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
	#endif
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
	
	DatasetPtrs mp = GetDataset();
	if (!mp.owner || !mp.biography || !categories.IsCursor() || !years.IsCursor() || !entries.IsCursor())
		return;
	TODO
	#if 0
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
	#endif
}

void ImageBiographyCtrl::MakeKeywords(int fn) {
	TODO
	#if 0
	TaskMgr& m = AiTaskManager();
	SocialArgs args;
	if (fn == 0)
		args.text = year.text.GetData();
	else
		args.text = year.image_text.GetData();
	m.GetSocial(args, [this,fn](String s) {PostCallback(THISBACK2(OnKeywords, fn, s));});
	#endif
}

void ImageBiographyCtrl::Translate() {
	TaskMgr& m = AiTaskManager();
	
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
	bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	bar.Separator();
	bar.Add(t_("Translate"), TextImgs::BlueRing(), THISBACK(Translate));
	bar.Add(t_("Make keywords"), TextImgs::BlueRing(), THISBACK1(MakeKeywords, 0));
	bar.Add(t_("Make keywords (image)"), TextImgs::BlueRing(), THISBACK1(MakeKeywords, 1));
	bar.Separator();
	bar.Add(t_("Paste Image path"), TextImgs::BlueRing(), THISBACK(PasteImagePath)).Key(K_CTRL_V);
	bar.Separator();
	bar.Add(t_("Analyse image"), TextImgs::RedRing(), THISBACK(AnalyseImage)).Key(K_F7);
}

void ImageBiographyCtrl::EntryListMenu(Bar& bar) {
	bar.Add(t_("Add Entry"), TextImgs::BlueRing(), THISBACK(AddEntry)).Key(K_CTRL_T);
	bar.Add(t_("Remove Entry"), TextImgs::BlueRing(), THISBACK(RemoveEntry)).Key(K_CTRL|K_SHIFT|K_W);
	
}

void ImageBiographyCtrl::Do(int fn) {
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.release)
		return;
	if (mp.editable_biography) {
		PromptOK(t_("The latest (and editable) revision can't be processed. Select older than latest revision."));
		return;
	}
	
	ImageBiographyProcess& ss = ImageBiographyProcess::Get(*mp.profile, *mp.snap);
	TODO
	#if 0
	if (fn == 0) {
		ss.Start();
	}
	else if (fn == 1) {
		ss.Stop();
	}
	#endif
}

void ImageBiographyCtrl::AddEntry() {
	
	DatasetPtrs mp = GetDataset();
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
	
	DatasetPtrs mp = GetDataset();
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
	
	
	DatasetPtrs mp = GetDataset();
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
	
	TODO
	#if 0
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
	#endif
}

void ImageBiographyCtrl::AnalyseImage() {
	DatasetPtrs mp = GetDataset();
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
	
	TODO
	#if 0
	String path = CacheImageFile(bimg.image_hash);
	if (!FileExists(path))
		path = ThumbnailImageFile(bimg.image_hash);
	/*Image i = StreamRaster::LoadFileAny(path);
	if (i.IsEmpty())
		return;*/
	String jpeg = LoadFile(path);
	if (jpeg.IsEmpty())
		return;
	
	TaskMgr& m = AiTaskManager();
	
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
	#endif
}


















ImageViewerCtrl::ImageViewerCtrl() {
	
}

void ImageViewerCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	
	d.DrawRect(sz, Black());
	
	if (!img.IsEmpty()) {
		Size orig_sz = img.GetSize();
		double orig_ratio = (double)orig_sz.cx / orig_sz.cy;
		double new_ratio = (double)sz.cx / sz.cy;
		Size new_sz;
		if (orig_ratio < new_ratio) {
			new_sz.cy = sz.cy;
			new_sz.cx = (int)(sz.cy * orig_ratio);
		}
		else {
			new_sz.cx = sz.cx;
			new_sz.cy = (int)(sz.cx / orig_ratio);
		}
		
		Image scaled_img = CachedRescale(img, new_sz, FILTER_BILINEAR);
		
		if (orig_ratio < new_ratio) {
			int off = (sz.cx - new_sz.cx) / 2;
			d.DrawImage(off,0,scaled_img);
		}
		else {
			int off = (sz.cy - new_sz.cy) / 2;
			d.DrawImage(0,off,scaled_img);
		}
	}
}

void ImageViewerCtrl::SetImage(const Image& i) {
	img = i;
	PostCallback([this](){Refresh();});
}

void ImageViewerCtrl::Clear() {
	img.Clear();
	PostCallback([this](){Refresh();});
}

void ImageViewerCtrl::Menu(Bar& menu) {
	TODO
	#if 0
	menu.Add("Save Image as", [this]() {
		String file = SelectFileSaveAs("*.jpg\n*.*");
		
		if (file.GetCount()) {
			JPGEncoder jpg;
			jpg.Quality(100);
			jpg.SaveFile(file, img);
		}
	});
	menu.Add("Copy image to clipboard", [this]() {
		WriteClipboardImage(img);
	});
	#endif
}

void ImageViewerCtrl::RightDown(Point p, dword keyflags) {
	MenuBar::Execute(THISBACK(Menu));
}











ImageBiographyProcess::ImageBiographyProcess() {
	
}

int ImageBiographyProcess::GetPhaseCount() const {
	return PHASE_COUNT;
}

int ImageBiographyProcess::GetBatchCount(int phase) const {
	switch (phase) {
		case PHASE_ANALYZE_IMAGE_BIOGRAPHY:			return max(1, vision_tasks.GetCount());
		default: return 1;
	}
}

int ImageBiographyProcess::GetSubBatchCount(int phase, int batch) const {
	return 1;
}

void ImageBiographyProcess::DoPhase() {
	switch (phase) {
		case PHASE_ANALYZE_IMAGE_BIOGRAPHY:			ProcessAnalyzeImageBiography(); return;
		default: return;
	}
}

ImageBiographyProcess& ImageBiographyProcess::Get(Profile& p, BiographyPerspectives& snap) {
	static ArrayMap<String, ImageBiographyProcess> arr;
	
	TODO
	#if 0
	String key = "PROFILE(" + p.name + "), REVISION(" + IntStr(snap.revision) + ")";
	ImageBiographyProcess& ts = arr.GetAdd(key);
	ts.owner = p.owner;
	ts.profile = &p;
	ts.snap = &snap;
	ASSERT(ts.owner);
	return ts;
	#endif
	return Single<ImageBiographyProcess>();
}

void ImageBiographyProcess::TraverseVisionTasks() {
	TODO
	#if 0
	Biography& biography = snap->data;
	for(int i = 0; i < BIOCATEGORY_COUNT; i++) {
		BiographyCategory& bcat = biography.GetAdd(*owner, i);
		for(int j = 0; j < bcat.years.GetCount(); j++) {
			BioYear& by = bcat.years[j];
			
			for(int k = 0; k < by.images.GetCount(); k++) {
				BioImage& bimg = by.images[k];
				if (phase == PHASE_ANALYZE_IMAGE_BIOGRAPHY && bimg.image_text.IsEmpty() && bimg.image_hash != 0) {
					String path = CacheImageFile(bimg.image_hash);
					if (!FileExists(path))
						path = ThumbnailImageFile(bimg.image_hash);
					String jpeg = LoadFile(path);
					if (!jpeg.IsEmpty()) {
						VisionTask& t = vision_tasks.Add();
						t.bimg = &bimg;
						t.jpeg = jpeg;
					}
				}
			}
		}
	}
	#endif
}

void ImageBiographyProcess::ProcessAnalyzeImageBiography() {
	TODO
	#if 0
	
	if (batch == 0) {
		vision_tasks.Clear();
		TraverseVisionTasks();
	}
	
	if (batch >= vision_tasks.GetCount()) {
		NextPhase();
		return;
	}
	
	const VisionTask& t = vision_tasks[batch];
	
	VisionArgs args;
	args.fn = 0;
	
	SetWaiting(1);
	TaskMgr& m = TaskMgr::Single();
	m.GetVision(t.jpeg, args, THISBACK(OnProcessAnalyzeImageBiography));
	
	#endif
}

void ImageBiographyProcess::OnProcessAnalyzeImageBiography(String res) {
	const VisionTask& t = vision_tasks[batch];
	
	String& s = t.bimg->image_text;
	s = TrimBoth(res);
	if (s.Left(1) == "\"") s = s.Mid(1);
	if (s.Right(1) == "\"") s = s.Left(s.GetCount()-1);
	
	TODO
	#if 0
	NextBatch();
	SetWaiting(0);
	#endif
}







INITIALIZER_COMPONENT_CTRL(ImageBiography, ImageBiographyCtrl)

END_UPP_NAMESPACE
