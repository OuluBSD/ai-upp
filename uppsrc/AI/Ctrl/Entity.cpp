#include "Ctrl.h"

NAMESPACE_UPP


DatasetPtrs ComponentCtrl::GetDataset() {
	if (!ext)
		return DatasetPtrs();
	Component& comp = dynamic_cast<Component&>(*ext);
	return comp.GetDataset();
}

Script& ComponentCtrl::GetScript() {
	return *GetDataset().script; // TODO fix: unsafe
}

const Index<String>& ComponentCtrl::GetTypeclasses() const {
	TODO static Index<String> i; return i;
}

const Vector<ContentType>& ComponentCtrl::GetContents() const {
	TODO static Vector<ContentType> i; return i;
}

const Vector<String>& ComponentCtrl::GetContentParts() const {
	TODO static Vector<String> i; return i;
}

EntityEditorCtrl::EntityEditorCtrl() {
	AddMenu();
	Add(hsplit.SizePos());
	
	hsplit.Horz() << lsplit << ext_place;
	hsplit.SetPos(1000);
	lsplit.Vert() << entlist << extlist;
	
	entlist.AddColumn("Entity");
	entlist.AddColumn("Extensions");
	entlist.ColumnWidths("3 1");
	entlist.WhenCursor = THISBACK(DataEntity);
	entlist.WhenBar = [this](Bar& b) {
		b.Add("Add entity", THISBACK(AddEntity));
		if (entlist.IsCursor())
			b.Add("Remove entity", THISBACK(RemoveEntity));
	};
	
	extlist.AddColumn("Kind");
	extlist.AddColumn("Name");
	extlist.ColumnWidths("4 1");
	extlist.WhenCursor = THISBACK(DataExtension);
	extlist.WhenBar = [this](Bar& b) {
		b.Add("Add Component", THISBACK(AddComponent));
		if (extlist.IsCursor())
			b.Add("Remove Component", THISBACK(RemoveComponent));
	};
	
}

void EntityEditorCtrl::SetExtensionCtrl(int kind, MetaExtCtrl* c) {
	if (ext_ctrl) {
		ext_place.RemoveChild(&*ext_ctrl);
		ext_ctrl.Clear();
		ext_ctrl_kind = -1;
	}
	if (c) {
		ext_ctrl_kind = kind;
		c->owner = this;
		ext_ctrl.Attach(c);
		ext_place.Add(c->SizePos());
		PostCallback(THISBACK(DataExtCtrl));
		c->WhenEditorChange = THISBACK(DataExtCtrl);
	}
	UpdateMenu();
}

void EntityEditorCtrl::DataEntityListOnly() {
	int row = 0;
	for(int i = 0; i < entities.GetCount(); i++) {
		auto& ep = entities[i];
		if (!ep) continue;
		auto& e = *ep;
		auto& enode = e.node;
		entlist.Set(row, 0, enode.id);
		
		auto& e_exts = extensions[i];
		e_exts = e.node.GetAllExtensions();
		
		entlist.Set(row, 1, e_exts.GetCount());
		row++;
	}
}

void EntityEditorCtrl::Data() {
	
	RealizeFileRoot();
	
	if (!file_root) {
		entlist.Clear();
		extlist.Clear();
		ClearExtensionCtrl();
		return;
	}
	
	entities = this->file_root->FindAll<Entity>();
	
	extensions.SetCount(entities.GetCount());
	
	int row = 0;
	for(int i = 0; i < entities.GetCount(); i++) {
		auto& ep = entities[i];
		if (!ep) continue;
		auto& e = *ep;
		auto& enode = e.node;
		entlist.Set(row, 0, enode.id);
		
		auto& e_exts = extensions[i];
		e_exts = e.node.GetAllExtensions();
		
		entlist.Set(row, 1, e_exts.GetCount());
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
		extlist.Clear();
		ClearExtensionCtrl();
		return;
	}
	
	int ent_i = entlist.GetCursor();
	auto& e = *entities[ent_i];
	auto& exts = extensions[ent_i];
	exts = e.node.GetAllExtensions();
	exts.Insert(0, &e);
	int row = 0;
	for(int i = 0; i < exts.GetCount(); i++) {
		auto& cp = exts[i];
		if (!cp) continue;
		auto& c = *cp;
		extlist.Set(row, 0, c.node.GetKindString());
		extlist.Set(row, 1, c.GetName());
		row++;
	}
	extlist.SetCount(row);
	
	if (!extlist.IsCursor() && extlist.GetCount())
		extlist.SetCursor(0);
	else
		DataExtension();
}

