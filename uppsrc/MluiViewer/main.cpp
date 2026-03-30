#include "MluiViewer.h"

using namespace Upp;

static bool ParsePositiveInt(const String& text, int& out_value)
{
	const char *end = NULL;
	int n = ScanInt(text, &end);
	if(!end || *end || n <= 0)
		return false;
	out_value = n;
	return true;
}

static String TrimCR(const String& s)
{
	if(!s.IsEmpty() && s[s.GetCount() - 1] == '\r')
		return s.Left(s.GetCount() - 1);
	return s;
}

static bool ParseRectText(String s, Rect& out_rect)
{
	s = TrimBoth(s);
	if(s.IsEmpty())
		return false;

	const char *p = s.Begin();
	const char *e = NULL;
	int left = ScanInt(p, &e);
	if(!e || *e != ',')
		return false;
	p = e + 1;
	int top = ScanInt(p, &e);
	if(!e)
		return false;
	p = e;
	while(*p == ' ' || *p == '\t')
		p++;
	int w = ScanInt(p, &e);
	if(!e || (*e != 'x' && *e != 'X'))
		return false;
	p = e + 1;
	int h = ScanInt(p, &e);
	if(!e || *e)
		return false;
	if(w <= 0 || h <= 0)
		return false;
	out_rect = RectC(left, top, w, h);
	return true;
}

static bool ResolvePathFromLeafKey(String key, String& out_path)
{
	out_path.Clear();
	key = TrimBoth(key);
	if(key.IsEmpty())
		return false;
	int q = key.Find('\n');
	if(q < 0)
		return false;
	String path = TrimBoth(key.Mid(q + 1));
	if(path.IsEmpty())
		return false;
	out_path = path;
	return true;
}

static String ExtractSemanticFromLeafKey(String key)
{
	key = TrimBoth(key);
	if(key.IsEmpty())
		return String();
	int q = key.Find('\n');
	if(q < 0)
		return key;
	return TrimBoth(key.Left(q));
}

static int GetSemanticDepth(const String& semantic_path)
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

static String GetLastSemanticNode(const String& semantic_path)
{
	String s = TrimBoth(semantic_path);
	if(s.IsEmpty())
		return String();
	int q = s.ReverseFind("->");
	if(q < 0)
		return s;
	return TrimBoth(s.Mid(q + 2));
}

static String GetLastSemanticType(const String& semantic_path)
{
	String node = GetLastSemanticNode(semantic_path);
	if(node.IsEmpty())
		return String();
	int q = node.Find(':');
	if(q < 0)
		return TrimBoth(node);
	return TrimBoth(node.Left(q));
}

static int GetSemanticTypeRank(const String& type)
{
	String t = ToLower(TrimBoth(type));
	if(t == "action")
		return 0;
	if(t == "option")
		return 1;
	if(t == "button")
		return 2;
	if(t == "bar")
		return 3;
	return 10;
}

static bool ParseMluiAddress(String s, String& host, int& port)
{
	s = TrimBoth(s);
	if(s.IsEmpty())
		return false;

	int q = s.ReverseFind(':');
	if(q >= 0) {
		host = TrimBoth(s.Left(q));
		String p = TrimBoth(s.Mid(q + 1));
		const char *end = NULL;
		int n = ScanInt(p, &end);
		if(!end || *end || n <= 0 || n > 65535)
			return false;
		port = n;
		if(host.IsEmpty())
			host = "127.0.0.1";
		return true;
	}

	host = "127.0.0.1";
	const char *end = NULL;
	int n = ScanInt(s, &end);
	if(!end || *end || n <= 0 || n > 65535)
		return false;
	port = n;
	return true;
}

static bool SendMluiRequest(const String& address, const ValueMap& req, ValueMap& out, String& error)
{
	String host;
	int port = 0;
	if(!ParseMluiAddress(address, host, port)) {
		error = "Invalid address. Use host:port";
		return false;
	}

	TcpSocket sock;
	sock.Timeout(3000);
	if(!sock.Connect(host, port)) {
		error = "Connect failed: " + sock.GetErrorDesc();
		return false;
	}

	String json = AsJSON(req);
	json.Cat('\n');
	if(!sock.PutAll(json)) {
		error = "Send failed: " + sock.GetErrorDesc();
		return false;
	}

	String resp = TrimCR(sock.GetLine(1 << 22));
	if(resp.IsEmpty()) {
		error = "Empty response";
		return false;
	}

	Value parsed = ParseJSON(resp);
	if(parsed.IsError() || !IsValueMap(parsed)) {
		error = "Invalid JSON response";
		return false;
	}

	out = parsed;
	return true;
}

