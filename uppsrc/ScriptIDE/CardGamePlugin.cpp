#include "ScriptIDE.h"
#include "CardGamePlugin.h"

// --- CardGameDocumentHost ---

CardGameDocumentHost::CardGameDocumentHost()
{
	game_log.SetQTF("Welcome to the Game!&Ready to play.");
	Upp::SetTimeCallback(-16, [=] { Animate(); }, this);
}

CardGameDocumentHost::~CardGameDocumentHost()
{
	Upp::KillTimeCallback(this);
	Upp::KillTimeCallback(&resize_refresh_pending);
}

bool CardGameDocumentHost::Load(const String& path_)
{
	path = path_;

	// Load the form layout if specified in the gamestate
	String json = LoadFile(path);
	Value gs = ParseJSON(json);
	if(!gs.IsVoid()) {
		String layout = gs["layout"];
		if(!layout.IsEmpty())
			SetLayout(AppendFileName(GetFileDirectory(path), layout));
	}

	Refresh();

	// Wire this view to the plugin and run the Python game
	if(plugin) {
		plugin->SetView(this);
		try {
			plugin->Execute(path);
		} catch(Exc& e) {
			LOG("CardGameDocumentHost: Execute error: " << e);
		}
	}

	return true;
}

void CardGameDocumentHost::Layout()
{
	Ctrl::Layout();

	Size sz = GetSize();
	if(sz == last_layout_size)
		return;

	last_layout_size = sz;
	if(sz.cx <= 0 || sz.cy <= 0 || refresh_running || resize_refresh_pending)
		return;

	resize_refresh_pending = true;
	Upp::PostCallback([=] {
		resize_refresh_pending = false;
		RefreshGameView();
	}, &resize_refresh_pending);
}

void CardGameDocumentHost::ActivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		if(!ide->context_pane_right.IsDocked())
			ide->DockRight(ide->context_pane_right);
		
		ide->context_pane_right.Title("Game Log");
		ide->context_pane_right.Add(game_log.SizePos());
		ide->context_pane_right.Show();
	}
}

void CardGameDocumentHost::DeactivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		game_log.Remove();
		ide->context_pane_right.Close();
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
								cz.rect = RectC(z.rect.left + (int)vc["rect"]["x"], z.rect.top + (int)vc["rect"]["y"], (int)vc["rect"]["w"], (int)vc["rect"]["h"]);
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

// IHeartsView implementation

void CardGameDocumentHost::ClearSprites()
{
	sprites.Clear();
	Refresh();
}

void CardGameDocumentHost::SetCard(const String& card_id, const String& asset_path, int x, int y)
{
	Sprite& s = sprites.GetAdd(card_id);

	String full_path = asset_path;
	if(!IsFullPath(asset_path))
		full_path = AppendFileName(GetFileDirectory(path), asset_path);

	if(s.img.IsEmpty() || s.asset_path != full_path) {
		s.img = StreamRaster::LoadFileAny(full_path);
		s.asset_path = full_path;
	}
	s.rect = RectC(x, y, s.img.GetWidth(), s.img.GetHeight());
	s.target_rect = s.rect;
	s.animating = false;
	Refresh();
}

void CardGameDocumentHost::MoveCardToZone(const String& card_id, const String& zone_id, int offset, bool animated)
{
	int qz = zones.Find(zone_id);
	int qs = sprites.Find(card_id);
	if(qz < 0 || qs < 0) return;

	Zone& z = zones[qz];
	Rect abs_z = GetAbsoluteRect(z.rect, z.anchor, GetSize());
	int tx = abs_z.left + (abs_z.GetWidth() / 2) - (sprites[qs].rect.GetWidth() / 2) + offset;
	int ty = abs_z.top  + (abs_z.GetHeight() / 2) - (sprites[qs].rect.GetHeight() / 2);

	Sprite& s = sprites[qs];
	s.target_rect = RectC(tx, ty, s.rect.GetWidth(), s.rect.GetHeight());
	if(animated)
		s.animating = true;
	else {
		s.rect = s.target_rect;
		s.animating = false;
	}
	Refresh();
}