void EntityEditorCtrl::DataExtension() {
	if (!entlist.IsCursor() || !extlist.IsCursor()) {
		ClearExtensionCtrl();
		return;
	}
	
	int ent_i = entlist.GetCursor();
	int ext_i = extlist.GetCursor();
	auto& e = *entities[ent_i];
	auto& exts = extensions[ent_i];
	if (ext_i < 0 || ext_i >= exts.GetCount())
		return;
	MetaNodeExt& ext = *exts[ext_i];
	
	if (ext_ctrl_kind != ext.node.kind) {
		int fac_i = MetaExtFactory::FindKindFactory(ext.node.kind);
		
		if (fac_i < 0) {
			ClearExtensionCtrl();
			return;
		}
		const auto& fac = MetaExtFactory::List()[fac_i];
		if (fac.new_ctrl_fn) {
			MetaExtCtrl* ctrl = fac.new_ctrl_fn();
			ctrl->ext = &ext;
			SetExtensionCtrl(ext.node.kind, ctrl);
			
			if (fac.kind == METAKIND_ECS_ENTITY) {
				EntityInfoCtrl& e = dynamic_cast<EntityInfoCtrl&>(*ctrl);
				e.WhenValueChange = THISBACK(DataEntityListOnly);
			}
		}
		else {
			ClearExtensionCtrl();
			return;
		}
	}
	else {
		ASSERT(ext_ctrl);
		ext_ctrl->ext = &ext;
	}
	DataExtCtrl();
}

void EntityEditorCtrl::DataExtCtrl() {
	if (ext_ctrl)
		ext_ctrl->Data();
}

void EntityEditorCtrl::SetFont(Font fnt) {
	
}

void EntityEditorCtrl::ToolMenu(Bar& bar) {
	if (ext_ctrl)
		ext_ctrl->ToolMenu(bar);
	else
		bar.Add("", Callback());
}

void EntityEditorCtrl::OnLoad(const String& data, const String& filepath) {
	MetaEnv().LoadFileRootJson(GetFileIncludes(), filepath, data, true);
}

void EntityEditorCtrl::OnSave(String& data, const String& filepath) {
	MetaSrcFile& file = RealizeFileRoot();
	file.MakeTempFromEnv(false);
	LOG("### ROOT ###");
	LOG(MetaEnv().root.GetTreeString());
	LOG("### Temp ###");
	LOG(file.temp->GetTreeString());
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
	ASSERT(e.node.kind == METAKIND_ECS_ENTITY);
	auto& enode = e.node;
	enode.id = "Unnamed";
	PostCallback(THISBACK(Data));
	PostCallback([this]{entlist.SetCursor(entlist.GetCount()-1);}); // select last entity
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
	String title = "Add Component";
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
	dlg.complist.SetFocus();
	if(dlg.Execute() == IDOK) {
		int i = dlg.complist.GetIndex();
		int ext_i = list[i];
		if (ext_i < 0 || ext_i >= MetaExtFactory::List().GetCount()) return;
		const auto& factory = MetaExtFactory::List()[ext_i];
		auto& ext = e->node.Add(factory.kind);
		ASSERT(ext.kind == factory.kind);
		PostCallback(THISBACK(Data));
		PostCallback([this]{extlist.SetCursor(extlist.GetCount()-1);}); // select last extension (component)
	}
}

void EntityEditorCtrl::RemoveComponent() {
	Entity* e = GetSelectedEntity();
	if (!e || !extlist.IsCursor()) return;
	int ext_i = extlist.GetCursor() - 1; // -1 --> don't remove EntityInfoCtrl
	if (ext_i >= 0 && ext_i < e->node.sub.GetCount())
		e->node.sub.Remove(ext_i);
	PostCallback(THISBACK(Data));
}

void EntityEditorCtrl::Do(int i) {
	
	//DatasetPtrs p = GetDataset();
	//p.file_root = file_root;
	
	if (i == 0) {
		
		
		
	}
	
}










