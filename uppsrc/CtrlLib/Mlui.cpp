#include "CtrlLib.h"

#include <cstdio>

NAMESPACE_UPP

static VectorMap<String, One<MluiFocusPage>>& MluiFocusRegistry()
{
	static VectorMap<String, One<MluiFocusPage>> pages;
	return pages;
}

template <class T>
static Vector<int> MluiSortedMapOrder(const VectorMap<String, T>& map)
{
	Vector<int> order;
	order.SetCount(map.GetCount());
	for(int i = 0; i < order.GetCount(); i++)
		order[i] = i;
	Sort(order, [&](int a, int b) {
		return map.GetKey(a) < map.GetKey(b);
	});
	return order;
}

static String MluiCtrlRef(const Ctrl& c)
{
	String id = c.GetLayoutId();
	if(id.IsEmpty())
		id = String().Cat() << Nvl(Ctrl::Name(&c), String("Ctrl")) << "@" << HexStr((uintptr_t)&c);
	return id;
}

MluiFocusPage& MluiFocusPage::Add(Ctrl& c)
{
	String id = MluiCtrlRef(c);
	if(FindIndex(linked_ctrls, id) < 0)
		linked_ctrls.Add(id);
	return *this;
}

MluiFocusPage& MluiFocusPage::Action(const String& action_id, const String& label, const String& hint, bool enabled)
{
	for(MluiFocusAction& a : actions) {
		if(a.id == action_id) {
			a.label = label;
			a.hint = hint;
			a.enabled = enabled;
			return *this;
		}
	}
	MluiFocusAction& a = actions.Add();
	a.id = action_id;
	a.label = label;
	a.hint = hint;
	a.enabled = enabled;
	return *this;
}

MluiFocusPage& MluiFocusPage::ActionHandler(const String& action_id, Function<Value(const ValueMap&)> handler)
{
	action_handlers.GetAdd(action_id) = pick(handler);
	return *this;
}

MluiFocusPage& MluiFocusPage::Context(const String& key, const Value& value)
{
	static_context.GetAdd(key) = value;
	return *this;
}

MluiFocusPage& MluiFocusPage::ClearRuntime()
{
	runtime_values.Clear();
	runtime_value_desc.Clear();
	runtime_ctrls.Clear();
	runtime_ctrl_desc.Clear();
	runtime_action_enabled.Clear();
	runtime_action_desc.Clear();
	return *this;
}

MluiFocusPage& MluiFocusPage::AddValue(const String& key, const Value& value, const String& description)
{
	runtime_values.GetAdd(key) = value;
	runtime_value_desc.GetAdd(key) = description;
	return *this;
}

MluiFocusPage& MluiFocusPage::AddCtrl(const String& key, const Ctrl& ctrl, const String& description)
{
	runtime_ctrls.GetAdd(key) = MluiCtrlRef(ctrl);
	runtime_ctrl_desc.GetAdd(key) = description;
	return *this;
}

MluiFocusPage& MluiFocusPage::AddState(const String& key, const Value& value, const String& description)
{
	String k = String().Cat() << "state." << key;
	runtime_values.GetAdd(k) = value;
	runtime_value_desc.GetAdd(k) = description;
	return *this;
}

MluiFocusPage& MluiFocusPage::AddAction(const String& action_id, bool enabled, const String& description)
{
	runtime_action_enabled.GetAdd(action_id) = enabled;
	runtime_action_desc.GetAdd(action_id) = description;
	return *this;
}

MluiFocusPage& RegisterMluiFocusPage(const String& id, const String& title, const String& description)
{
	VectorMap<String, One<MluiFocusPage>>& pages = MluiFocusRegistry();
	int i = pages.Find(id);
	if(i < 0) {
		One<MluiFocusPage>& op = pages.Add(id);
		op.Create();
		MluiFocusPage& p = *op;
		p.id = id;
		p.title = title;
		p.description = description;
		return p;
	}
	MluiFocusPage& p = *pages[i];
	p.title = title;
	p.description = description;
	return p;
}

MluiFocusPage& GetMluiFocusPage(const String& id)
{
	VectorMap<String, One<MluiFocusPage>>& pages = MluiFocusRegistry();
	int i = pages.Find(id);
	if(i < 0)
		return RegisterMluiFocusPage(id, id, "No description");
	return *pages[i];
}