Value CardGameDocumentHost::GetZoneRect(const String& zone_id)
{
	int q = zones.Find(zone_id);
	ValueMap m;
	if(q >= 0) {
		Rect r = GetAbsoluteRect(zones[q].rect, zones[q].anchor, GetSize());
		m.Add("x", r.left);
		m.Add("y", r.top);
		m.Add("w", r.GetWidth());
		m.Add("h", r.GetHeight());
	} else {
		m.Add("x", 0); m.Add("y", 0); m.Add("w", 552); m.Add("h", 96);
	}
	return m;
}

void CardGameDocumentHost::Log(const String& msg)
{
	game_log.SetQTF(game_log.GetQTF() + "&" + DeQtfLf(msg));
	Refresh();
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
			
			if(Upp::abs(dx) <= 2 && Upp::abs(dy) <= 2) {
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

void CardGameDocumentHost::RefreshGameView()
{
	if(refresh_running)
		return;

	PyVM* vm = plugin && plugin->GetContext() ? plugin->GetContext()->GetVM() : nullptr;
	if(!vm) {
		Refresh();
		return;
	}

	PyValue refresh_ui = vm->GetGlobals().GetDict().Get("refresh_ui", PyValue());
	if(!refresh_ui.IsFunction()) {
		Refresh();
		return;
	}

	refresh_running = true;
	try {
		vm->Call(refresh_ui, {});
	}
	catch(Exc& e) {
		LOG("refresh_ui error: " << e);
		Refresh();
	}
	refresh_running = false;
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
			String card_id = sprites.GetKey(i);
			if(plugin) {
				PyVM* vm = plugin->GetContext() ? plugin->GetContext()->GetVM() : nullptr;
				if(vm) {
					PyValue on_click = vm->GetGlobals().GetDict().Get("on_click", PyValue());
					if(on_click.IsFunction()) {
						try { vm->Call(on_click, {PyValue(card_id)}); }
						catch(Exc& e) { LOG("on_click error: " << e); }
						Refresh();
					}
				}
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
		if(!ide->context_pane_left.IsDocked())
			ide->DockLeft(ide->context_pane_left);
		
		if(!ide->context_pane_right.IsDocked())
			ide->DockRight(ide->context_pane_right);
		
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
		ide->context_pane_left.Close();
		ide->context_pane_right.Close();
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

// --- CardGamePluginGUI ---

CardGamePluginGUI::CardGamePluginGUI()
{
	gamestate_handler.plugin = this;
	form_handler.plugin = this;
}

CardGamePluginGUI::~CardGamePluginGUI()
{
}

void CardGamePluginGUI::Init(IPluginContext& context_)
{
	CardGamePlugin::Init(context_);
	
	if(IPluginContextGUI* gui = dynamic_cast<IPluginContextGUI*>(&context_)) {
		if(IPluginRegistryGUI* reg = dynamic_cast<IPluginRegistryGUI*>(&context_)) {
			reg->RegisterFileTypeHandler(gamestate_handler);
			reg->RegisterFileTypeHandler(form_handler);
		}
		gui->GetIDE().Log("CardGamePlugin (GUI) initialized.");
	}
}

void CardGamePluginGUI::Shutdown()
{
	CardGamePlugin::Shutdown();
}

IDocumentHost* CardGamePluginGUI::GameStateHandler::CreateDocumentHost()
{
	CardGameDocumentHost* host = new CardGameDocumentHost();
	host->SetPlugin(plugin);
	return host;
}

IDocumentHost* CardGamePluginGUI::FormHandler::CreateDocumentHost()
{
	return new CardGameLayoutEditor();
}

// Registration
REGISTER_PLUGIN(CardGamePluginGUI)