EntityInfoCtrl::EntityInfoCtrl() {
	CtrlLayout(info);
	Add(info.SizePos());
	info.split.Horz() << data << value;
	info.split.SetPos(7500);
	
	data.AddColumn("Key");
	data.AddColumn("Value");
	data.AddIndex("IDX");
	data.ColumnWidths("1 4");
	data.WhenCursor = THISBACK(DataCursor);
	
	info.name.WhenAction = THISBACK(OnEdit);
	info.show_hidden_values.WhenAction = THISBACK(Data);
	value.WhenAction = THISBACK(OnEditValue);
}

void EntityInfoCtrl::Data() {
	Entity& ent = GetExt<Entity>();
	MetaNode& enode = ent.node;
	
	info.name = enode.id;
	
	// Realize some default fields
	ent.Data("description");
	ent.Data("gender");
	
	Vector<MetaNode*> envs = MetaEnv().FindAllEnvs();
	
	// Get all contexts
	all_ctxs.Clear();
	for (MetaNode* env : envs) {
		Vector<MetaNode*> ctxs = env->FindAllShallow(METAKIND_CONTEXT);
		for (MetaNode* ctx : ctxs) {
			String key = /*env->id + ": " +*/ ctx->id;
			all_ctxs.Add(key, ctx);
		}
	}
	
	// Entity's context
	String ent_ctx = ent.Data("ctx");
	String match_key = ent_ctx;
	
	
	// Entity data fields
	data.SetCount(0);
	bool show_hidden_values = info.show_hidden_values.Get();
	int row = 0;
	for(int i = 0; i < ent.data.GetCount(); i++) {
		String key = ent.data.GetKey(i);
		if (!show_hidden_values && key.GetCount() && key[0] == '.')
			continue;
		Value value = ent.data[i];
		data.Set(row, "IDX", i);
		data.Set(row, 0, key);
		if (key == "gender") {
			DropList* dl = new DropList;
			for (auto g : GetCategories())
				dl->Add(g);
			data.SetCtrl(row, 1, dl);
			int gender_i = max(0, FindCategory(value.ToString()));
			dl->SetIndex(gender_i);
			dl->WhenAction = [&ent,dl]{ent.data.GetAdd("gender") = GetCategoryString(dl->GetIndex());};
		}
		else if (key == "ctx") {
			DropList* dl = new DropList;
			data.SetCtrl(row, 1, dl);
			dl->Clear();
			dl->Add("");
			int active_ctx = 0;
			for(int i = 0; i < all_ctxs.GetCount(); i++) {
				String ctx_key = all_ctxs.GetKey(i);
				dl->Add(ctx_key);
				if (ctx_key == match_key)
					active_ctx = 1+i;
			}
			if (active_ctx >= 0 && active_ctx < dl->GetCount()) {
				dl->SetIndex(active_ctx);
			}
			dl->WhenAction = [this,&ent,dl]{
				int ctx_i = dl->GetIndex()-1;
				if (ctx_i >= 0 && ctx_i < dl->GetCount()) {
					MetaNode& ctx = *all_ctxs[ctx_i];
					ent.Data("ctx") = ctx.id;
				}
				else {
					ent.Data("ctx") = String();
				}
			};
		}
		else {
			data.Set(row, 1, value);
		}
		row++;
	}
	data.SetCount(row);
	data.SetSortColumn(0);
	
	if (!data.IsCursor())
		data.SetCursor(0);
	else
		DataCursor();
}

void EntityInfoCtrl::DataCursor() {
	if (!data.IsCursor()) {
		value.Clear();
		return;
	}
	
	Entity& ent = GetExt<Entity>();
	int data_i = data.Get("IDX");
	Value val = ent.data[data_i];
	value.SetData(val.ToString());
}

void EntityInfoCtrl::OnEdit() {
	Entity& e = GetExt<Entity>();
	auto& enode = e.node;
	enode.id = ~info.name;
		
	WhenValueChange();
}

void EntityInfoCtrl::OnEditValue() {
	if (!data.IsCursor())
		return;
	Entity& e = GetExt<Entity>();
	int data_i = data.Get("IDX");
	Value val = value.GetData();
	e.data[data_i] = val.ToString();
	data.Set(1, val);
}

void EntityInfoCtrl::ToolMenu(Bar& bar) {
	
}

DatasetPtrs EntityInfoCtrl::GetDataset() {
	DatasetPtrs p;
	MetaNode& n = GetNode();
	p.entity = &GetExt<Entity>();
	FillDataset(p, n, 0);
	return p;
}

INITIALIZER_COMPONENT_CTRL(Entity, EntityInfoCtrl)

END_UPP_NAMESPACE
