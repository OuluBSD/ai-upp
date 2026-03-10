#include "ScriptIDE.h"
#include "CardGamePlugin.h"

NAMESPACE_UPP

// --- CardGameDocumentHost ---

CardGameDocumentHost::CardGameDocumentHost()
{
	game_log.SetQTF("Welcome to the Game!&Ready to play.");
	SetTimeCallback(-16, [=] { Animate(); }, (intptr_t)this);
}

CardGameDocumentHost::~CardGameDocumentHost()
{
	KillTimeCallback((intptr_t)this);
}

bool CardGameDocumentHost::Load(const String& path_)
{
	path = path_;
	Refresh();
	return true;
}

void CardGameDocumentHost::ActivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		ide->context_pane_right.Title("Game Log");
		ide->context_pane_right.Add(game_log.SizePos());
		ide->context_pane_right.Show();
	}
}

void CardGameDocumentHost::DeactivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		game_log.Remove();
		ide->context_pane_right.Hide();
	}
}

void CardGameDocumentHost::MainMenu(Bar& bar)
{
	bar.Sub("Game", [=](Bar& b) {
		b.Add("Restart Match", [=] { Todo("Restart"); });
		b.Add("Concede", [=] { Todo("Concede"); });
	});
}

void CardGameDocumentHost::Toolbar(Bar& bar)
{
	bar.Add("Play", CtrlImg::right_arrow(), [=] { Todo("Play Turn"); });
}

void CardGameDocumentHost::SetLayout(const String& path)
{
	form_path = path;
	sprites.Clear();
	zones.Clear();
	
	Value v = ParseJSON(LoadFile(path));
	if(v.IsVoid()) return;
	
	if(v.Is<ValueMap>()) {
		Value bg = v["background_color"];
		if(bg.Is<ValueMap>()) {
			background_color = Color((int)bg["r"], (int)bg["g"], (int)bg["b"]);
		}
		
		Value vzones = v["zones"];
		if(vzones.Is<ValueArray>()) {
			for(int i = 0; i < vzones.GetCount(); i++) {
				Value vz = vzones[i];
				if(vz.Is<ValueMap>()) {
					String zid = vz["id"];
					Zone& z = zones.Add(zid);
					z.id = zid;
					Value vr = vz["rect"];
					z.rect = RectC((int)vr["x"], (int)vr["y"], (int)vr["w"], (int)vr["h"]);
					z.anchor = vz["anchor"];
					z.type = vz["type"];
					
					Value vchildren = vz["children"];
					if(vchildren.Is<ValueArray>()) {
						for(int j = 0; j < vchildren.GetCount(); j++) {
							Value vc = vchildren[j];
							if(vc.Is<ValueMap>()) {
								String cid = vc["id"];
								Zone& cz = zones.Add(cid);
								cz.id = cid;
								Value cr = vc["rect"];
								cz.rect = RectC(z.rect.left + (int)cr["x"], z.rect.top + (int)cr["y"], (int)cr["w"], (int)cr["h"]);
								cz.anchor = z.anchor; 
								cz.type = vc["type"];
							}
						}
					}
				}
			}
		}
	}
	
	Refresh();
}

Rect CardGameDocumentHost::GetAbsoluteRect(const Rect& r, const String& anchor, const Size& parent_sz)
{
	int x = r.left;
	int y = r.top;
	
	if(anchor == "CENTER") {
		x = (parent_sz.cx / 2) - (r.GetWidth() / 2) + r.left;
		y = (parent_sz.cy / 2) - (r.GetHeight() / 2) + r.top;
	}
	else if(anchor == "BOTTOM_CENTER") {
		x = (parent_sz.cx / 2) - (r.GetWidth() / 2) + r.left;
		y = parent_sz.cy - r.GetHeight() - r.top;
	}
	else if(anchor == "TOP_CENTER") {
		x = (parent_sz.cx / 2) - (r.GetWidth() / 2) + r.left;
	}
	
	return RectC(x, y, r.GetWidth(), r.GetHeight());
}