bool HasMluiFocusPage(const String& id)
{
	return MluiFocusRegistry().Find(id) >= 0;
}

static bool MluiMatchPattern(const String& text, const String& pattern)
{
	String p = ToLower(TrimBoth(pattern));
	if(p.IsEmpty())
		return true;
	String t = ToLower(text);
	if(p.Find('*') < 0)
		return t.Find(p) >= 0;

	Vector<String> parts = Split(p, '*');
	int pos = 0;
	bool anchored_start = !p.StartsWith("*");
	bool anchored_end = !p.EndsWith("*");
	bool first_nonempty = true;
	String last_nonempty;
	for(const String& part0 : parts) {
		String part = TrimBoth(part0);
		if(part.IsEmpty())
			continue;
		int q = t.Find(part, pos);
		if(q < 0)
			return false;
		if(first_nonempty && anchored_start && q != 0)
			return false;
		first_nonempty = false;
		last_nonempty = part;
		pos = q + part.GetCount();
	}
	if(anchored_end && !last_nonempty.IsEmpty() && !t.EndsWith(last_nonempty))
		return false;
	return true;
}

static bool MluiGetActionEnabled(const MluiFocusPage& page, const MluiFocusAction& action)
{
	int i = page.runtime_action_enabled.Find(action.id);
	if(i >= 0)
		return page.runtime_action_enabled[i];
	return action.enabled;
}

static String MluiGetActionDescription(const MluiFocusPage& page, const String& action_id)
{
	int i = page.runtime_action_desc.Find(action_id);
	return i >= 0 ? page.runtime_action_desc[i] : String();
}

static ValueMap MluiFocusPageToJson(const MluiFocusPage& page)
{
	ValueMap out;
	out.Add("id", page.id);
	out.Add("title", page.title);
	out.Add("description", page.description);

	ValueMap context;
	for(int i : MluiSortedMapOrder(page.static_context))
		context.Add(page.static_context.GetKey(i), page.static_context[i]);
	out.Add("context", context);

	ValueArray linked;
	Vector<String> linked_sorted = clone(page.linked_ctrls);
	Sort(linked_sorted);
	for(const String& c : linked_sorted)
		linked.Add(c);
	out.Add("linked_ctrls", linked);

	ValueArray actions;
	Vector<int> action_order;
	action_order.SetCount(page.actions.GetCount());
	for(int i = 0; i < action_order.GetCount(); i++)
		action_order[i] = i;
	Sort(action_order, [&](int a, int b) {
		return page.actions[a].id < page.actions[b].id;
	});
	for(int ai : action_order) {
		const MluiFocusAction& a = page.actions[ai];
		ValueMap item;
		item.Add("id", a.id);
		item.Add("label", a.label);
		item.Add("hint", a.hint);
		item.Add("enabled", MluiGetActionEnabled(page, a));
		String desc = MluiGetActionDescription(page, a.id);
		if(!desc.IsEmpty())
			item.Add("runtime_description", desc);
		item.Add("has_handler", page.action_handlers.Find(a.id) >= 0);
		actions.Add(item);
	}
	out.Add("actions", actions);

	ValueArray values;
	for(int i : MluiSortedMapOrder(page.runtime_values)) {
		ValueMap item;
		String key = page.runtime_values.GetKey(i);
		item.Add("key", key);
		item.Add("value", page.runtime_values[i]);
		int di = page.runtime_value_desc.Find(key);
		if(di >= 0)
			item.Add("description", page.runtime_value_desc[di]);
		values.Add(item);
	}

	ValueArray ctrls;
	for(int i : MluiSortedMapOrder(page.runtime_ctrls)) {
		ValueMap item;
		String key = page.runtime_ctrls.GetKey(i);
		item.Add("key", key);
		item.Add("ref", page.runtime_ctrls[i]);
		int di = page.runtime_ctrl_desc.Find(key);
		if(di >= 0)
			item.Add("description", page.runtime_ctrl_desc[di]);
		ctrls.Add(item);
	}

	ValueArray runtime_actions;
	for(int i : MluiSortedMapOrder(page.runtime_action_enabled)) {
		ValueMap item;
		String key = page.runtime_action_enabled.GetKey(i);
		item.Add("id", key);
		item.Add("enabled", page.runtime_action_enabled[i]);
		int di = page.runtime_action_desc.Find(key);
		if(di >= 0)
			item.Add("description", page.runtime_action_desc[di]);
		runtime_actions.Add(item);
	}

	ValueMap runtime;
	runtime.Add("values", values);
	runtime.Add("ctrls", ctrls);
	runtime.Add("actions", runtime_actions);
	out.Add("runtime", runtime);

	return out;
}

