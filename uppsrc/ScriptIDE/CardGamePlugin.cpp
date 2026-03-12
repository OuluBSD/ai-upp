#include "ScriptIDE.h"
#include "CardGamePlugin.h"

namespace {

struct CardGameZoneDef : Moveable<CardGameZoneDef> {
	String id;
	Rect   rect;
	String anchor;
	String zone_type;
	String parent;
	String label;
};

bool IsLikelyJsonForm(const String& data)
{
	for(int i = 0; i < data.GetCount(); i++) {
		int c = data[i];
		if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
			continue;
		return c == '{' || c == '[';
	}
	return false;
}

String NormalizeZoneType(const String& zone_type, const String& control_type = String())
{
	String type = ToUpper(zone_type);
	if(type.IsEmpty()) {
		if(control_type == "Button")
			type = "BUTTON";
		else if(control_type == "Label")
			type = "LABEL";
	}
	return type;
}

String ZoneTypeToControlType(const String& zone_type)
{
	return NormalizeZoneType(zone_type) == "BUTTON" ? "Button" : "Label";
}

String EncodeFontColor(const Color& color)
{
	Color c = color;
	return Encode64(StoreAsString(c));
}

Color ParseBackgroundColor(const String& value, const Color& fallback)
{
	Vector<String> parts = Split(value, ',');
	if(parts.GetCount() != 3)
		return fallback;
	return Color(ScanInt(parts[0]), ScanInt(parts[1]), ScanInt(parts[2]));
}

void ConfigureZoneObject(FormObject& obj, const CardGameZoneDef& zone)
{
	String zone_type = NormalizeZoneType(zone.zone_type);
	String control_type = ZoneTypeToControlType(zone_type);

	obj.SetRect(zone.rect);
	obj.SetHAlign(Ctrl::LEFT);
	obj.SetVAlign(Ctrl::TOP);
	obj.Set("Variable", zone.id);
	obj.Set("Type", control_type);
	obj.Set("ZoneType", zone_type);
	obj.Set("Anchor", zone.anchor.IsEmpty() ? "TOP_LEFT" : zone.anchor);
	obj.Set("Parent", zone.parent);

	if(control_type == "Button") {
		obj.Set("Label", zone.label);
	}
	else {
		obj.Set("Label", zone.label);
		obj.Set("Text.Align", zone.anchor == "CENTER_LEFT" ? "Left" :
		                        zone.anchor == "CENTER_RIGHT" ? "Right" : "Center");
		obj.Set("Font.Color", EncodeFontColor(White()));
		if(zone.id == "status_line")
			obj.SetNumber("Font.Height", 16);
		else if(zone_type == "LABEL")
			obj.SetNumber("Font.Height", 18);
	}
}

void AddZoneObject(FormView& view, const CardGameZoneDef& zone)
{
	if(!view.IsLayout())
		return;
	view.GetObjects()->Add();
	ConfigureZoneObject(view.GetObjects()->Top(), zone);
}

Size GuessCardGameFormSize(const Vector<CardGameZoneDef>& zones)
{
	int max_right = 800;
	int max_bottom = 600;
	for(const CardGameZoneDef& zone : zones) {
		max_right = max(max_right, zone.rect.right + 32);
		max_bottom = max(max_bottom, zone.rect.bottom + 32);
	}
	return Size(max_right, max_bottom);
}

bool LoadCardGameJsonForm(FormView& view, const String& data, Color& background_color)
{
	Value root = ParseJSON(data);
	if(root.IsVoid() || !root.Is<ValueMap>())
		return false;

	Value bg = root["background_color"];
	if(bg.Is<ValueMap>())
		background_color = Color((int)bg["r"], (int)bg["g"], (int)bg["b"]);

	Vector<CardGameZoneDef> defs;
	Value zones = root["zones"];
	if(zones.Is<ValueArray>()) {
		for(int i = 0; i < zones.GetCount(); i++) {
			Value zone = zones[i];
			if(!zone.Is<ValueMap>())
				continue;

			CardGameZoneDef def;
			def.id = zone["id"];
			def.zone_type = zone["type"];
			def.anchor = zone["anchor"];
			Value rect = zone["rect"];
			def.rect = RectC((int)rect["x"], (int)rect["y"], (int)rect["w"], (int)rect["h"]);
			def.label = NormalizeZoneType(def.zone_type) == "BUTTON" ? def.id : String();
			defs.Add(def);

			Value children = zone["children"];
			if(children.Is<ValueArray>()) {
				for(int j = 0; j < children.GetCount(); j++) {
					Value child = children[j];
					if(!child.Is<ValueMap>())
						continue;

					CardGameZoneDef cdef;
					cdef.id = child["id"];
					cdef.zone_type = child["type"];
					cdef.anchor = def.anchor;
					cdef.parent = def.id;
					Value child_rect = child["rect"];
					cdef.rect = RectC(def.rect.left + (int)child_rect["x"],
					                  def.rect.top + (int)child_rect["y"],
					                  (int)child_rect["w"], (int)child_rect["h"]);
					defs.Add(cdef);
				}
			}
		}
	}

	view.New();
	if(view.GetLayoutCount() <= 0)
		return false;

	FormLayout* layout = view.GetCurrentLayout();
	layout->Set("Form.Name", root["name"]);
	layout->SetNumber("Form.Width", GuessCardGameFormSize(defs).cx);
	layout->SetNumber("Form.Height", GuessCardGameFormSize(defs).cy);
	layout->Set("CardGame.Background", AsString(background_color.GetR()) + "," +
	                                  AsString(background_color.GetG()) + "," +
	                                  AsString(background_color.GetB()));
	for(const CardGameZoneDef& def : defs)
		AddZoneObject(view, def);

	return true;
}

bool LoadCardGameFormView(FormView& view, const String& path, Color& background_color)
{
	String data = LoadFile(path);
	if(data.IsVoid())
		return false;

	background_color = Color(40, 160, 40);
	if(IsLikelyJsonForm(data))
		return LoadCardGameJsonForm(view, data, background_color);

	if(!view.LoadAll(path, false))
		return false;

	if(!view.IsLayout() && view.GetLayoutCount() > 0)
		view.SelectLayout(0);
	if(view.IsLayout()) {
		String bg = view.GetCurrentLayout()->Get("CardGame.Background");
		if(!bg.IsEmpty())
			background_color = ParseBackgroundColor(bg, background_color);
	}
	return true;
}

Vector<CardGameZoneDef> ExtractCardGameZones(const FormView& view)
{
	Vector<CardGameZoneDef> defs;
	const FormLayout* layout = view.GetCurrentLayout();
	if(!layout)
		return defs;

	const Array<FormObject>& objects = layout->GetObjects();
	for(int i = 0; i < objects.GetCount(); i++) {
		const FormObject& obj = objects[i];
		CardGameZoneDef& def = defs.Add();
		def.id = obj.Get("Variable");
		def.rect = obj.GetRect();
		def.anchor = obj.Get("Anchor");
		if(def.anchor.IsEmpty())
			def.anchor = "TOP_LEFT";
		def.zone_type = NormalizeZoneType(obj.Get("ZoneType"), obj.Get("Type"));
		def.parent = obj.Get("Parent");
		def.label = obj.Get("Label");
	}
	return defs;
}

Rect GetZoneRectFromForm(const Form& form, const String& zone_id)
{
	if(Ctrl* ctrl = const_cast<Form&>(form).GetCtrl(zone_id))
		return ctrl->GetRect();
	return Rect();
}

Rect ResolveAnchoredRect(const Rect& r, const String& anchor, const Size& parent_sz)
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
	else if(anchor == "CENTER_LEFT") {
		y = (parent_sz.cy / 2) - (r.GetHeight() / 2) + r.top;
	}
	else if(anchor == "CENTER_RIGHT") {
		x = parent_sz.cx - r.GetWidth() - r.left;
		y = (parent_sz.cy / 2) - (r.GetHeight() / 2) + r.top;
	}
	else if(anchor == "BOTTOM_LEFT") {
		y = parent_sz.cy - r.GetHeight() - r.top;
	}

