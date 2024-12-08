#include "ImgCtrl.h"

NAMESPACE_UPP


SnapCoverImage::SnapCoverImage() {
	this->Add(vsplit.SizePos());
	
	vsplit.Vert();
	for(int i = 0; i < 3; i++)
		vsplit << hsplit[i];
	
	hsplit[0].Horz() << attr_list << attr_text;
	hsplit[1].Horz() << sugg_list << sugg_text;
	hsplit[2].Horz();
	for(int i = 0; i < 4; i++)
		hsplit[2] << suggestion[i];
	hsplit[0].SetPos(2500);
	hsplit[1].SetPos(2500);
	
	attr_list.AddColumn(t_("Attribute"));
	//attr_list.Set(0, 0, t_("Lyrics combined"));
	//attr_list.Set(1, 0, t_("Lyric summaries combined"));
	for(int i = 0; i < SNAPANAL_COUNT; i++) {
		attr_list.Set(i, 0, GetSnapshotAnalysisKey(i));
	}
	attr_list.SetCursor(0);
	attr_list.WhenCursor << THISBACK(DataAttribute);
	attr_text.WhenAction << THISBACK(OnAttributeChange);
	
	sugg_list.AddColumn(t_("Cover suggestion"));
	sugg_list.WhenCursor << THISBACK(DataSuggestion);
	sugg_text.WhenAction << THISBACK(OnSuggestionChange);
	
	sugg_list.WhenBar << THISBACK(SuggestionMenu);
}

void SnapCoverImage::Data() {
	EditorPtrs& p = GetPointers();
	if (!p.release) return;
	
	for(int i = 0; i < p.release->cover_suggestions.GetCount(); i++) {
		sugg_list.Set(i, 0, p.release->cover_suggestions[i]);
	}
	INHIBIT_CURSOR(sugg_list);
	sugg_list.SetCount(p.release->cover_suggestions.GetCount());
	if (!sugg_list.IsCursor() && sugg_list.GetCount())
		sugg_list.SetCursor(0);
	
	DataAttribute();
	DataSuggestion();
}

void SnapCoverImage::DataAttribute() {
	EditorPtrs& p = GetPointers();
	if (!p.release) return;
	
	if (!attr_list.IsCursor())
		return;
	
	int attr_i = attr_list.GetCursor();
	if (attr_i < 0 || attr_i >= p.release->analysis.GetCount())
		return;
	
	attr_text.SetData(p.release->analysis[attr_i]);
}

void SnapCoverImage::DataSuggestion() {
	EditorPtrs& p = GetPointers();
	if (!p.release) return;
	
	if (!sugg_list.IsCursor())
		return;
	
	int sugg_i = sugg_list.GetCursor();
	if (sugg_i < 0 || sugg_i >= p.release->cover_suggestions.GetCount())
		return;
	
	sugg_text.SetData(p.release->cover_suggestions[sugg_i]);
	
	DataSuggestionImage();
}

void SnapCoverImage::DataSuggestionImage() {
	EditorPtrs& p = GetPointers();
	if (!p.release) return;
	
	if (!sugg_list.IsCursor())
		return;
	
	int sugg_i = sugg_list.GetCursor();
	if (sugg_i < 0 || sugg_i >= p.release->cover_suggestions.GetCount())
		return;
	
	String title = IntStr64(p.release->cover_suggestions[sugg_i].GetHashValue());
	
	//const String& dir = MetaDatabase::Single().dir;
	//const String& share = MetaDatabase::Single().share;
	//String img_dir = dir + DIR_SEPS + share + DIR_SEPS + GetAppModeDir() + DIR_SEPS + "images" + DIR_SEPS;
	//String img_dir = ConfigFile("images");
	String img_dir = AppendFileName(MetaDatabase::GetDirectory(), "images" DIR_SEPS "full");
	RealizeDirectory(img_dir);
	for(int i = 0; i < 4; i++) {
		String path = AppendFileName(img_dir, title + "_" + IntStr(i) + ".jpg");
		if (FileExists(path))
			suggestion[i].SetImage(StreamRaster::LoadFileAny(path));
		else
			suggestion[i].Clear();
	}
		
}

void SnapCoverImage::OnAttributeChange() {
	EditorPtrs& p = GetPointers();
	if (!p.release) return;
	
	if (!attr_list.IsCursor())
		return;
	
	int attr_i = attr_list.GetCursor();
	if (attr_i >= p.release->analysis.GetCount())
		p.release->analysis.SetCount(attr_i);
	
	p.release->analysis[attr_i] = attr_text.GetData();
}

void SnapCoverImage::OnSuggestionChange() {
	EditorPtrs& p = GetPointers();
	if (!p.release) return;
	
	if (!sugg_list.IsCursor())
		return;
	
	int sugg_i = sugg_list.GetCursor();
	if (sugg_i >= p.release->cover_suggestions.GetCount())
		p.release->cover_suggestions.SetCount(sugg_i);
	
	p.release->cover_suggestions[sugg_i] = sugg_text.GetData();
}

void SnapCoverImage::ToolMenu(Bar& bar) {
	bar.Add(t_("Make all images"), AppImg::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	
	/*bar.Add(t_("Create suggestions for prompts"), AppImg::Part(), THISBACK(CreateSuggestionsForPrompts)).Key(K_F5);
	bar.Add(t_("Make single image"), AppImg::Part(), THISBACK(MakeSingleImage)).Key(K_F6);
	bar.Add(t_("Make all images"), AppImg::Part(), THISBACK(MakeAllImages)).Key(K_F7);
	*/
}

void SnapCoverImage::Do(int fn) {
	EditorPtrs& p = GetPointers();
	if (!p.release) return;
	
	if (fn == 0) {
		SnapSolver& tm = SnapSolver::Get(*p.release, GetAppMode());
		tm.Start();
	}
}

void SnapCoverImage::CreateSuggestionsForPrompts() {
	TextDatabase& db = GetDatabase();
	EditorPtrs& p = GetPointers();
	if(!p.component || !p.entity || !p.release)
		return;
	
	Snapshot& rel = *p.release;
	
	
}

void SnapCoverImage::SuggestionMenu(Bar& bar) {
	bar.Add(t_("Add suggestion"), AppImg::RedRing(), [this]() {
		EditorPtrs& p = GetPointers();
		p.release->cover_suggestions.Add();
		PostCallback(THISBACK(Data));
	});
	
}


END_UPP_NAMESPACE