Value BuildMluiFocusList()
{
	ValueArray out;
	MluiGuiCall([&] {
		const VectorMap<String, One<MluiFocusPage>>& pages = MluiFocusRegistry();
		for(int i : MluiSortedMapOrder(pages)) {
			const MluiFocusPage& p = *pages[i];
			ValueMap item;
			item.Add("id", p.id);
			item.Add("title", p.title);
			item.Add("description", p.description);
			item.Add("action_count", p.actions.GetCount());
			item.Add("runtime_value_count", p.runtime_values.GetCount());
			item.Add("runtime_ctrl_count", p.runtime_ctrls.GetCount());
			out.Add(item);
		}
	});
	return out;
}

Value BuildMluiFocusPage(const String& id)
{
	Value out;
	MluiGuiCall([&] {
		const VectorMap<String, One<MluiFocusPage>>& pages = MluiFocusRegistry();
		int i = pages.Find(id);
		if(i >= 0)
			out = MluiFocusPageToJson(*pages[i]);
	});
	return out;
}

Value BuildMluiFocusTree(const String& id, int depth)
{
	depth = max(1, depth);
	ValueMap out;
	ValueArray lines;
	MluiGuiCall([&] {
		const VectorMap<String, One<MluiFocusPage>>& pages = MluiFocusRegistry();
		lines.Add("focus");
		if(!id.IsEmpty()) {
			int i = pages.Find(id);
			if(i < 0)
				return;
			const MluiFocusPage& p = *pages[i];
			String root = String().Cat() << "  " << p.id << " (" << p.title << ")";
			lines.Add(root);
			if(depth >= 2) {
				lines.Add(String().Cat() << "    context: " << p.static_context.GetCount());
				lines.Add(String().Cat() << "    actions: " << p.actions.GetCount());
				lines.Add(String().Cat() << "    runtime.values: " << p.runtime_values.GetCount());
				lines.Add(String().Cat() << "    runtime.ctrls: " << p.runtime_ctrls.GetCount());
			}
		}
		else {
			for(int i : MluiSortedMapOrder(pages)) {
				const MluiFocusPage& p = *pages[i];
				String row = String().Cat() << "  " << p.id << " (" << p.title << ")";
				lines.Add(row);
				if(depth >= 2) {
					lines.Add(String().Cat() << "    actions: " << p.actions.GetCount()
					                         << ", runtime.values: " << p.runtime_values.GetCount()
					                         << ", runtime.ctrls: " << p.runtime_ctrls.GetCount());
				}
			}
		}
	});
	out.Add("depth", depth);
	out.Add("page_id", id);
	out.Add("lines", lines);
	return out;
}

Value SearchMluiFocus(const String& query, const String& page_filter, int limit)
{
	limit = max(1, limit);
	ValueArray out;
	MluiGuiCall([&] {
		const VectorMap<String, One<MluiFocusPage>>& pages = MluiFocusRegistry();
		for(int i : MluiSortedMapOrder(pages)) {
			if(out.GetCount() >= limit)
				break;
			const MluiFocusPage& p = *pages[i];
			if(!page_filter.IsEmpty() && p.id != page_filter)
				continue;

			auto AddMatch = [&](const String& kind, const String& key, const String& text) {
				if(out.GetCount() >= limit || !MluiMatchPattern(text, query))
					return;
				ValueMap m;
				m.Add("page_id", p.id);
				m.Add("kind", kind);
				m.Add("key", key);
				m.Add("text", text);
				out.Add(m);
			};

			AddMatch("page", p.id, p.id + " " + p.title + " " + p.description);
			for(int j : MluiSortedMapOrder(p.runtime_values)) {
				if(out.GetCount() >= limit)
					break;
				String key = p.runtime_values.GetKey(j);
				AddMatch("runtime.value", key, key + " " + AsString(p.runtime_values[j]));
			}
			for(int j : MluiSortedMapOrder(p.runtime_ctrls)) {
				if(out.GetCount() >= limit)
					break;
				String key = p.runtime_ctrls.GetKey(j);
				AddMatch("runtime.ctrl", key, key + " " + p.runtime_ctrls[j]);
			}
			Vector<int> action_order;
			action_order.SetCount(p.actions.GetCount());
			for(int j = 0; j < action_order.GetCount(); j++)
				action_order[j] = j;
			Sort(action_order, [&](int a, int b) {
				return p.actions[a].id < p.actions[b].id;
			});
			for(int aj : action_order) {
				if(out.GetCount() >= limit)
					break;
				const MluiFocusAction& a = p.actions[aj];
				AddMatch("action", a.id, a.id + " " + a.label + " " + a.hint);
			}
		}
	});
	return out;
}