void MluiLayoutView::SetItems(const Vector<Item>& items)
{
	this->items = clone(items);
	screen_rects.SetCount(items.GetCount(), Null);
	Refresh();
}

void MluiLayoutView::SetSelectedKey(const String& key)
{
	if(selected_key == key)
		return;
	selected_key = key;
	Refresh();
}

Color MluiLayoutView::GetFillColor(const String& key) const
{
	unsigned h = (unsigned)GetHashValue(key);
	int r = 80 + (h & 0x3f);
	int g = 90 + ((h >> 6) & 0x3f);
	int b = 100 + ((h >> 12) & 0x3f);
	return Color(r, g, b);
}

Rect MluiLayoutView::MapRect(const Rect& world, const Rect& bounds, double scale, Point off) const
{
	int l = off.x + fround((world.left - bounds.left) * scale);
	int t = off.y + fround((world.top - bounds.top) * scale);
	int r = off.x + fround((world.right - bounds.left) * scale);
	int b = off.y + fround((world.bottom - bounds.top) * scale);
	if(r <= l) r = l + 1;
	if(b <= t) b = t + 1;
	return Rect(l, t, r, b);
}

void MluiLayoutView::Paint(Draw& w)
{
	Rect rr = GetSize();
	w.DrawRect(rr, SColorPaper());
	screen_rects.SetCount(items.GetCount(), Null);

	Rect bounds = Null;
	Vector<int> draw_order;
	for(int i = 0; i < items.GetCount(); i++) {
		if(IsNull(items[i].rect))
			continue;
		bounds = IsNull(bounds) ? items[i].rect : bounds | items[i].rect;
		draw_order.Add(i);
	}

	if(draw_order.IsEmpty()) {
		w.DrawText(8, 8, "No layout rectangles", StdFont(), SColorText());
		return;
	}

	Sort(draw_order, [this](int a, int b) {
		int aa = max(1, items[a].rect.GetWidth()) * max(1, items[a].rect.GetHeight());
		int bb = max(1, items[b].rect.GetWidth()) * max(1, items[b].rect.GetHeight());
		return aa > bb;
	});

	int bw = max(1, bounds.GetWidth());
	int bh = max(1, bounds.GetHeight());
	int mw = max(20, rr.GetWidth() - 16);
	int mh = max(20, rr.GetHeight() - 16);
	double sx = (double)mw / (double)bw;
	double sy = (double)mh / (double)bh;
	double scale = min(sx, sy);
	if(scale <= 0)
		scale = 1.0;

	int dx = fround(bw * scale);
	int dy = fround(bh * scale);
	Point off((rr.GetWidth() - dx) / 2, (rr.GetHeight() - dy) / 2);

	for(int idx : draw_order) {
		const Item& it = items[idx];
		Rect dr = MapRect(it.rect, bounds, scale, off);
		screen_rects[idx] = dr;
		Color fill = GetFillColor(it.key);
		w.DrawRect(dr, fill);

		Color border = (it.key == selected_key) ? Color(235, 30, 30) : Black();
		w.DrawRect(dr.left, dr.top, dr.GetWidth(), 1, border);
		w.DrawRect(dr.left, dr.bottom - 1, dr.GetWidth(), 1, border);
		w.DrawRect(dr.left, dr.top, 1, dr.GetHeight(), border);
		w.DrawRect(dr.right - 1, dr.top, 1, dr.GetHeight(), border);

		String txt = it.label;
		if(txt.IsEmpty())
			txt = it.key;
		if(txt.GetCount() > 80)
			txt = txt.Left(77) + "...";
		w.DrawText(dr.left + 2, dr.top + 2, txt, Arial(9), White());
	}
}