	return RectC(x, y, r.GetWidth(), r.GetHeight());
}

}

// --- CardGameDocumentHost ---

CardGameOverlay::CardGameOverlay()
{
	Transparent();
}

void CardGameOverlay::Paint(Draw& w)
{
	if(owner)
		owner->PaintOverlay(w);
}

void CardGameOverlay::LeftDown(Point p, dword flags)
{
	if(owner)
		owner->OverlayLeftDown(p, flags);
}

CardGameDocumentHost::CardGameDocumentHost()
{
	game_log.SetQTF("Welcome to the Game!&Ready to play.");
	Add(table_form.SizePos());
	overlay.owner = this;
	overlay.NoWantFocus();
	Add(overlay.SizePos());
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
	SyncFormExplorer();

	// Wire this view to the plugin and run the Python game
	if(plugin) {
		plugin->SetView(this);
		try {
			plugin->Execute(path);
			// The first script-driven layout can happen before the host has a
			// stable on-screen size. Re-run once on the next UI tick so zone
			// based positioning lands in the visible area.
			Upp::PostCallback([=] { RefreshGameView(); }, &last_layout_size);
		} catch(Exc& e) {
			LOG("CardGameDocumentHost: Execute error: " << e);
		}
	}

	return true;
}

void CardGameDocumentHost::Layout()
{
	Ctrl::Layout();
	ApplyFormLayout();

	Size sz = GetSize();
	if(sz == last_layout_size)
		return;

	last_layout_size = sz;
	if(sz.cx <= 0 || sz.cy <= 0 || refresh_running || resize_refresh_pending)
		return;

	SyncFormExplorer();

	resize_refresh_pending = true;
	Upp::PostCallback([=] {
		resize_refresh_pending = false;
		ApplyFormLayout();
		RefreshGameView();
	}, &resize_refresh_pending);
}