void CardGameDocumentHost::ClearSprites()
{
	sprites.Clear();
	Refresh();
}

void CardGameDocumentHost::SetSprite(const String& id, const String& asset_path, int x, int y)
{
	Sprite& s = sprites.GetAdd(id);
	s.img = StreamRaster::LoadFileAny(asset_path);
	s.rect = RectC(x, y, s.img.GetWidth(), s.img.GetHeight());
	s.target_rect = s.rect;
	s.animating = false;
	Refresh();
}

void CardGameDocumentHost::MoveSprite(const String& id, int x, int y, bool animated)
{
	int q = sprites.Find(id);
	if(q >= 0) {
		Sprite& s = sprites[q];
		s.target_rect = RectC(x, y, s.rect.GetWidth(), s.rect.GetHeight());
		if(animated) {
			s.animating = true;
		} else {
			s.rect = s.target_rect;
			s.animating = false;
			Refresh();
		}
	}
}

void CardGameDocumentHost::MoveSpriteToZone(const String& id, const String& zone_id, bool animated)
{
	int qz = zones.Find(zone_id);
	int qs = sprites.Find(id);
	if(qz >= 0 && qs >= 0) {
		Zone& z = zones[qz];
		Rect abs_z = GetAbsoluteRect(z.rect, z.anchor, GetSize());
		int tx = abs_z.left + (abs_z.GetWidth() / 2) - (sprites[qs].rect.GetWidth() / 2);
		int ty = abs_z.top + (abs_z.GetHeight() / 2) - (sprites[qs].rect.GetHeight() / 2);
		MoveSprite(id, tx, ty, animated);
	}
}

void CardGameDocumentHost::Log(const String& msg)
{
	game_log.SetQTF(game_log.GetQTF() + "&" + msg);
}

void CardGameDocumentHost::Animate()
{
	bool changed = false;
	for(int i = 0; i < sprites.GetCount(); i++) {
		Sprite& s = sprites[i];
		if(s.animating) {
			Point p = s.rect.TopLeft();
			Point tp = s.target_rect.TopLeft();
			
			int dx = tp.x - p.x;
			int dy = tp.y - p.y;
			
			if(abs(dx) <= 2 && abs(dy) <= 2) {
				s.rect = s.target_rect;
				s.animating = false;
			} else {
				s.rect = RectC(p.x + dx / 5, p.y + dy / 5, s.rect.GetWidth(), s.rect.GetHeight());
			}
			changed = true;
		}
	}
	if(changed) Refresh();
}

void CardGameDocumentHost::Paint(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, background_color);
	
	for(int i = 0; i < sprites.GetCount(); i++) {
		const Sprite& s = sprites[i];
		if(!s.img.IsEmpty())
			w.DrawImage(s.rect.left, s.rect.top, s.img);
	}
}

void CardGameDocumentHost::LeftDown(Point p, dword flags)
{
	for(int i = sprites.GetCount() - 1; i >= 0; i--) {
		if(sprites[i].rect.Contains(p)) {
			String id = sprites.GetKey(i);
			if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
				PyValue on_click = ide->vm.GetGlobals().Get("on_click", PyValue());
				if(!on_click.IsNone())
					ide->vm.Call(on_click, {PyValue(id)});
			}
			break;
		}
	}
}

// --- CardGameProperties ---

void CardGameProperties::Generate(FormObject* pI, int index)
{
	if (!pI) return;

	_Properties.Clear();
	_Options.Clear();

	_Item  = pI;
	_Index = index;

	String type = pI->Get("Type");
	if (type.IsEmpty()) return;

	Property("Variable", t_("ID:"), "EditField", Array<String>() << pI->Get("Variable"));
	Property("Type", t_("Zone Type:"), "DropList", Array<String>() << pI->Get("Type") << "HAND" << "TRICK" << "CONTAINER" << "LABEL" << "SPRITE");
	Property("Anchor", t_("Anchor:"), "DropList", Array<String>() << pI->Get("Anchor") << "TOP_LEFT" << "CENTER" << "BOTTOM_CENTER" << "TOP_CENTER" << "CENTER_LEFT" << "CENTER_RIGHT");
	
	if(type == "LABEL") {
		Property("Label", t_("Text:"), "EditField", Array<String>() << pI->Get("Label"));
	}
	
	if(type == "SPRITE") {
		Property("Image", t_("Asset:"), "EditField", Array<String>() << pI->Get("Image"));
	}

	_Options.HideRow(0);
}