void MluiLayoutView::LeftDown(Point p, dword flags)
{
	int best = -1;
	int best_area = INT_MAX;
	bool best_clickable = false;
	int best_depth = -1;
	int best_type_rank = INT_MAX;
	for(int i = 0; i < screen_rects.GetCount(); i++) {
		if(IsNull(screen_rects[i]) || !screen_rects[i].Contains(p))
			continue;
		bool clickable = items[i].clickable;
		int area = max(1, screen_rects[i].GetWidth()) * max(1, screen_rects[i].GetHeight());
		if(best < 0
		|| (clickable && !best_clickable)
		|| (clickable == best_clickable && area < best_area)
		|| (clickable == best_clickable && area == best_area && items[i].semantic_depth > best_depth)
		|| (clickable == best_clickable && area == best_area && items[i].semantic_depth == best_depth
		    && items[i].semantic_type_rank < best_type_rank)) {
			best_area = area;
			best_clickable = clickable;
			best_depth = items[i].semantic_depth;
			best_type_rank = items[i].semantic_type_rank;
			best = i;
		}
	}
	if(best >= 0) {
		SetSelectedKey(items[best].key);
		WhenSelectKey(items[best].key);
	}
	Ctrl::LeftDown(p, flags);
}

Vector<String> MluiViewerWindow::ParseSemanticPath(const String& semantic_path) const
{
	Vector<String> out;
	String s = TrimBoth(semantic_path);
	int pos = 0;
	while(pos < s.GetCount()) {
		int q = s.Find("->", pos);
		if(q < 0)
			q = s.GetCount();
		String part = TrimBoth(s.Mid(pos, q - pos));
		if(!part.IsEmpty())
			out.Add(part);
		pos = q + 2;
	}
	return out;
}

String MluiViewerWindow::BuildLeafKey(const String& semantic_path, const String& path) const
{
	String k = TrimBoth(semantic_path);
	String p = TrimBoth(path);
	return p.IsEmpty() ? k : k + "\n" + p;
}

bool MluiViewerWindow::ExtractLeafKey(const String& key, String& out_semantic, String& out_path) const
{
	out_semantic.Clear();
	out_path.Clear();

	String k = TrimBoth(key);
	if(k.IsEmpty())
		return false;

	int q = k.Find('\n');
	if(q < 0) {
		out_semantic = k;
		return true;
	}

	out_semantic = TrimBoth(k.Left(q));
	out_path = TrimBoth(k.Mid(q + 1));
	return !out_semantic.IsEmpty() || !out_path.IsEmpty();
}

bool MluiViewerWindow::ResolveClickTarget(const TreeArrayCtrl& tree, int id, String& out_target) const
{
	out_target.Clear();
	if(!tree.IsValid(id))
		return false;

	// Prefer explicit path column; parent semantic nodes are not clickable targets.
	String path = TrimBoth((String)tree.GetRowValue(id, 1));
	if(!path.IsEmpty()) {
		out_target = path;
		return true;
	}

	// Leaf key format is "<semantic>\\n<path>".
	if(ResolvePathFromLeafKey((String)tree.Get(id), out_target))
		return true;

	return false;
}

int MluiViewerWindow::FindChildByKey(const TreeArrayCtrl& tree, int parent_id, const String& key) const
{
	if(!tree.IsValid(parent_id))
		return -1;
	for(int i = 0; i < tree.GetChildCount(parent_id); i++) {
		int child = tree.GetChild(parent_id, i);
		if((String)tree.Get(child) == key)
			return child;
	}
	return -1;
}

int MluiViewerWindow::EnsureTreeNode(TreeArrayCtrl& tree, int parent_id, const String& key, const String& label)
{
	int id = FindChildByKey(tree, parent_id, key);
	if(id >= 0) {
		tree.Set(id, key, label);
		return id;
	}
	return tree.Add(parent_id, Null, key, label, true);
}

void MluiViewerWindow::CollectStaleNodes(const TreeArrayCtrl& tree, int id, const Index<String>& alive_keys, Vector<int>& out_remove) const
{
	if(!tree.IsValid(id))
		return;
	for(int i = 0; i < tree.GetChildCount(id); i++)
		CollectStaleNodes(tree, tree.GetChild(id, i), alive_keys, out_remove);

	if(id != 0) {
		String key = tree.Get(id);
		if(alive_keys.Find(key) < 0)
			out_remove.Add(id);
	}
}