void CardGameDocumentHost::ActivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		SyncFormExplorer();
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
		if(ide->form_explorer)
			ide->form_explorer->Clear();
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
	labels.Clear();
	buttons.Clear();
	highlights.Clear();

	FormView view;
	if(!LoadCardGameFormView(view, path, background_color))
		return;

	String xml;
	view.SaveAllString(xml, false);
	if(!table_form.LoadString(xml, false))
		return;

	String layout_name = "Default";
	if(view.IsLayout())
		layout_name = view.GetCurrentLayout()->Get("Form.Name", "Default");
	if(!table_form.Layout(layout_name))
		return;

	Vector<CardGameZoneDef> defs = ExtractCardGameZones(view);
	for(const CardGameZoneDef& def : defs) {
		Zone& z = zones.GetAdd(def.id);
		z.id = def.id;
		z.rect = def.rect;
		z.anchor = def.anchor;
		z.type = def.zone_type;
	}

	ApplyFormLayout();
	SyncFormControls();
	Refresh();
	SyncFormExplorer();
}

Rect CardGameDocumentHost::GetAbsoluteRect(const Rect& r, const String& anchor, const Size& parent_sz)
{
	return ResolveAnchoredRect(r, anchor, parent_sz);
}

// IHeartsView implementation

void CardGameDocumentHost::ClearSprites()
{
	sprites.Clear();
	SyncFormExplorer();
	overlay.Refresh();
}

void CardGameDocumentHost::SetLabel(const String& zone_id, const String& text)
{
	labels.GetAdd(zone_id) = text;
	SyncFormControls();
	SyncFormExplorer();
	table_form.Refresh();
}

void CardGameDocumentHost::SetButton(const String& zone_id, const String& text, bool enabled)
{
	ActionButton& b = buttons.GetAdd(zone_id);
	b.text = text;
	b.enabled = enabled;
	SyncFormControls();
	SyncFormExplorer();
	table_form.Refresh();
}

void CardGameDocumentHost::SetHighlight(const String& zone_id, bool enabled)
{
	int q = highlights.Find(zone_id);
	if(enabled) {
		if(q < 0)
			highlights.Add(zone_id);
	}
	else if(q >= 0) {
		highlights.Remove(q);
	}
	SyncFormExplorer();
	overlay.Refresh();
}

void CardGameDocumentHost::SetStatus(const String& text)
{
	status_text = text;
	SyncFormControls();
	SyncFormExplorer();
	table_form.Refresh();
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
	SyncFormExplorer();
	overlay.Refresh();
}

void CardGameDocumentHost::MoveCardToZone(const String& card_id, const String& zone_id, int offset, bool animated)
{
	int qs = sprites.Find(card_id);
	if(qs < 0)
		return;

	Rect abs_z = GetZoneRectFromForm(table_form, zone_id);
	if(abs_z.IsEmpty())
		return;
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
	SyncFormExplorer();
	overlay.Refresh();
}