// --- CardGameLayoutEditor ---

CardGameLayoutEditor::CardGameLayoutEditor()
{
	Construct(true);
	embedded = true;
	
	_TypeList.Clear();
	_TypeList.Add("ZONE");
	_TypeList.Add("CARD_SLOT");
	_TypeList.Add("LABEL");
	_TypeList.Add("CONTAINER");
	_TypeList.Add("SPRITE");
	
	_View.WhenObjectProperties = [this](const Vector<int>& idx) { this->OpenCardProperties(idx); };
}

CardGameLayoutEditor::~CardGameLayoutEditor()
{
}

void CardGameLayoutEditor::OpenCardProperties(const Vector<int>& indexes)
{
	if (!_View.IsLayout())
		return;

	String temp = _TempObjectName;
	_TempObjectName.Clear();
	_ItemList.EndEdit(false, false, false);
	int row = _ItemList.GetCurrentRow();
	if (row >= 0 && !temp.IsEmpty())
	{
		_View.GetCurrentLayout()->GetObjects()[row].Set("Variable", temp);
		_ItemList.Set(row, 1, temp);
	}
	_LayoutList.EndEdit();

	if (indexes.GetCount() == 1)
	{
		FormObject* pI = _View.GetObject(indexes[0]);
		if (!pI) return;

		card_properties.Generate(pI, indexes[0]);
	}

	if (indexes.GetCount() == 0)
	{
		card_properties._Headers.Clear();
		card_properties._Options.Clear();
	}

	UpdateItemList();
}

bool CardGameLayoutEditor::Load(const String& path_)
{
	path = path_;
	_View.New();
	
	Value v = ParseJSON(LoadFile(path));
	if(v.IsVoid()) return false;
	
	if(v.Is<ValueMap>()) {
		_View.AddLayout(v["name"]);
		_View.SelectLayout(0);
		
		Value vzones = v["zones"];
		if(vzones.Is<ValueArray>()) {
			for(int i = 0; i < vzones.GetCount(); i++) {
				Value vz = vzones[i];
				Rect r = RectC((int)vz["rect"]["x"], (int)vz["rect"]["y"], (int)vz["rect"]["w"], (int)vz["rect"]["h"]);
				
				_View.CreateObject(r.TopLeft(), "ZONE"); 
				int last = _View.GetObjectCount() - 1;
				FormObject& obj = (*_View.GetObjects())[last];
				obj.SetRect(r);
				obj.Set("Variable", vz["id"]);
				obj.Set("Type", vz["type"]);
				obj.Set("Anchor", vz["anchor"]);
				
				Value vchildren = vz["children"];
				if(vchildren.Is<ValueArray>()) {
					for(int j = 0; j < vchildren.GetCount(); j++) {
						Value vc = vchildren[j];
						Rect cr = RectC(r.left + (int)vc["rect"]["x"], r.top + (int)vc["rect"]["y"], (int)vc["rect"]["w"], (int)vc["rect"]["h"]);
						_View.CreateObject(cr.TopLeft(), "ZONE");
						int clast = _View.GetObjectCount() - 1;
						FormObject& cobj = (*_View.GetObjects())[clast];
						cobj.SetRect(cr);
						cobj.Set("Variable", vc["id"]);
						cobj.Set("Type", vc["type"]);
						cobj.Set("Parent", vz["id"]);
					}
				}
			}
		}
	}
	
	UpdateTools();
	ProjectSaved(true);
	return true; 
}