int MluiViewerWindow::FindNodeByKey(const TreeArrayCtrl& tree, int id, const String& key) const
{
	if(!tree.IsValid(id))
		return -1;
	if((String)tree.Get(id) == key)
		return id;
	for(int i = 0; i < tree.GetChildCount(id); i++) {
		int q = FindNodeByKey(tree, tree.GetChild(id, i), key);
		if(q >= 0)
			return q;
	}
	return -1;
}

int MluiViewerWindow::FindNodeByKey(const TreeArrayCtrl& tree, const String& key) const
{
	return FindNodeByKey(tree, 0, key);
}

int MluiViewerWindow::FindBestActionableDescendant(const TreeArrayCtrl& tree, int id) const
{
	if(!tree.IsValid(id))
		return -1;

	int best_id = -1;
	int best_rank = INT_MAX;
	int best_depth = -1;

	Vector<int> stack;
	stack.Add(id);
	while(!stack.IsEmpty()) {
		int q = stack.Pop();
		if(!tree.IsValid(q))
			continue;

		String target;
		if(ResolveClickTarget(tree, q, target)) {
			String key = tree.Get(q);
			String semantic = ExtractSemanticFromLeafKey(key);
			int rank = GetSemanticTypeRank(GetLastSemanticType(semantic));
			int depth = GetSemanticDepth(semantic);
			if(best_id < 0
			|| rank < best_rank
			|| (rank == best_rank && depth > best_depth)) {
				best_id = q;
				best_rank = rank;
				best_depth = depth;
			}
		}

		for(int i = tree.GetChildCount(q) - 1; i >= 0; i--)
			stack.Add(tree.GetChild(q, i));
	}

	return best_id;
}

void MluiViewerWindow::ParseSnapshotEntries(const ValueArray& elements_json, Vector<SnapshotEntry>& out)
{
	out.Clear();
	for(int i = 0; i < elements_json.GetCount(); i++) {
		if(!IsValueMap(elements_json[i]))
			continue;
		ValueMap item = elements_json[i];

		SnapshotEntry& e = out.Add();
		e.semantic = item.Get("semantic_path", String());
		e.path = item.Get("path", String());
		e.type = item.Get("type", String());
		e.text = item.Get("text", String());
		e.visible_text = item.Get("visible_text", String());
		e.is_visible = item.Get("visible", true);
		e.ratio = item.Get("visible_text_ratio", 0.0);

		Value rect = item.Get("rect", ValueMap());
		if(IsValueMap(rect)) {
			ValueMap rr = rect;
			int l = rr.Get("left", 0);
			int t = rr.Get("top", 0);
			int w = rr.Get("width", 0);
			int h = rr.Get("height", 0);
			if(w > 0 && h > 0)
				e.rect = RectC(l, t, w, h);
			e.rect_text << l << "," << t << " " << w << "x" << h;
		}
	}
}

void MluiViewerWindow::UpdateDataTreeFromEntries(const Vector<SnapshotEntry>& entries)
{
	Index<String> alive_keys;
	alive_keys.FindAdd("__root__");

	for(const SnapshotEntry& e : entries) {
		Vector<String> parts = ParseSemanticPath(e.semantic);
		if(parts.IsEmpty()) {
			String leaf = TrimBoth(e.type);
			if(leaf.IsEmpty())
				leaf = TrimBoth(e.text);
			if(leaf.IsEmpty())
				leaf = "Element";
			parts.Add(leaf);
		}

		int parent = 0;
		String prefix;
		for(int p = 0; p < parts.GetCount(); p++) {
			if(!prefix.IsEmpty())
				prefix << " -> ";
			prefix << parts[p];
			bool is_leaf = (p + 1 == parts.GetCount());
			String node_key = is_leaf ? BuildLeafKey(prefix, e.path) : prefix;

			alive_keys.FindAdd(node_key);
			int id = EnsureTreeNode(elements, parent, node_key, parts[p]);
			if(!elements.IsOpen(id))
				elements.Open(id, true);
			if(!is_leaf) {
				elements.SetRowValue(id, 1, Null);
				elements.SetRowValue(id, 2, Null);
				elements.SetRowValue(id, 3, Null);
				elements.SetRowValue(id, 4, Null);
				elements.SetRowValue(id, 5, Null);
				elements.SetRowValue(id, 6, Null);
			}
			parent = id;
		}

		elements.SetRowValue(parent, 1, e.path);
		elements.SetRowValue(parent, 2, e.type);
		elements.SetRowValue(parent, 3, e.text);
		elements.SetRowValue(parent, 4, e.is_visible ? "true" : "false");
		elements.SetRowValue(parent, 5, Format("%.2f", e.ratio));
		elements.SetRowValue(parent, 6, e.rect_text);
	}

	Vector<int> stale;
	CollectStaleNodes(elements, 0, alive_keys, stale);
	for(int i = 0; i < stale.GetCount(); i++)
		elements.Remove(stale[i]);
}

