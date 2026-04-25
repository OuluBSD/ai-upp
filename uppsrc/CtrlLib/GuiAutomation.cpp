#include "CtrlLib.h"
#include "GuiAutomation.h"

NAMESPACE_UPP

static String GetSemanticCtrlType(Ctrl& c)
{
	if(dynamic_cast<MenuBar *>(&c))
		return "MenuBar";
	if(dynamic_cast<ToolBar *>(&c))
		return "ToolBar";
	if(dynamic_cast<Bar *>(&c))
		return "Bar";
	if(dynamic_cast<TopWindow *>(&c))
		return "Window";
	if(dynamic_cast<Button *>(&c))
		return "Button";
	if(dynamic_cast<EditField *>(&c))
		return "Edit";
	if(dynamic_cast<LabelBase *>(&c))
		return "Label";
	return "Ctrl";
}

static void ComputeVisibleText(AutomationElement& el, int width_px)
{
	String text = TrimBoth(el.text);
	if(text.IsEmpty()) {
		el.visible_text.Clear();
		el.visible_text_ratio = 0.0;
		return;
	}

	if(width_px <= 0) {
		el.visible_text.Clear();
		el.visible_text_ratio = 0.0;
		return;
	}

	int full_px = GetTextSize(text, StdFont()).cx;
	if(full_px <= width_px) {
		el.visible_text = text;
		el.visible_text_ratio = 1.0;
		return;
	}

	int lo = 0;
	int hi = text.GetCount();
	while(lo < hi) {
		int mid = (lo + hi + 1) / 2;
		if(GetTextSize(text.Left(mid), StdFont()).cx <= width_px)
			lo = mid;
		else
			hi = mid - 1;
	}

	el.visible_text = text.Left(lo);
	el.visible_text_ratio = text.GetCount() ? (double)lo / (double)text.GetCount() : 1.0;
}

static Rect GetTopRelativeScreenRect(const Ctrl& c)
{
	Rect r = c.GetScreenRect();
	const Ctrl *top = c.GetTopCtrl();
	if(top)
		r -= top->GetScreenRect().TopLeft();
	return r;
}

static int SemanticDepth(const String& semantic_path)
{
	String s = TrimBoth(semantic_path);
	if(s.IsEmpty())
		return 0;
	int depth = 1;
	int pos = 0;
	while(true) {
		int q = s.Find("->", pos);
		if(q < 0)
			break;
		depth++;
		pos = q + 2;
	}
	return depth;
}

static bool IsDirectSemanticChild(const String& parent_semantic_path, const String& child_semantic_path)
{
	String parent = TrimBoth(parent_semantic_path);
	String child = TrimBoth(child_semantic_path);
	if(parent.IsEmpty() || child.IsEmpty())
		return false;
	String prefix = parent + " -> ";
	if(!child.StartsWith(prefix))
		return false;
	return SemanticDepth(child) == SemanticDepth(parent) + 1;
}

static Rect ClampToOuter(const Rect& r, const Rect& outer)
{
	Rect q = r;
	q.left = minmax(q.left, outer.left, outer.right);
	q.right = minmax(q.right, outer.left, outer.right);
	q.top = minmax(q.top, outer.top, outer.bottom);
	q.bottom = minmax(q.bottom, outer.top, outer.bottom);
	return q;
}

static void AddCtrlFrameRegions(GuiAutomationVisitor& vis, Ctrl& c, bool is_visible, const Rect& ctrl_rect)
{
	if(vis.write_mode || ctrl_rect.IsEmpty())
		return;

	Rect outer = Rect(c.GetRect().Size());
	if(outer.IsEmpty())
		return;

	Rect view = ClampToOuter(c.GetView(), outer);
	if(view == outer)
		return;

	struct Part {
		const char *id;
		const char *name;
		Rect rect;
	};

	Part parts[] = {
		{ "top",    "Top",    Rect(outer.left, outer.top, outer.right, view.top) },
		{ "left",   "Left",   Rect(outer.left, view.top, view.left, view.bottom) },
		{ "right",  "Right",  Rect(view.right, view.top, outer.right, view.bottom) },
		{ "bottom", "Bottom", Rect(outer.left, view.bottom, outer.right, outer.bottom) },
	};

	for(const Part& p : parts) {
		if(p.rect.IsEmpty())
			continue;

		AutomationElement& el = vis.elements.Add();
		el.text = String("@frame ") + p.name;
		el.path = vis.current_path;
		if(!el.path.IsEmpty())
			el.path << "/";
		el.path << "@frame/" << p.id << "/" << ctrl_rect.left << "," << ctrl_rect.top;
		vis.SetElementSemantic(el, "Frame", p.name);
		el.visible = is_visible;
		el.enabled = false;
		el.checked = false;
		el.is_menu = false;
		el.rect = p.rect + ctrl_rect.TopLeft();
		el.visible_text.Clear();
		el.visible_text_ratio = 0.0;
	}
}