bool CardGameLayoutEditor::Save()
{
	ValueMap m;
	if(_View.IsLayout()) {
		m.Add("name", _View.GetCurrentLayout()->Get("Form.Name"));
		
		ValueMap bg;
		bg.Add("r", 40); bg.Add("g", 160); bg.Add("b", 40);
		m.Add("background_color", bg);
		
		ValueArray vzones;
		Array<FormObject>* objs = _View.GetObjects();
		if(objs) {
			for(int i = 0; i < objs->GetCount(); i++) {
				FormObject& obj = (*objs)[i];
				if(!obj.Get("Parent").IsEmpty()) continue;
				
				ValueMap vz;
				vz.Add("id", obj.Get("Variable"));
				vz.Add("type", obj.Get("Type"));
				vz.Add("anchor", obj.Get("Anchor", "TOP_LEFT"));
				
				Rect r = obj.GetRect();
				ValueMap rect;
				rect.Add("x", r.left); rect.Add("y", r.top); rect.Add("w", r.GetWidth()); rect.Add("h", r.GetHeight());
				vz.Add("rect", rect);
				
				ValueArray children;
				for(int j = 0; j < objs->GetCount(); j++) {
					FormObject& cobj = (*objs)[j];
					if(cobj.Get("Parent") == obj.Get("Variable")) {
						ValueMap cvz;
						cvz.Add("id", cobj.Get("Variable"));
						cvz.Add("type", cobj.Get("Type", "ZONE"));
						Rect cr = cobj.GetRect();
						ValueMap crect;
						crect.Add("x", cr.left - r.left); crect.Add("y", cr.top - r.top); crect.Add("w", cr.GetWidth()); crect.Add("h", cr.GetHeight());
						cvz.Add("rect", crect);
						children.Add(cvz);
					}
				}
				if(children.GetCount() > 0)
					vz.Add("children", children);
					
				vzones.Add(vz);
			}
		}
		m.Add("zones", vzones);
	}
	
	String json = AsJSON(Value(m), true);
	if(::Upp::SaveFile(path, json)) {
		ProjectSaved(true);
		return true;
	}
	return false;
}

bool CardGameLayoutEditor::SaveAs(const String& path_)
{
	path = path_;
	return Save();
}

void CardGameLayoutEditor::ActivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		ide->context_pane_left.Title("Properties");
		ide->context_pane_left.Add(card_properties.SizePos());
		ide->context_pane_left.Show();
		
		ide->context_pane_right.Title("Items");
		ide->context_pane_right.Add(_ItemList.SizePos());
		ide->context_pane_right.Show();
	}
}

void CardGameLayoutEditor::DeactivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		card_properties.Remove();
		_ItemList.Remove();
		ide->context_pane_left.Hide();
		ide->context_pane_right.Hide();
	}
}

void CardGameLayoutEditor::MainMenu(Bar& bar)
{
	CreateMenuBar(bar);
}

void CardGameLayoutEditor::Toolbar(Bar& bar)
{
	CreateToolBar(bar);
}

// --- CardGamePlugin ---

CardGamePlugin::CardGamePlugin()
{
	gamestate_handler.plugin = this;
	form_handler.plugin = this;
}

CardGamePlugin::~CardGamePlugin()
{
}

void CardGamePlugin::Init(IPluginContext& ctx)
{
	context = &ctx;
	context->GetIDE().plugin_manager->RegisterFileTypeHandler(gamestate_handler);
	context->GetIDE().plugin_manager->RegisterFileTypeHandler(form_handler);
	context->GetIDE().plugin_manager->RegisterCustomExecuteProvider(*this);
	context->GetIDE().plugin_manager->RegisterPythonBindingProvider(*this);
	
	context->GetIDE().Log("CardGamePlugin initialized.");
}

void CardGamePlugin::Shutdown()
{
}

IDocumentHost* CardGamePlugin::GameStateHandler::CreateDocumentHost()
{
	return new CardGameDocumentHost();
}