void MluiViewerWindow::UpdateLayoutViewFromEntries(const Vector<SnapshotEntry>& entries)
{
	Vector<MluiLayoutView::Item> items;
	for(const SnapshotEntry& e : entries) {
		if(!e.is_visible)
			continue;
		if(IsNull(e.rect))
			continue;
		MluiLayoutView::Item& it = items.Add();
		it.key = BuildLeafKey(e.semantic, e.path);
		Vector<String> parts = ParseSemanticPath(e.semantic);
		it.label = parts.IsEmpty() ? e.type : parts.Top();
		if(it.label.IsEmpty())
			it.label = e.path;
		if(it.label.IsEmpty())
			it.label = "node";
		it.rect = e.rect;
		it.semantic_depth = GetSemanticDepth(e.semantic);
		it.semantic_type_rank = GetSemanticTypeRank(GetLastSemanticType(e.semantic));
		it.clickable = !TrimBoth(e.path).IsEmpty();
	}
	layout_view.SetItems(items);
}

void MluiViewerWindow::RequestSnapshot(bool show_errors)
{
	if(snapshot_request_running) {
		snapshot_request_pending = true;
		snapshot_pending_show_errors = snapshot_pending_show_errors || show_errors;
		return;
	}

	snapshot_request_running = true;
	String addr = ~address;
	int req_id = request_id++;
	Ptr<MluiViewerWindow> self = this;

	Thread::Start([self, addr, req_id, show_errors]() {
		SnapshotJobResult* job = new SnapshotJobResult;
		job->show_errors = show_errors;

		auto post_result = [self, job]() {
			UPP::PostCallback([self, job]() {
				if(self)
					self->OnSnapshotResult(job);
				else
					delete job;
			});
		};

		ValueMap req;
		req.Add("id", req_id);
		req.Add("method", "snapshot");

		ValueMap resp;
		String error;
		if(!SendMluiRequest(addr, req, resp, error)) {
			job->error = error;
			post_result();
			return;
		}

		job->transport_ok = true;
		if(!(bool)resp.Get("ok", false)) {
			job->error = (String)resp.Get("error", "Request failed");
			post_result();
			return;
		}

		job->rpc_ok = true;
		job->result = resp.Get("result", ValueMap());
		post_result();
	});
}

void MluiViewerWindow::OnSnapshotResult(SnapshotJobResult* job)
{
	One<SnapshotJobResult> owned(job);
	snapshot_request_running = false;

	if(!job->transport_ok || !job->rpc_ok) {
		if(job->show_errors)
			Exclamation(job->error);
	}
	else {
		raw_json.Set(AsJSON(job->result, true));

		if(IsValueMap(job->result)) {
			ValueMap r = job->result;
			Value elem_value = r.Get("elements", ValueArray());
			if(IsValueArray(elem_value)) {
				Vector<SnapshotEntry> entries;
				ParseSnapshotEntries(elem_value, entries);
				UpdateDataTreeFromEntries(entries);
				UpdateLayoutViewFromEntries(entries);
				OnElementsCursor();
			}
		}
	}

	if(snapshot_request_pending) {
		bool show_errors = snapshot_pending_show_errors;
		snapshot_request_pending = false;
		snapshot_pending_show_errors = false;
		RequestSnapshot(show_errors);
	}
}

void MluiViewerWindow::RefreshSnapshot()
{
	RequestSnapshot(true);
}

void MluiViewerWindow::RefreshSnapshotSilent()
{
	RequestSnapshot(false);
}