bool PerformMluiFocusAction(const String& page_id, const String& action_id, const ValueMap& args,
                            Value& out_result, String& out_error)
{
	bool ok = false;
	MluiGuiCall([&] {
		VectorMap<String, One<MluiFocusPage>>& pages = MluiFocusRegistry();
		int pi = pages.Find(page_id);
		if(pi < 0) {
			out_error = "Focus page not found: " + page_id;
			return;
		}

		MluiFocusPage& page = *pages[pi];
		const MluiFocusAction *action = NULL;
		for(MluiFocusAction& a : page.actions)
			if(a.id == action_id) {
				action = &a;
				break;
			}
		if(!action) {
			out_error = "Focus action not found: " + action_id;
			return;
		}
		if(!MluiGetActionEnabled(page, *action)) {
			String desc = MluiGetActionDescription(page, action_id);
			out_error = "Focus action disabled: " + action_id + (desc.IsEmpty() ? String() : " (" + desc + ")");
			return;
		}

		int hi = page.action_handlers.Find(action_id);
		if(hi < 0) {
			out_error = "Focus action handler not set: " + action_id;
			return;
		}

		Function<Value(const ValueMap&)>& fn = page.action_handlers[hi];
		if(!fn) {
			out_error = "Focus action handler empty: " + action_id;
			return;
		}

		out_result = fn(args);
		ok = true;
	});
	return ok;
}

static bool MluiGetBool(const ValueMap& map, const char *key, bool def)
{
	Value v = map[key];
	if(IsNull(v))
		return def;
	return (bool)v;
}

static int MluiGetInt(const ValueMap& map, const char *key, int def)
{
	Value v = map[key];
	if(IsNull(v))
		return def;
	return (int)v;
}

static String MluiGetString(const ValueMap& map, const char *key, const String& def = String())
{
	Value v = map[key];
	if(IsNull(v))
		return def;
	return (String)v;
}

static void MluiCollectElements(bool include_hidden, Array<AutomationElement>& out)
{
	GuiAutomationVisitor vis;
	vis.include_hidden = include_hidden;
	Vector<Ctrl *> top = Ctrl::GetTopCtrls();
	for(Ctrl *c : top) {
		if(!c)
			continue;
		if(c->IsVisible() || include_hidden) {
			vis.Read(*c);
			for(const AutomationElement& el : vis.elements)
				out.Add(el);
		}
	}
}

static void MluiRefreshFocusRuntime(bool include_hidden)
{
	Array<AutomationElement> ignored;
	MluiGuiCall([&] {
		MluiCollectElements(include_hidden, ignored);
	});
}

static ValueMap MluiElementToJson(const AutomationElement& el, const MluiSnapshotOptions& options)
{
	ValueMap item;
	item.Add("path", el.path);
	item.Add("type", el.semantic_type);
	item.Add("semantic_name", el.semantic_name);
	item.Add("semantic_path", el.semantic_path);
	item.Add("text", el.text);
	item.Add("value", el.value);
	item.Add("checked", el.checked);
	item.Add("enabled", el.enabled);
	item.Add("visible", el.visible);
	item.Add("is_menu", el.is_menu);

	if(options.include_layout) {
		ValueMap rect;
		if(!IsNull(el.rect)) {
			rect.Add("left", el.rect.left);
			rect.Add("top", el.rect.top);
			rect.Add("right", el.rect.right);
			rect.Add("bottom", el.rect.bottom);
			rect.Add("width", el.rect.GetWidth());
			rect.Add("height", el.rect.GetHeight());
		}
		item.Add("rect", rect);
	}
	if(options.include_visible_text)
		item.Add("visible_text", el.visible_text);
	if(options.include_visible_text_ratio)
		item.Add("visible_text_ratio", el.visible_text_ratio);

	return item;
}