IDocumentHost* CardGamePlugin::FormHandler::CreateDocumentHost()
{
	return new CardGameLayoutEditor();
}

bool CardGamePlugin::CanExecute(const String& path)
{
	return ToLower(GetFileExt(path)) == ".gamestate";
}

void CardGamePlugin::Execute(const String& path)
{
	if(!context) return;
	
	PythonIDE& ide = context->GetIDE();
	ide.Log("Launching Game State: " + path);
	
	Value g = ParseJSON(LoadFile(path));
	if(g.IsVoid()) {
		ide.Error("Failed to parse .gamestate");
		return;
	}
	
	String script = g["entry_script"];
	String layout = g["layout"];
	
	ide.LoadFile(path);
	CardGameDocumentHost* view = nullptr;
	if(ide.active_file >= 0) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~ide.open_files[ide.active_file].editor))
			view = dynamic_cast<CardGameDocumentHost*>(h);
	}
	
	if(!view) {
		ide.Error("Active tab is not a CardGameDocumentHost");
		return;
	}
	
	if(!layout.IsEmpty())
		view->SetLayout(AppendFileName(GetFileDirectory(path), layout));
	
	String script_path = AppendFileName(GetFileDirectory(path), script);
	String code = LoadFile(script_path);
	
	ide.plugin_manager->SyncBindings(ide.vm);
	ide.run_manager.Run(code, script_path);
}

static PyValue SetCard(const Vector<PyValue>& args, void* user_data)
{
	CardGameDocumentHost* view = (CardGameDocumentHost*)user_data;
	if(args.GetCount() >= 4 && view) {
		view->SetSprite(args[0].ToString(), args[1].ToString(), args[2].AsInt(), args[3].AsInt());
	}
	return PyValue();
}

static PyValue MoveCard(const Vector<PyValue>& args, void* user_data)
{
	CardGameDocumentHost* view = (CardGameDocumentHost*)user_data;
	if(args.GetCount() >= 2 && view) {
		bool anim = args.GetCount() >= 4 ? args[3].AsInt() != 0 : false;
		if(args[1].GetType() == PY_STR) {
			view->MoveSpriteToZone(args[0].ToString(), args[1].ToString(), anim);
		} else if(args.GetCount() >= 3) {
			view->MoveSprite(args[0].ToString(), args[1].AsInt(), args[2].AsInt(), anim);
		}
	}
	return PyValue();
}

static PyValue ClearSprites(const Vector<PyValue>& args, void* user_data)
{
	CardGameDocumentHost* view = (CardGameDocumentHost*)user_data;
	if(view) {
		view->ClearSprites();
	}
	return PyValue();
}

static PyValue Log(const Vector<PyValue>& args, void* user_data)
{
	CardGameDocumentHost* view = (CardGameDocumentHost*)user_data;
	if(args.GetCount() >= 1 && view) {
		view->Log(args[0].ToString());
	}
	return PyValue();
}

void CardGamePlugin::SyncBindings(PyVM& vm)
{
	PythonIDE& ide = context->GetIDE();
	CardGameDocumentHost* view = nullptr;
	if(ide.active_file >= 0 && ide.open_files[ide.active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~ide.open_files[ide.active_file].editor))
			view = dynamic_cast<CardGameDocumentHost*>(h);
	}
	
	PyValue hearts_view = PyValue::Dict();
	hearts_view.SetItem("set_card", PyValue::Function("set_card", &SetCard, view));
	hearts_view.SetItem("move_card", PyValue::Function("move_card", &MoveCard, view));
	hearts_view.SetItem("clear_sprites", PyValue::Function("clear_sprites", &ClearSprites, view));
	hearts_view.SetItem("log", PyValue::Function("log", &Log, view));
	
	vm.GetGlobals().GetAdd("hearts_view") = hearts_view;
}

REGISTER_PLUGIN(CardGamePlugin)

END_UPP_NAMESPACE