void MluiViewerWindow::AutoRefreshTick()
{
	if(!auto_refresh_enabled)
		return;
	RefreshSnapshotSilent();
	auto_refresh_cb.KillSet(auto_refresh_ms, THISBACK(AutoRefreshTick));
}

void MluiViewerWindow::SetAddress(const String& addr)
{
	address.SetData(addr);
}

void MluiViewerWindow::EnableAutoRefresh(int period_ms)
{
	auto_refresh_enabled = true;
	auto_refresh_ms = max(100, period_ms);
	auto_refresh_cb.KillSet(auto_refresh_ms, THISBACK(AutoRefreshTick));
}

void MluiViewerWindow::ClickSelected()
{
	int id = elements.GetCursor();

	ValueMap req;
	req.Add("id", request_id++);
	req.Add("method", "click");
	ValueMap params;
	// Viewer should be able to trigger hidden entries (e.g. closed menu branches).
	params.Add("include_hidden", true);
	String target;
	if(!ResolveClickTarget(elements, id, target)) {
		int best = FindBestActionableDescendant(elements, id);
		if(best >= 0 && ResolveClickTarget(elements, best, target)) {
			if(elements.GetCursor() != best) {
				elements.SetCursor(best);
				elements.MakeVisible(best);
				OnElementsCursor();
			}
		}
		else {
			String selected_layout_key = layout_view.GetSelectedKey();
			if(!selected_layout_key.IsEmpty() && ResolvePathFromLeafKey(selected_layout_key, target)) {
				int node_id = FindNodeByKey(elements, selected_layout_key);
				if(node_id >= 0 && elements.GetCursor() != node_id) {
					elements.SetCursor(node_id);
					elements.MakeVisible(node_id);
					OnElementsCursor();
				}
			}
			else if(!selected_layout_key.IsEmpty()) {
				int node_id = FindNodeByKey(elements, selected_layout_key);
				int node_best = FindBestActionableDescendant(elements, node_id);
				if(node_best >= 0 && ResolveClickTarget(elements, node_best, target)) {
					elements.SetCursor(node_best);
					elements.MakeVisible(node_best);
					OnElementsCursor();
				}
				else {
					Exclamation("Select a clickable row first");
					return;
				}
			}
			else {
				Exclamation("Select a clickable row first");
				return;
			}
		}
	}
	else {
		params.Add("path", target);
	}
	params.Set("path", target);
	req.Add("params", params);

	ValueMap resp;
	String error;
	if(!SendMluiRequest(~address, req, resp, error)) {
		Exclamation(error);
		return;
	}

	if(!(bool)resp.Get("ok", false)) {
		Exclamation((String)resp.Get("error", "Click failed"));
		return;
	}

	RefreshSnapshot();
}

void MluiViewerWindow::SendInput()
{
	String value = TrimBoth(~input);
	if(value.IsEmpty())
		return;

	ValueMap req;
	req.Add("id", request_id++);
	req.Add("method", "key");

	ValueMap params;
	const char *end = NULL;
	int n = ScanInt(value, &end);
	if(end && *end == 0)
		params.Add("key", n);
	else
		params.Add("text", value);
	req.Add("params", params);

	ValueMap resp;
	String error;
	if(!SendMluiRequest(~address, req, resp, error)) {
		Exclamation(error);
		return;
	}

	if(!(bool)resp.Get("ok", false)) {
		Exclamation((String)resp.Get("error", "Input failed"));
		return;
	}

	RefreshSnapshot();
}

void MluiViewerWindow::InitTree()
{
	elements.Header();
	elements.HeaderTab(0).SetText("Node");
	elements.AddColumn("Path", 260);
	elements.AddColumn("Type", 80);
	elements.AddColumn("Text", 120);
	elements.AddColumn("Visible", 120);
	elements.AddColumn("Ratio", 60);
	elements.AddColumn("Rect", 120);
	elements.ColumnWidths("300 260 80 120 120 60 120");
	elements.NoRoot();
	elements.MultiSelect(false);
	elements.SetLineCy(22);
	elements.SetRoot(Null, "__root__", "MLUI");
	elements.Open(0, true);
}