static void ApplyBarItemGeometry(GuiAutomationVisitor& vis, Ctrl& c, int first, int end, const String& scope_semantic_path, const Rect& fallback_rect)
{
	const MenuBar *menu = dynamic_cast<const MenuBar *>(&c);
	const ToolBar *tool = dynamic_cast<const ToolBar *>(&c);
	if(!menu && !tool)
		return;

	Vector<Rect> item_rect;
	Vector<bool> item_visible;

	int count = menu ? menu->GetAutomationItemCount() : tool->GetAutomationItemCount();
	for(int i = 0; i < count; i++) {
		const Ctrl *ci = menu ? menu->GetAutomationItemCtrl(i) : tool->GetAutomationItemCtrl(i);
		if(!ci)
			continue;
		item_rect.Add(GetTopRelativeScreenRect(*ci));
		item_visible.Add(ci->IsVisible());
	}

	int direct_i = 0;
	Rect current_direct_rect = fallback_rect;

	for(int i = first; i < end; i++) {
		AutomationElement& el = vis.elements[i];
		if(IsDirectSemanticChild(scope_semantic_path, el.semantic_path)) {
			if(direct_i < item_rect.GetCount()) {
				el.rect = item_rect[direct_i];
				el.visible = el.visible && item_visible[direct_i];
				current_direct_rect = el.rect;
			}
			else
				el.rect = fallback_rect;
			direct_i++;
		}
		else {
			// Closed submenu entries are not onscreen; keep a parent anchor but mark hidden.
			if(!IsNull(current_direct_rect))
				el.rect = current_direct_rect;
			else
				el.rect = fallback_rect;
			el.visible = false;
		}
	}
}

Bar::Item& AutomationBar::Item::Text(const char *text) {
	el.text = text;
	el.path = v.current_path;
	if(el.text.GetCount()) {
		if(el.path.GetCount()) el.path << "/";
		el.path << el.text;
	}
	bool menu_context = v.current_semantic_path.Find("MenuBar") >= 0;
	el.semantic_type = (menu_context || el.is_menu) ? "Bar" : "Action";
	el.semantic_name = TrimBoth(el.text);
	String node = v.BuildSemanticNode(el.semantic_type, el.semantic_name);
	el.semantic_path = v.current_semantic_path.IsEmpty() ? node : v.current_semantic_path + " -> " + node;

	if(v.write_mode) {
		if(el.path == v.target_path || el.semantic_path == v.target_path) {
			v.found = true;
			if(v.target_action && callback) {
				callback();
			}
		}
	}

	if(submenu_proc) {
		String old_path = v.current_path;
		String old_semantic = v.current_semantic_path;
		v.current_path = el.path;
		v.current_semantic_path = el.semantic_path;
		AutomationBar ab(v);
		submenu_proc(ab);
		v.current_semantic_path = old_semantic;
		v.current_path = old_path;
	}
	return *this;
}

Bar::Item& AutomationBar::Item::Key(dword key) { return *this; }
Bar::Item& AutomationBar::Item::Image(const ::Upp::Image& img) { return *this; }
Bar::Item& AutomationBar::Item::Check(bool check) { el.checked = check; return *this; }
Bar::Item& AutomationBar::Item::Radio(bool check) { el.checked = check; return *this; }
Bar::Item& AutomationBar::Item::Enable(bool _enable) { el.enabled = _enable; return *this; }
Bar::Item& AutomationBar::Item::Bold(bool bold) { return *this; }
Bar::Item& AutomationBar::Item::Tip(const char *tip) { return *this; }
Bar::Item& AutomationBar::Item::Help(const char *help) { return *this; }
Bar::Item& AutomationBar::Item::Topic(const char *topic) { return *this; }
Bar::Item& AutomationBar::Item::Description(const char *desc) { return *this; }
Bar::Item& AutomationBar::Item::AccessValue(const ::Upp::Value& val) { el.value = val; return *this; }

Bar::Item& AutomationBar::AddItem(Event<> cb) {
	v.AccessAction("", cb);
	Item& it = automation_items.Add(new Item(v, v.elements.Top()));
	it.callback = cb;
	return it;
}

Bar::Item& AutomationBar::AddSubMenu(Event<Bar&> proc) {
	v.AccessAction("", []{});
	AutomationElement& el = v.elements.Top();
	el.is_menu = true;
	Item& it = automation_items.Add(new Item(v, el));
	it.submenu_proc = proc;
	return it;
}

void AutomationBar::AddCtrl(Ctrl *ctrl, int gapsize) { if(ctrl) ctrl->Access(v); }
void AutomationBar::AddCtrl(Ctrl *ctrl, Size sz) { if(ctrl) ctrl->Access(v); }
bool AutomationBar::IsEmpty() const { return false; }
void AutomationBar::Separator() {}