Value CardGameDocumentHost::GetZoneRect(const String& zone_id)
{
	ValueMap m;
	Rect r = GetZoneRectFromForm(table_form, zone_id);
	if(!r.IsEmpty()) {
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
	if(changed) overlay.Refresh();
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
	ApplyFormLayout();
	SyncFormControls();
}

void CardGameDocumentHost::SyncFormExplorer()
{
	PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow());
	if(!ide || !ide->form_explorer)
		return;

	Vector<FormExplorerEntry> entries;
	Size sz = GetSize();

	for(int i = 0; i < zones.GetCount(); i++) {
		const Zone& z = zones[i];
		FormExplorerEntry& e = entries.Add();
		e.path = "zones/" + z.id;
		e.type = "Zone";
		e.rect = GetZoneRectFromForm(table_form, z.id);
		e.details = z.type + ", " + z.anchor;
	}

	for(int i = 0; i < sprites.GetCount(); i++) {
		const Sprite& s = sprites[i];
		FormExplorerEntry& e = entries.Add();
		e.path = "sprites/" + sprites.GetKey(i);
		e.type = s.animating ? "Sprite (animating)" : "Sprite";
		e.rect = s.rect;
		e.details = GetFileName(s.asset_path);
	}

	for(int i = 0; i < labels.GetCount(); i++) {
		String zone_id = labels.GetKey(i);
		FormExplorerEntry& e = entries.Add();
		e.path = "labels/" + zone_id;
		e.type = "Label";
		e.rect = GetZoneRectFromForm(table_form, zone_id);
		e.details = labels[i];
	}

	for(int i = 0; i < buttons.GetCount(); i++) {
		String zone_id = buttons.GetKey(i);
		FormExplorerEntry& e = entries.Add();
		e.path = "buttons/" + zone_id;
		e.type = buttons[i].enabled ? "Button" : "Button (disabled)";
		e.rect = GetZoneRectFromForm(table_form, zone_id);
		e.details = buttons[i].text;
	}

	for(int i = 0; i < highlights.GetCount(); i++) {
		String zone_id = highlights[i];
		FormExplorerEntry& e = entries.Add();
		e.path = "highlights/" + zone_id;
		e.type = "Highlight";
		e.rect = GetZoneRectFromForm(table_form, zone_id);
		e.details = "active";
	}

	ide->form_explorer->SetScene(sz, entries);
}

void CardGameDocumentHost::Paint(Draw& w)
{
	w.DrawRect(GetSize(), background_color);
}

void CardGameDocumentHost::ApplyFormLayout()
{
	for(int i = 0; i < zones.GetCount(); i++) {
		const Zone& zone = zones[i];
		Ctrl* ctrl = table_form.GetCtrl(zone.id);
		if(!ctrl)
			continue;
		ctrl->SetRect(GetAbsoluteRect(zone.rect, zone.anchor, table_form.GetSize()));
	}
}

void CardGameDocumentHost::SyncFormControls()
{
	for(int i = 0; i < labels.GetCount(); i++) {
		if(Ctrl* ctrl = table_form.GetCtrl(labels.GetKey(i))) {
			if(Label* label = dynamic_cast<Label*>(ctrl)) {
				label->SetLabel(labels[i]);
				label->SetInk(White());
				int q = zones.Find(labels.GetKey(i));
				if(q >= 0) {
					label->SetAlign(zones[q].anchor == "CENTER_LEFT" ? ALIGN_LEFT :
					               zones[q].anchor == "CENTER_RIGHT" ? ALIGN_RIGHT :
					               ALIGN_CENTER);
				}
			}
		}
	}

	for(int i = 0; i < buttons.GetCount(); i++) {
		if(Ctrl* ctrl = table_form.GetCtrl(buttons.GetKey(i))) {
			if(Button* button = dynamic_cast<Button*>(ctrl)) {
				button->SetLabel(buttons[i].text);
				button->Enable(buttons[i].enabled);
			}
		}
	}

	if(Ctrl* ctrl = table_form.GetCtrl("status_line")) {
		if(Label* label = dynamic_cast<Label*>(ctrl)) {
			label->SetLabel(status_text);
			label->SetInk(White());
			label->SetAlign(ALIGN_LEFT);
		}
	}
}

void CardGameDocumentHost::PaintOverlay(Draw& w)
{
	for(int i = 0; i < highlights.GetCount(); i++) {
		Rect r = GetZoneRectFromForm(table_form, highlights[i]);
		if(r.IsEmpty())
			continue;
		Color glow = Color(255, 215, 70);
		w.DrawRect(r.left - 4, r.top - 4, r.GetWidth() + 8, 3, glow);
		w.DrawRect(r.left - 4, r.bottom + 1, r.GetWidth() + 8, 3, glow);
		w.DrawRect(r.left - 4, r.top - 4, 3, r.GetHeight() + 8, glow);
		w.DrawRect(r.right + 1, r.top - 4, 3, r.GetHeight() + 8, glow);
	}

	for(int i = 0; i < sprites.GetCount(); i++) {
		const Sprite& s = sprites[i];
		if(!s.img.IsEmpty())
			w.DrawImage(s.rect.left, s.rect.top, s.img);
	}
}