Value BuildMluiSnapshot(const MluiSnapshotOptions& options)
{
	ValueArray arr;
	MluiGuiCall([&] {
		Array<AutomationElement> elements;
		MluiCollectElements(options.include_hidden, elements);
		for(const AutomationElement& el : elements)
			arr.Add(MluiElementToJson(el, options));
	});

	ValueMap out;
	out.Add("timestamp", Format("%", GetSysTime()));
	out.Add("count", arr.GetCount());
	out.Add("elements", arr);
	if(options.include_focus) {
		Value pages = BuildMluiFocusList();
		ValueMap focus;
		focus.Add("pages", pages);
		if(IsValueArray(pages))
			focus.Add("count", ((const ValueArray&)pages).GetCount());
		else
			focus.Add("count", 0);
		out.Add("focus", focus);
	}
	return out;
}

static bool MluiWriteByPath(const String& path, const Value& value, bool do_action, bool include_hidden)
{
	if(path.IsEmpty())
		return false;

	bool found = false;
	MluiGuiCall([&] {
		GuiAutomationVisitor vis;
		vis.include_hidden = include_hidden;
		Vector<Ctrl *> top = Ctrl::GetTopCtrls();
		for(Ctrl *c : top) {
			if(c && (c->IsVisible() || include_hidden)) {
				if(vis.Write(*c, path, value, do_action)) {
					found = true;
					break;
				}
			}
		}
	});
	return found;
}

static String MluiFindPathByPoint(Point p, bool include_hidden)
{
	Array<AutomationElement> elements;
	MluiGuiCall([&] {
		MluiCollectElements(include_hidden, elements);
	});

	int best = -1;
	int best_area = INT_MAX;
	for(int i = 0; i < elements.GetCount(); i++) {
		const AutomationElement& el = elements[i];
		if(el.path.IsEmpty() || IsNull(el.rect))
			continue;
		if(!el.rect.Contains(p))
			continue;
		int area = max(1, el.rect.GetWidth()) * max(1, el.rect.GetHeight());
		if(area <= best_area) {
			best_area = area;
			best = i;
		}
	}

	return best >= 0 ? elements[best].path : String();
}

static bool MluiSendKey(dword key, int count)
{
	bool ok = false;
	MluiGuiCall([&] {
		Ctrl *focus = Ctrl::GetFocusCtrl();
		if(focus) {
			ok = focus->Key(key, count);
			return;
		}

		Vector<Ctrl *> top = Ctrl::GetTopCtrls();
		for(Ctrl *c : top) {
			if(c && c->IsVisible()) {
				if(c->Key(key, count)) {
					ok = true;
					return;
				}
			}
		}
	});
	return ok;
}

