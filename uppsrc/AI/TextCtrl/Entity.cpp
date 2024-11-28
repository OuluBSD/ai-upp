#include "TextCtrl.h"

NAMESPACE_UPP

EntityEditorCtrl::EntityEditorCtrl() {
	Add(hsplit.SizePos());
	
	hsplit.Horz() << lsplit << comp_place;
	hsplit.SetPos(2000);
	lsplit.Vert() << entlist << complist;
	
	entlist.AddColumn("Entity");
	entlist.AddColumn("Components");
	entlist.ColumnWidths("3 1");
	entlist.WhenCursor = THISBACK(DataEntity);
	
	complist.AddColumn("Component");
	complist.AddColumn("");
	complist.ColumnWidths("3 1");
	complist.WhenCursor = THISBACK(DataComponent);
	
	
}

void EntityEditorCtrl::SetComponentCtrl(Ctrl* c) {
	
}

void EntityEditorCtrl::Data() {
	
	RealizeFileRoot();
	
	if (!file_root) {
		entlist.Clear();
		complist.Clear();
		SetComponentCtrl(0);
		return;
	}
	
	entities = this->file_root->FindAll<Entity>();
	
	components.SetCount(entities.GetCount());
	
	int row = 0;
	for(int i = 0; i < entities.GetCount(); i++) {
		auto& ep = entities[i];
		if (!ep) continue;
		auto& e = *ep;
		entlist.Set(row, 0, e.name);
		
		auto& e_comps = components[i];
		e_comps = e.FindAll<Component>();
		
		entlist.Set(row, 1, e_comps.GetCount());
		row++;
	}
	entlist.SetCount(row);
	
	if (!entlist.IsCursor() && entlist.GetCount())
		entlist.SetCursor(0);
	else
		DataEntity();
}

void EntityEditorCtrl::DataEntity() {
	if (!entlist.IsCursor()) {
		complist.Clear();
		SetComponentCtrl(0);
		return;
	}
	
	int ent_i = entlist.GetCursor();
	auto& comps = components[ent_i];
	
	int row = 0;
	for(int i = 0; i < comps.GetCount(); i++) {
		auto& cp = comps[i];
		if (!cp) continue;
		auto& c = *cp;
		complist.Set(row, 0, c.name);
		complist.Set(row, 1, c.sub.GetCount());
		row++;
	}
	complist.SetCount(row);
	
	if (!complist.IsCursor() && complist.GetCount())
		complist.SetCursor(0);
	else
		DataComponent();
}

void EntityEditorCtrl::DataComponent() {
	if (!entlist.IsCursor()) {
		SetComponentCtrl(0);
		return;
	}
	
	
}

void EntityEditorCtrl::SetFont(Font fnt) {
	
}

void EntityEditorCtrl::ToolMenu(Bar& bar) {
	bar.Add("Test function 1", THISBACK1(Do, 0));
}

void EntityEditorCtrl::RealizeFileRoot() {
	MetaEnvironment& env = MetaEnv();
	String path = this->GetFilePath();
	MetaSrcPkg& pkg = env.ResolveFile("", path);
	int file_id = pkg.FindFile(path);
	ASSERT(file_id >= 0);
	MetaNode& n = env.RealizeFileNode(pkg.id, file_id, METAKIND_ECS_SPACE);
	this->file_root = &n;
}

void EntityEditorCtrl::Do(int i) {
	
	//DatasetPtrs& p = GetDataset();
	//p.file_root = file_root;
	
	if (i == 0) {
		
		
		
	}
	
}

END_UPP_NAMESPACE