void CardGameDocumentHost::OverlayLeftDown(Point p, dword flags)
{
	for(int i = 0; i < buttons.GetCount(); i++) {
		if(!buttons[i].enabled)
			continue;
		Rect r = GetZoneRectFromForm(table_form, buttons.GetKey(i));
		if(!r.Contains(p))
			continue;
		if(plugin) {
			PyVM* vm = plugin->GetContext() ? plugin->GetContext()->GetVM() : nullptr;
			if(vm) {
				PyValue on_button = vm->GetGlobals().GetDict().Get("on_button", PyValue());
				if(on_button.IsFunction()) {
					try { vm->Call(on_button, {PyValue(buttons.GetKey(i))}); }
					catch(Exc& e) { LOG("on_button error: " << e); }
					SyncFormControls();
					overlay.Refresh();
				}
			}
		}
		return;
	}

	for(int i = sprites.GetCount() - 1; i >= 0; i--) {
		if(!sprites[i].rect.Contains(p))
			continue;
		String card_id = sprites.GetKey(i);
		if(plugin) {
			PyVM* vm = plugin->GetContext() ? plugin->GetContext()->GetVM() : nullptr;
			if(vm) {
				PyValue on_click = vm->GetGlobals().GetDict().Get("on_click", PyValue());
				if(on_click.IsFunction()) {
					try { vm->Call(on_click, {PyValue(card_id)}); }
					catch(Exc& e) { LOG("on_click error: " << e); }
					SyncFormControls();
					overlay.Refresh();
				}
			}
		}
		break;
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
	String zone_type = NormalizeZoneType(pI->Get("ZoneType"), type);
	if (type.IsEmpty()) return;

	Property("Variable", t_("ID:"), "EditField", Array<String>() << pI->Get("Variable"));
	Property("ZoneType", t_("Zone Type:"), "DropList", Array<String>() << zone_type << "HAND" << "TRICK" << "CONTAINER" << "LABEL" << "BUTTON" << "SPRITE");
	Property("Anchor", t_("Anchor:"), "DropList", Array<String>() << pI->Get("Anchor", "TOP_LEFT") << "TOP_LEFT" << "CENTER" << "BOTTOM_CENTER" << "TOP_CENTER" << "CENTER_LEFT" << "CENTER_RIGHT" << "BOTTOM_LEFT");
	
	if(zone_type == "LABEL" || zone_type == "BUTTON") {
		Property("Label", t_("Text:"), "EditField", Array<String>() << pI->Get("Label"));
	}
	
	if(zone_type == "SPRITE") {
		Property("Image", t_("Asset:"), "EditField", Array<String>() << pI->Get("Image"));
	}

	_Options.HideRow(0);
}

// --- CardGameLayoutEditor ---

CardGameLayoutEditor::CardGameLayoutEditor()
{
	embedded = true;

	Add(hsplit.SizePos());
	hsplit.Horz(vsplit, main);
	hsplit.SetPos(2000);

	vsplit.Vert() << _LayoutList << _ItemList << card_properties;

	main.Add(_CtrlContainer.SizePos());
	main.Add(_Container.SizePos());

	Construct(false);
	
	_TypeList.Clear();
	_TypeList.Add("Label");
	_TypeList.Add("Button");
	
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
	Clear();

	Color bg;
	if(!LoadCardGameFormView(_View, path, bg))
		return false;
	if(!_View.IsLayout() && _View.GetLayoutCount() > 0)
		_View.SelectLayout(0);
	UpdateLayoutList();
	UpdateChildZ();
	_Container.Set(_View, _View.GetPageRect().GetSize());
	SetViewMode(VIEW_MODE_INFO);
	
	UpdateTools();
	ProjectSaved(true);
	return true; 
}

bool CardGameLayoutEditor::Save()
{
	if(_View.IsLayout()) {
		FormLayout* layout = _View.GetCurrentLayout();
		if(layout && layout->Get("CardGame.Background").IsEmpty())
			layout->Set("CardGame.Background", "40,160,40");

		Array<FormObject>* objs = _View.GetObjects();
		if(objs) {
			for(int i = 0; i < objs->GetCount(); i++) {
				FormObject& obj = (*objs)[i];
				String zone_type = NormalizeZoneType(obj.Get("ZoneType"), obj.Get("Type"));
				obj.Set("ZoneType", zone_type);
				obj.Set("Type", ZoneTypeToControlType(zone_type));
				if(zone_type == "BUTTON" && obj.Get("Label").IsEmpty())
					obj.Set("Label", obj.Get("Variable"));
			}
		}
	}

	bool ok = _View.SaveAll(path, false);

	if(ok) {
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
}

void CardGameLayoutEditor::DeactivateUI()
{
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