void GuiAutomationVisitor::Read(Ctrl& c)
{
	Cout() << "GuiAutomation READ: " << c.GetLayoutId() << "\n";
	elements.Clear();
	automation_items.Clear();
	synthetic_id = 0;
	current_path = "";
	current_semantic_path = "";
	write_mode = false;
	Walk(c, true);
}

void GuiAutomationVisitor::Walk(Ctrl& c, bool parent_visible)
{
	if(found && write_mode) return;
	
	bool is_visible = parent_visible && c.IsVisible();
	if(!is_visible && !include_hidden) return;
	
	::Upp::String old_path = current_path;
	::Upp::String old_semantic = current_semantic_path;

	// Update path with LayoutId if present
	String l = c.GetLayoutId();
	if(!l.IsEmpty()) {
		if(!current_path.IsEmpty()) current_path << "/";
		current_path << l;
	}

	String scope_type = GetSemanticCtrlType(c);
	String scope_name = TrimBoth(l);
	if(!scope_type.IsEmpty()) {
		String node = BuildSemanticNode(scope_type, scope_name);
		if(current_semantic_path.IsEmpty())
			current_semantic_path = node;
		else
			current_semantic_path << " -> " << node;
	}

	int n0 = elements.GetCount();
	String scope_semantic_path = current_semantic_path;
	bool handled = c.Access(*(AutomationVisitor*)this);
	int n1 = elements.GetCount();
	Rect ctrl_rect = GetTopRelativeScreenRect(c);

	if(n1 == n0 && !write_mode) {
		AutomationElement& el = elements.Add();
		el.path = current_path;
		if(!el.path.IsEmpty())
			el.path << "/";
		el.path << "@ctrl/" << ++synthetic_id;
		el.text = scope_name.IsEmpty() ? scope_type : scope_name;
		if(el.text.IsEmpty())
			el.text = "Ctrl";
		SetElementSemantic(el, scope_type.IsEmpty() ? "Ctrl" : scope_type, scope_name);
		el.visible = is_visible;
		el.enabled = c.IsEnabled();
		el.checked = false;
		el.is_menu = false;
		el.rect = ctrl_rect;
		ComputeVisibleText(el, c.GetSize().cx);
		n1 = elements.GetCount();
	}
	
	// Ensure elements added during Access have the correct path prefix if not already set
	bool is_bar_ctrl = dynamic_cast<MenuBar *>(&c) || dynamic_cast<ToolBar *>(&c);
	for(int i = n0; i < n1; i++) {
		elements[i].visible = is_visible;
		if(!is_bar_ctrl && IsNull(elements[i].rect))
			elements[i].rect = ctrl_rect;
		ComputeVisibleText(elements[i], c.GetSize().cx);
		if(elements[i].semantic_path.IsEmpty())
			SetElementSemantic(elements[i], "Element", elements[i].text);
		// If the element's path is just its text, prefix it with current_path
		if(!current_path.IsEmpty() && !elements[i].path.StartsWith(current_path)) {
			String p = current_path;
			if(elements[i].path.GetCount() && elements[i].path[0] != '/')
				p << "/";
			p << elements[i].path;
			elements[i].path = p;
		}
	}
	ApplyBarItemGeometry(*this, c, n0, n1, scope_semantic_path, ctrl_rect);
	AddCtrlFrameRegions(*this, c, is_visible, ctrl_rect);

	if(handled && dynamic_cast<Bar*>(&c)) {
		current_semantic_path = old_semantic;
		current_path = old_path;
		return;
	}

	::Upp::Vector<CtrlGeometry> children;
	for(Ctrl& child : c) {
		CtrlGeometry& g = children.Add();
		g.ctrl = &child;
		g.rect = child.GetRect();
	}
	
	::Upp::Sort(children);
	
	for(const auto& g : children) {
		Walk(*g.ctrl, is_visible);
	}
	
	current_semantic_path = old_semantic;
	current_path = old_path;
}

::Upp::Value GuiAutomationVisitor::Read(Ctrl& c, const String& path)
{
	Read(c);
	for(const auto& el : elements)
		if(el.path == path || el.semantic_path == path)
			return el.value;
	return ::Upp::Value();
}

bool GuiAutomationVisitor::Write(Ctrl& c, const String& path, const ::Upp::Value& val, bool do_action)
{
	Cout() << "GuiAutomation WRITE: " << path << " action=" << do_action << "\n";
	write_mode = true;
	target_path = path;
	target_value = val;
	target_action = do_action;
	found = false;
	current_path = "";
	Walk(c, true);
	return found;
}

END_UPP_NAMESPACE