void MluiViewerWindow::OnElementsCursor()
{
	int id = elements.GetCursor();
	if(id <= 0) {
		layout_view.SetSelectedKey(String());
		return;
	}
	String key = elements.Get(id);
	if(!key.IsEmpty())
		layout_view.SetSelectedKey(key);
}

void MluiViewerWindow::OnLayoutViewSelect(String key)
{
	int id = FindNodeByKey(elements, key);
	if(id >= 0) {
		elements.SetCursor(id);
		elements.MakeVisible(id);
		OnElementsCursor();
	}
	else {
		layout_view.SetSelectedKey(key);
	}
}

void MluiViewerWindow::LayoutControls()
{
	address_lbl.SetLabel("MLUI");
	input_lbl.SetLabel("Input");
	refresh.SetLabel("Refresh");
	click_selected.SetLabel("Click Selected");
	send_input.SetLabel("Send");

	address.SetData("127.0.0.1:8082");
	InitTree();

	elements.WhenLeftDouble = THISBACK(ClickSelected);
	elements.WhenCursor = THISBACK(OnElementsCursor);
	layout_view.WhenSelectKey = THISBACK(OnLayoutViewSelect);

	raw_json.SetReadOnly();
	raw_json.SetFont(Courier(13));

	right_layout_tab.Add(layout_view.SizePos());
	right_json_tab.Add(raw_json.SizePos());
	right_tabs.Add(right_layout_tab.SizePos(), "Layout");
	right_tabs.Add(right_json_tab.SizePos(), "JSON");
	right_tabs.Set(0);

	split.Horz(elements, right_tabs);
	split.SetPos(4700);

	Add(address_lbl.LeftPosZ(6, 36).TopPosZ(8, 20));
	Add(address.LeftPosZ(44, 220).TopPosZ(6, 24));
	Add(refresh.LeftPosZ(270, 70).TopPosZ(6, 24));
	Add(click_selected.LeftPosZ(346, 110).TopPosZ(6, 24));
	Add(input_lbl.LeftPosZ(462, 40).TopPosZ(8, 20));
	Add(input.LeftPosZ(504, 180).TopPosZ(6, 24));
	Add(send_input.LeftPosZ(690, 58).TopPosZ(6, 24));
	Add(split.HSizePosZ(6, 6).VSizePosZ(36, 6));

	refresh << THISBACK(RefreshSnapshot);
	click_selected << THISBACK(ClickSelected);
	send_input << THISBACK(SendInput);
}

MluiViewerWindow::MluiViewerWindow()
{
	Title("MLUI Viewer");
	Sizeable().Zoomable();
	SetRect(0, 0, 1300, 820);
	LayoutControls();
}

GUI_APP_MAIN
{
	MluiViewerWindow app;

	String address;
	bool auto_refresh = false;
	int auto_refresh_ms = 1000;

	const Vector<String>& cmd = CommandLine();
	for(int i = 0; i < cmd.GetCount(); i++) {
		String a = cmd[i];
		if(a == "--mlui" && i + 1 < cmd.GetCount()) {
			address = cmd[++i];
			continue;
		}
		if(a.StartsWith("--mlui=")) {
			address = a.Mid(7);
			continue;
		}
		if(a == "--auto-refresh") {
			auto_refresh = true;
			continue;
		}
		if(a.StartsWith("--auto-refresh=")) {
			auto_refresh = true;
			int parsed_ms = 0;
			String v = a.Mid(15);
			if(ParsePositiveInt(v, parsed_ms))
				auto_refresh_ms = parsed_ms;
			continue;
		}
		if(a == "--auto-refresh-ms" && i + 1 < cmd.GetCount()) {
			int parsed_ms = 0;
			if(ParsePositiveInt(cmd[i + 1], parsed_ms))
				auto_refresh_ms = parsed_ms;
			i++;
			continue;
		}
		if(a.StartsWith("--auto-refresh-ms=")) {
			int parsed_ms = 0;
			String v = a.Mid(18);
			if(ParsePositiveInt(v, parsed_ms))
				auto_refresh_ms = parsed_ms;
			continue;
		}
	}

	if(!address.IsEmpty())
		app.SetAddress(address);
	if(auto_refresh)
		app.EnableAutoRefresh(auto_refresh_ms);

	app.Run();
}