String HandleMluiJsonRequest(const String& request_json)
{
	MluiRequest req;
	String parse_error;
	if(!MluiParseJsonRequest(request_json, req, parse_error))
		return MluiMakeJsonResponse(false, Value(), Value(), parse_error);

	Value id = req.id;
	String method = req.method;
	ValueMap p = req.params;

	if(method == "ping") {
		ValueMap m;
		m.Add("text", "pong");
		return MluiMakeJsonResponse(true, id, m);
	}

	if(method == "snapshot") {
		MluiSnapshotOptions o;
		o.include_hidden = MluiGetBool(p, "include_hidden", false);
		o.include_layout = MluiGetBool(p, "include_layout", true);
		o.include_visible_text = MluiGetBool(p, "include_visible_text", true);
		o.include_visible_text_ratio = MluiGetBool(p, "include_visible_text_ratio", true);
		o.include_focus = MluiGetBool(p, "include_focus", true);

		String mode = ToLower(MluiGetString(p, "visibility_mode", "both"));
		if(mode == "none") {
			o.include_visible_text = false;
			o.include_visible_text_ratio = false;
		}
		else if(mode == "absolute" || mode == "text") {
			o.include_visible_text = true;
			o.include_visible_text_ratio = false;
		}
		else if(mode == "ratio" || mode == "percent") {
			o.include_visible_text = false;
			o.include_visible_text_ratio = true;
		}

		return MluiMakeJsonResponse(true, id, BuildMluiSnapshot(o));
	}

	if(method == "focus.list")
	{
		MluiRefreshFocusRuntime(MluiGetBool(p, "include_hidden", false));
		return MluiMakeJsonResponse(true, id, BuildMluiFocusList());
	}

	if(method == "focus.get") {
		MluiRefreshFocusRuntime(MluiGetBool(p, "include_hidden", false));
		String page_id = MluiGetString(p, "page_id");
		if(page_id.IsEmpty())
			page_id = MluiGetString(p, "page");
		if(page_id.IsEmpty())
			page_id = MluiGetString(p, "id");
		if(page_id.IsEmpty())
			return MluiMakeJsonResponse(false, id, Value(), "Missing page_id/page/id");
		Value page = BuildMluiFocusPage(page_id);
		if(IsNull(page))
			return MluiMakeJsonResponse(false, id, Value(), "Focus page not found: " + page_id);
		return MluiMakeJsonResponse(true, id, page);
	}

	if(method == "focus.tree") {
		MluiRefreshFocusRuntime(MluiGetBool(p, "include_hidden", false));
		String page_id = MluiGetString(p, "page_id");
		if(page_id.IsEmpty())
			page_id = MluiGetString(p, "page");
		int depth = MluiGetInt(p, "depth", 2);
		if(depth < 1 || depth > 16)
			return MluiMakeJsonResponse(false, id, Value(), "depth must be in range [1..16]");
		return MluiMakeJsonResponse(true, id, BuildMluiFocusTree(page_id, depth));
	}

	if(method == "focus.search") {
		MluiRefreshFocusRuntime(MluiGetBool(p, "include_hidden", false));
		String q = MluiGetString(p, "query");
		if(q.IsEmpty())
			q = MluiGetString(p, "pattern");
		if(q.IsEmpty())
			q = MluiGetString(p, "q");
		if(q.IsEmpty())
			return MluiMakeJsonResponse(false, id, Value(), "Missing query/pattern/q");
		String page_filter = MluiGetString(p, "page");
		if(page_filter.IsEmpty())
			page_filter = MluiGetString(p, "page_id");
		int limit = MluiGetInt(p, "limit", 64);
		if(limit < 1 || limit > 1024)
			return MluiMakeJsonResponse(false, id, Value(), "limit must be in range [1..1024]");
		return MluiMakeJsonResponse(true, id, SearchMluiFocus(q, page_filter, limit));
	}

	if(method == "focus.action") {
		MluiRefreshFocusRuntime(MluiGetBool(p, "include_hidden", false));
		String page_id = MluiGetString(p, "page_id");
		if(page_id.IsEmpty())
			page_id = MluiGetString(p, "page");
		String action_id = MluiGetString(p, "action_id");
		if(action_id.IsEmpty())
			action_id = MluiGetString(p, "action");
		if(action_id.IsEmpty())
			action_id = MluiGetString(p, "id");
		if(page_id.IsEmpty() || action_id.IsEmpty())
			return MluiMakeJsonResponse(false, id, Value(), "Missing page_id/page and action_id/action");

		ValueMap args;
		Value av = p.Get("args", ValueMap());
		if(IsValueMap(av))
			args = av;
		else {
			Value pv = p.Get("payload", ValueMap());
			if(IsValueMap(pv))
				args = pv;
		}

		Value result;
		String error;
		if(!PerformMluiFocusAction(page_id, action_id, args, result, error))
			return MluiMakeJsonResponse(false, id, Value(), error);
		if(IsNull(result)) {
			ValueMap ok;
			ok.Add("ok", true);
			ok.Add("page_id", page_id);
			ok.Add("action_id", action_id);
			return MluiMakeJsonResponse(true, id, ok);
		}
		return MluiMakeJsonResponse(true, id, result);
	}

	if(method == "click") {
		bool include_hidden = MluiGetBool(p, "include_hidden", false);
		String path = MluiGetString(p, "path");
		if(path.IsEmpty() && !IsNull(p.Get("x", Value())) && !IsNull(p.Get("y", Value()))) {
			Point pt(MluiGetInt(p, "x", 0), MluiGetInt(p, "y", 0));
			path = MluiFindPathByPoint(pt, include_hidden);
		}
		if(path.IsEmpty())
			return MluiMakeJsonResponse(false, id, Value(), "Missing target path/point");
		if(!MluiWriteByPath(path, Value(), true, include_hidden))
			return MluiMakeJsonResponse(false, id, Value(), "Click target not found: " + path);
		ValueMap m;
		m.Add("path", path);
		m.Add("clicked", true);
		return MluiMakeJsonResponse(true, id, m);
	}

	if(method == "set") {
		bool include_hidden = MluiGetBool(p, "include_hidden", false);
		String path = MluiGetString(p, "path");
		if(path.IsEmpty())
			return MluiMakeJsonResponse(false, id, Value(), "Missing path");
		Value value = p.Get("value", Value());
		if(!MluiWriteByPath(path, value, false, include_hidden))
			return MluiMakeJsonResponse(false, id, Value(), "Set target not found: " + path);
		ValueMap m;
		m.Add("path", path);
		m.Add("set", true);
		return MluiMakeJsonResponse(true, id, m);
	}

	if(method == "key") {
		String text = MluiGetString(p, "text");
		if(!text.IsEmpty()) {
			bool ok = true;
			WString w = text.ToWString();
			for(int i = 0; i < w.GetCount(); i++)
				ok = MluiSendKey((dword)w[i], 1) && ok;
			if(!ok)
				return MluiMakeJsonResponse(false, id, Value(), "Key dispatch failed");
			return MluiMakeJsonResponse(true, id, text);
		}

		dword key = (dword)MluiGetInt(p, "key", 0);
		int count = max(1, MluiGetInt(p, "count", 1));
		if(!key)
			return MluiMakeJsonResponse(false, id, Value(), "Missing key/text");
		if(!MluiSendKey(key, count))
			return MluiMakeJsonResponse(false, id, Value(), "Key dispatch failed");
		ValueMap m;
		m.Add("key", (int)key);
		m.Add("count", count);
		return MluiMakeJsonResponse(true, id, m);
	}

	if(method == "mouse") {
		String event = ToLower(MluiGetString(p, "event", "click"));
		if(event == "click" || event == "leftdown" || event == "leftup" || event == "leftdouble") {
			ValueMap p2 = p;
			p2.Add("include_hidden", MluiGetBool(p, "include_hidden", false));
			ValueMap req2;
			req2.Add("method", "click");
			req2.Add("params", p2);
			return HandleMluiJsonRequest(AsJSON(req2));
		}
		return MluiMakeJsonResponse(false, id, Value(), "Unsupported mouse event: " + event);
	}

	return MluiMakeJsonResponse(false, id, Value(), "Unknown method: " + method);
}

void StartMluiRuntime()
{
	static ::Upp::MluiRuntime s_runtime;
	if(s_runtime.IsStarted())
		return;

	bool use_stdio = false;
	bool use_tcp = false;
	String bind_host;
	int bind_port = 0;

#ifdef flagMLUI
	use_stdio = true;
#endif

	String bind_spec = GetMluiServerAddress();
	if(!bind_spec.IsEmpty()) {
		if(MluiParseBindAddress(bind_spec, bind_host, bind_port))
			use_tcp = true;
		else
			std::fprintf(stderr, "MLUI: invalid --mlui-server__ value: %s\n", bind_spec.ToStd().c_str());
	}

	if(!use_stdio && !use_tcp)
		return;

	s_runtime.Configure(use_stdio, use_tcp, bind_host, bind_port);
	s_runtime.SetJsonHandler([](const String& request_json) {
		return HandleMluiJsonRequest(request_json);
	});
	s_runtime.Start();
}

void RegisterMluiRuntimeStarter()
{
	SetMluiRuntimeStarter(StartMluiRuntime);
}

END_UPP_NAMESPACE
