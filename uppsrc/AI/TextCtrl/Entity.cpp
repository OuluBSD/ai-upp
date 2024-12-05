#include "TextCtrl.h"

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
	
	extlist.AddColumn("Extension");
	extlist.AddColumn("Kind");
	extlist.ColumnWidths("3 2");
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
		ext_ctrl.Attach(c);
		ext_place.Add(c->SizePos());
		PostCallback(THISBACK(DataExtCtrl));
	}
	UpdateMenu();
}

void EntityEditorCtrl::DataEntityListOnly() {
	int row = 0;
	for(int i = 0; i < entities.GetCount(); i++) {
		auto& ep = entities[i];
		if (!ep) continue;
		auto& e = *ep;
		entlist.Set(row, 0, e.name);
		
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
		entlist.Set(row, 0, e.name);
		
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
		extlist.Set(row, 0, c.GetName());
		extlist.Set(row, 1, c.node.GetKindString());
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
	MetaEnv().LoadFileRootJson("", filepath, data, true);
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
	if(dlg.Execute() == IDOK) {
		int i = dlg.complist.GetIndex();
		int ext_i = list[i];
		if (ext_i < 0 || ext_i >= MetaExtFactory::List().GetCount()) return;
		const auto& factory = MetaExtFactory::List()[ext_i];
		auto& ext = e->node.Add(factory.kind);
		ASSERT(ext.kind == factory.kind);
		PostCallback(THISBACK(Data));
	}
}

void EntityEditorCtrl::RemoveComponent() {
	Entity* e = GetSelectedEntity();
	if (!e || !extlist.IsCursor()) return;
	int ext_i = extlist.GetCursor();
	if (ext_i == 0) return; // don't remove EntityInfoCtrl
	if (ext_i >= 0 && ext_i < e->node.sub.GetCount())
		e->node.sub.Remove(ext_i);
	PostCallback(THISBACK(Data));
}

void EntityEditorCtrl::Do(int i) {
	
	//DatasetPtrs& p = GetDataset();
	//p.file_root = file_root;
	
	if (i == 0) {
		
		
		
	}
	
}










EntityInfoCtrl::EntityInfoCtrl() {
	Add(info.SizePos());
	CtrlLayout(info);
	
	info.name.WhenAction = THISBACK(OnEdit);
	info.ctx.WhenAction = THISBACK(OnEdit);
	info.desc.WhenAction = THISBACK(OnEdit);
}

void EntityInfoCtrl::Data() {
	DatasetPtrs p = GetDataset();
	info.name = p.entity->name;
	info.desc.SetData((String)p.entity->Data("description"));
	
	Vector<MetaNode*> envs = MetaEnv().FindAllEnvs();
	
	all_ctxs.Clear();
	for (MetaNode* env : envs) {
		Vector<MetaNode*> ctxs = env->FindAllShallow(METAKIND_CONTEXT);
		for (MetaNode* ctx : ctxs) {
			String key = /*env->id + ": " +*/ ctx->id;
			all_ctxs.Add(key, ctx);
		}
	}
	
	String ent_ctx = p.entity->Data("ctx");
	String match_key = ent_ctx;
	
	info.ctx.Clear();
	info.ctx.Add("");
	int active_ctx = 0;
	for(int i = 0; i < all_ctxs.GetCount(); i++) {
		String ctx_key = all_ctxs.GetKey(i);
		info.ctx.Add(ctx_key);
		if (ctx_key == match_key)
			active_ctx = 1+i;
	}
	if (active_ctx >= 0 && active_ctx < info.ctx.GetCount()) {
		info.ctx.SetIndex(active_ctx);
	}
	
	//info.ctx
}

void EntityInfoCtrl::OnEdit() {
	Entity& e = GetExt<Entity>();
	e.name = ~info.name;
	
	int ctx_i = info.ctx.GetIndex()-1;
	if (ctx_i >= 0 && ctx_i < info.ctx.GetCount()) {
		MetaNode& ctx = *all_ctxs[ctx_i];
		e.Data("ctx") = ctx.id;
	}
	else {
		e.Data("ctx") = String();
	}
	
	e.Data("description") = info.desc.GetData();
	
	WhenValueChange();
}

void EntityInfoCtrl::ToolMenu(Bar& bar) {
	
}

DatasetPtrs EntityInfoCtrl::GetDataset() {
	DatasetPtrs p;
	MetaNode& n = GetNode();
	p.entity = &GetExt<Entity>();
	if (p.entity) {
		p.env = MetaEnv().FindNodeEnv(*p.entity);
		if (p.env) {
			bool found_db_src = false;
			for (MetaNode& s : p.env->sub) {
				if (s.kind == METAKIND_DB_REF) {
					for (auto db : ~DatasetIndex()) {
						MetaNodeExt& ext = *db.value;
						if (ext.node.kind == METAKIND_DATABASE_SOURCE) {
							p.src = dynamic_cast<SrcTxtHeader*>(&ext);
							ASSERT(p.src);
							p.src->RealizeData();
							found_db_src = true;
							break;
						}
					}
				}
				if (found_db_src) break;
			}
		}
	}
	return p;
}

INITIALIZER_COMPONENT_CTRL(Entity, EntityInfoCtrl)

END_UPP_NAMESPACE
