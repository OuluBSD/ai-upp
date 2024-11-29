#include "TextCtrl.h"

NAMESPACE_UPP

EntityEditorCtrl::EntityEditorCtrl() {
	AddMenu();
	Add(hsplit.SizePos());
	
	hsplit.Horz() << lsplit << comp_place;
	hsplit.SetPos(2000);
	lsplit.Vert() << entlist << complist;
	
	entlist.AddColumn("Entity");
	entlist.AddColumn("Components");
	entlist.ColumnWidths("3 1");
	entlist.WhenCursor = THISBACK(DataEntity);
	entlist.WhenBar = [this](Bar& b) {
		b.Add("Add entity", THISBACK(AddEntity));
		if (entlist.IsCursor())
			b.Add("Remove entity", THISBACK(RemoveEntity));
	};
	
	complist.AddColumn("Component");
	complist.AddColumn("Kind");
	complist.ColumnWidths("3 2");
	complist.WhenCursor = THISBACK(DataComponent);
	complist.WhenBar = [this](Bar& b) {
		b.Add("Add component", THISBACK(AddComponent));
		if (complist.IsCursor())
			b.Add("Remove component", THISBACK(RemoveComponent));
	};
	
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
		e_comps = e.node->FindAll<Component>();
		
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
	auto& e = *entities[ent_i];
	auto& comps = components[ent_i];
	comps = e.node->FindAll<Component>();
	int row = 0;
	for(int i = 0; i < comps.GetCount(); i++) {
		auto& cp = comps[i];
		if (!cp) continue;
		auto& c = *cp;
		complist.Set(row, 0, c.name);
		complist.Set(row, 1, c.node->GetKindString());
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

void EntityEditorCtrl::OnLoad(const String& data, const String& filepath) {
	MetaEnv().LoadFileRootJson("", filepath, data, true);
}

void EntityEditorCtrl::OnSave(String& data, const String& filepath) {
	MetaSrcFile& file = RealizeFileRoot();
	file.MakeTempFromEnv(false);
	data = file.StoreJson();
}

MetaSrcFile& EntityEditorCtrl::RealizeFileRoot() {
	MetaEnvironment& env = MetaEnv();
	String path = this->GetFilePath();
	MetaSrcFile& file = env.ResolveFile("", path);
	MetaSrcPkg& pkg = *file.pkg;
	ASSERT(file.id >= 0);
	MetaNode& n = env.RealizeFileNode(pkg.id, file.id, METAKIND_ECS_SPACE);
	this->file_root = &n;
	return file;
}

void EntityEditorCtrl::AddEntity() {
	RealizeFileRoot();
	MetaNode& n = *file_root;
	Entity& e = n.Add<Entity>();
	ASSERT(e.node->kind == METAKIND_ECS_ENTITY);
	e.name = "Unnamed";
	PostCallback(THISBACK(Data));
}

void EntityEditorCtrl::RemoveEntity() {
	if (!entlist.IsCursor()) return;
	MetaNode& n = *file_root;
	int ent_i = entlist.GetCursor();
	if (ent_i >= 0 && ent_i < n.sub.GetCount())
		n.sub.Remove(ent_i);
	PostCallback(THISBACK(Data));
}

Entity* EntityEditorCtrl::GetSelectedEntity() {
	if (!entlist.IsCursor()) return 0;
	MetaNode& n = *file_root;
	int ent_i = entlist.GetCursor();
	if (ent_i >= 0 && ent_i < entities.GetCount()) {
		return entities[ent_i];
	}
	return 0;
}

void EntityEditorCtrl::AddComponent() {
	Entity* e = GetSelectedEntity();
	if (!e) return;
	String title = "Add component";
	WithComponentSelection<TopWindow> dlg;
	CtrlLayoutOKCancel(dlg, title);
	Vector<int> list;
	for(int i = 0; i < MetaExtFactory::List().GetCount(); i++) {
		auto& cf = MetaExtFactory::List()[i];
		if (IsEcsComponentKind(cf.kind)) {
			list.Add(i);
			dlg.complist.Add(cf.name);
		}
	}
	if (dlg.complist.GetCount() == 0) return;
	dlg.complist.SetIndex(0);
	if(dlg.Execute() == IDOK) {
		int i = dlg.complist.GetIndex();
		int comp_i = list[i];
		if (comp_i < 0 || comp_i >= MetaExtFactory::List().GetCount()) return;
		const auto& factory = MetaExtFactory::List()[comp_i];
		auto& comp = e->node->Add(factory.kind);
		ASSERT(comp.kind == factory.kind);
		PostCallback(THISBACK(Data));
	}
}

void EntityEditorCtrl::RemoveComponent() {
	Entity* e = GetSelectedEntity();
	if (!e || !complist.IsCursor()) return;
	int comp_i = complist.GetCursor();
	if (comp_i >= 0 && comp_i < e->node->sub.GetCount())
		e->node->sub.Remove(comp_i);
	PostCallback(THISBACK(Data));
}

void EntityEditorCtrl::Do(int i) {
	
	//DatasetPtrs& p = GetDataset();
	//p.file_root = file_root;
	
	if (i == 0) {
		
		
		
	}
	
}

END_UPP_NAMESPACE
