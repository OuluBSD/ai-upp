#include "Ctrl.h"
#include <ide/ide.h>

NAMESPACE_UPP


SolverBaseIndicator::SolverBaseIndicator() {
	ProgressIndicator::Set(0,1);
	
}

void SolverBaseIndicator::Attach(SolverBase& sb) {
	sb.WhenProgress << [this](int a, int t) {PostCallback(THISBACK2(SetProgress, a,t));};
}

void SolverBaseIndicator::SetProgress(int a, int t) {
	ProgressIndicator::Set(a, t);
}



ToolAppCtrl::~ToolAppCtrl() {
	if (TheIde()->addon_ctrl == this) {
		TheIde()->addon_menu.Clear();
		PostCallback([]{TheIde()->SetBar();});
	}
}

bool ToolAppCtrl::IsScript() const {
	const auto& p = GetDataset();
	return p.script;
}

bool ToolAppCtrl::HasPointers() const {
	// TODO remove this function
	return true;
}

Script& ToolAppCtrl::GetScript() {
	DatasetPtrs& p = GetDataset();
	if(!p.script)
		throw NoPointerExc("no scripts");
	return *p.script;
}

Component& ToolAppCtrl::GetComponent() {
	DatasetPtrs& p = GetDataset();
	if(!p.component || !p.entity)
		throw NoPointerExc("no song");
	return *p.component;
}

void ToolAppCtrl::AddMenu() {
	//AddFrame(menu);
	//ToolMenu(menu);
	UpdateMenu();
}

void ToolAppCtrl::UpdateMenu() {
	TheIde()->addon_ctrl = this;
	TheIde()->addon_menu = [this](Bar& b){ToolMenu(b);};
	PostCallback([]{TheIde()->SetBar();});
}

Entity& ToolAppCtrl::GetEntity() {
	DatasetPtrs& p = GetDataset();
	if(!p.entity)
		throw NoPointerExc("no artist");
	return *p.entity;
}

String ToolAppCtrl::GetComponentTitle() const {
	/*DatasetPtrs p = GetDataset();
	DatasetPtrs& p = GetDataset();
	if(!p.component || !p.entity)
		throw NoPointerExc("no song");
	Component& song = *p.component;
	Entity& artist = *p.entity;
	String s;
	s << artist.english_name << " - " << song.english_title;
	return s;*/
	TODO
	return "";
}

DatasetPtrs& ToolAppCtrl::GetDataset() const {
	TODO return DatasetPtrs::Single();
}

/*const Index<String>& ToolAppCtrl::GetTypeclasses() const {
	TODO static Index<String> i; return i;
}

const Vector<ContentType>& ToolAppCtrl::GetContents() const {
	TODO static Vector<ContentType> i; return i;
}

const Vector<String>& ToolAppCtrl::GetContentParts() const {
	TODO static Vector<String> i; return i;
}*/

/*int ToolAppCtrl::GetDataset() {
	DatasetPtrs p = GetDataset();
	DatasetPtrs& p = GetDataset();
	if(!p.component || !p.entity)
		throw NoPointerExc("no song");
	Component& song = *p.component;
	return ScanInt(song.data.Get("ATTR_DATASET", "0"));
}*/

void ToolAppCtrl::GetAttrs(const VectorMap<String,String>& data, VectorMap<String,String>& v) {
	for(int i = 0; i < ATTR_COUNT; i++) {
		const char* key = AttrKeys[i][0];
		int value = StrInt(data.Get(key, "0"));
		
		if (value) {
			if (value > 0) {
				v.GetAdd(key) = AttrKeys[i][2];
			}
			else {
				v.GetAdd(key) = AttrKeys[i][3];
			}
		}
	}
}

void ToolAppCtrl::MakeComponentParts(ArrayCtrl& parts) {
	/*Component& song = GetComponent();
	
	for(int i = 0; i < song.__parts.GetCount(); i++) {
		StaticPart& sp = song.__parts[i];
		//parts.Set(i, 0, sp.name);
		//parts.Set(i, 1, part.GetTypeString());
		parts.Set(i, 0,
			AttrText(sp.name).NormalPaper(GetComponentPartPaperColor(sp.type)));
		
		DropList& dl = parts.CreateCtrl<DropList>(i, 1);
		for(int j = 0; j < StaticPart::PART_TYPE_COUNT; j++)
			dl.Add(StaticPart::GetTypeString(j));
		dl.SetIndex((int&)sp.part_type);
		dl.WhenAction << [&dl,i,&sp]() {(int&)sp.part_type = dl.GetIndex();};
	}
	INHIBIT_CURSOR(parts);
	parts.SetCount(song.__parts.GetCount());
	if (!parts.IsCursor() && parts.GetCount())
		parts.SetCursor(0);*/
}

void ToolAppCtrl::Load(const String& includes, const String& filename, Stream& in, byte charset) {
	data = in.Get((int)in.GetSize());
	data_filepath = filename;
	OnLoad(data, filename);
}

void ToolAppCtrl::Save(Stream& s, byte charset) {
	String new_data;
	OnSave(new_data, data_filepath);
	if (!new_data.IsEmpty())
		data = new_data;
	s.Put(data);
}
	
END_UPP_NAMESPACE
