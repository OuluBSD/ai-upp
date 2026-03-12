#include <CtrlLib/CtrlLib.h>

using namespace Upp;

struct ToolBarApp : TopWindow {
	struct GroupHandle;

	struct DemoToolBar : ToolBar {
		Callback1<Point> WhenEmptyRightDown;

		void ChildMouseEvent(Ctrl *child, int event, Point p, int zdelta, dword keyflags) override
		{
			ToolBar::ChildMouseEvent(child, event, p, zdelta, keyflags);
			if(event != RIGHTDOWN || !WhenEmptyRightDown)
				return;
			Point tp = p + child->GetRect().TopLeft();
			Ctrl *q = Ctrl::GetVisibleChild(this, tp, false);
			if(dynamic_cast<ToolButton *>(q))
				return;
			WhenEmptyRightDown(tp);
		}
	};

	struct GroupHandle : Ctrl {
		int          group = -1;
		bool         hidden = false;
		Callback1<int> WhenStartDrag;

		Size GetMinSize() const override
		{
			return hidden ? Size(0, 0) : Size(HorzLayoutZoom(10), VertLayoutZoom(20));
		}

		void Paint(Draw& w) override
		{
			if(hidden)
				return;
			Size sz = GetSize();
			w.DrawRect(sz, Blend(SColorFace(), SColorPaper(), 200));
			Color c = Blend(SColorShadow(), SColorText(), 120);
			int cx = sz.cx / 2;
			for(int y = 4; y < sz.cy - 3; y += 4) {
				w.DrawRect(cx - 2, y, 1, 1, c);
				w.DrawRect(cx + 1, y, 1, 1, c);
			}
		}

		void LeftDown(Point p, dword) override
		{
			if(hidden)
				return;
			drag_origin = p;
			SetCapture();
		}

		void MouseMove(Point p, dword) override
		{
			if(!HasCapture() || hidden)
				return;
			if(abs(p.x - drag_origin.x) + abs(p.y - drag_origin.y) > HorzLayoutZoom(3)) {
				ReleaseCapture();
				WhenStartDrag(group);
			}
		}

		void LeftUp(Point, dword) override
		{
			if(HasCapture())
				ReleaseCapture();
		}

		Point drag_origin;

		GroupHandle()
		{
			NoWantFocus();
		}
	};

	struct Group {
		String      name;
		Vector<String> actions;
		bool        right = false;
		bool        show_handle = true;
		GroupHandle handle;
	};

	DemoToolBar  toolbar;
	StatusBar    status;
	Option       hide_handles;
	Option       right_search;
	Array<Group> group;
	Vector<int>  left_order;
	Vector<int>  right_order;
	Vector<int>  drag_left_backup;
	Vector<int>  drag_right_backup;
	int          dragging_group = -1;

	typedef ToolBarApp CLASSNAME;

	struct LayoutState {
		Vector<int> left_order;
		Vector<int> right_order;
		bool        hide_handles = false;
		bool        right_search = true;

		void Serialize(Stream& s)
		{
			int version = 1;
			s / version;
			s % left_order % right_order % hide_handles % right_search;
		}

		void Jsonize(JsonIO& jio)
		{
			jio("left_order", left_order)
			   ("right_order", right_order)
			   ("hide_handles", hide_handles)
			   ("right_search", right_search);
		}
	};

	int AddGroup(const char *name, std::initializer_list<const char *> actions, bool right, bool show_handle)
	{
		Group& g = group.Add();
		g.name = name;
		g.right = right;
		g.show_handle = show_handle;
		for(const char *s : actions)
			g.actions.Add(s);
		return group.GetCount() - 1;
	}

	void BuildToolBar(Bar& bar)
	{
		auto add_group = [&](int id, bool right, bool first_on_right, bool first_on_left) {
			Group& g = group[id];
			bool show = g.show_handle && !hide_handles.Get();
			if(right)
				bar.ToolGroup(first_on_right, show);
			else if(!first_on_left)
				bar.ToolGroup(false, show);
			g.handle.group = id;
			g.handle.hidden = !show;
			if(show)
				bar.Add(g.handle, g.handle.GetMinSize());
			for(const String& action : g.actions) {
				String text = g.name + "/" + action;
				bar.Add(text, Null, [=] {
					status = "Action: " + text;
				});
			}
		};

		bool first_left = true;
		for(int id : left_order) {
			add_group(id, false, false, first_left);
			first_left = false;
		}

		bool first_right = true;
		for(int id : right_order) {
			add_group(id, true, first_right, false);
			first_right = false;
		}
	}

	void RebuildOrder()
	{
		left_order.Clear();
		right_order.Clear();
		for(int i = 0; i < group.GetCount(); i++) {
			if(group[i].right)
				right_order.Add(i);
			else
				left_order.Add(i);
		}
	}

	static void RemoveGroup(Vector<int>& order, int id)
	{
		int q = -1;
		for(int i = 0; i < order.GetCount(); i++)
			if(order[i] == id) {
				q = i;
				break;
			}
		if(q >= 0)
			order.Remove(q);
	}

	static Vector<int> CloneVec(const Vector<int>& src)
	{
		Vector<int> dst;
		for(int x : src)
			dst.Add(x);
		return dst;
	}

	LayoutState CaptureLayoutState() const
	{
		LayoutState st;
		st.left_order = CloneVec(left_order);
		st.right_order = CloneVec(right_order);
		st.hide_handles = hide_handles.Get();
		st.right_search = right_search.Get();
		return st;
	}

	void ApplyLayoutState(const LayoutState& st)
	{
		Vector<int> used;
		used.SetCount(group.GetCount(), 0);
		left_order.Clear();
		right_order.Clear();
		for(int id : st.left_order)
			if(id >= 0 && id < group.GetCount() && !used[id]) {
				used[id] = 1;
				group[id].right = false;
				left_order.Add(id);
			}
		for(int id : st.right_order)
			if(id >= 0 && id < group.GetCount() && !used[id]) {
				used[id] = 1;
				group[id].right = true;
				right_order.Add(id);
			}
		for(int id = 0; id < group.GetCount(); id++)
			if(!used[id]) {
				if(group[id].right)
					right_order.Add(id);
				else
					left_order.Add(id);
			}
		hide_handles = st.hide_handles;
		right_search = st.right_search;
	}

	bool SaveLayoutJson()
	{
		return StoreAsJsonFile(CaptureLayoutState(), GetDataFile("toolbar_layout.json"), true);
	}

	bool LoadLayoutJson()
	{
		LayoutState st;
		if(!LoadFromJsonFile(st, GetDataFile("toolbar_layout.json")))
			return false;
		ApplyLayoutState(st);
		return true;
	}

	bool SaveLayoutBin()
	{
		FileOut out(GetDataFile("toolbar_layout.bin"));
		if(!out.IsOpen())
			return false;
		LayoutState st = CaptureLayoutState();
		st.Serialize(out);
		return !out.IsError();
	}

	bool LoadLayoutBin()
	{
		FileIn in(GetDataFile("toolbar_layout.bin"));
		if(!in.IsOpen())
			return false;
		LayoutState st;
		st.Serialize(in);
		if(in.IsError())
			return false;
		ApplyLayoutState(st);
		return true;
	}

	int FindInsertPos(const Vector<int>& order, int mouse_screen_x, int skip_id) const
	{
		for(int i = 0; i < order.GetCount(); i++) {
			int id = order[i];
			if(id == skip_id)
				continue;
			Rect r = group[id].handle.GetScreenRect();
			if(!r.IsEmpty() && mouse_screen_x < r.CenterPoint().x)
				return i;
		}
		return order.GetCount();
	}

	void StartGroupDrag(int id)
	{
		if(id < 0 || id >= group.GetCount())
			return;
		dragging_group = id;
		drag_left_backup = CloneVec(left_order);
		drag_right_backup = CloneVec(right_order);
		status = "Dragging group: " + group[id].name;
		int q = group[id].handle.DoDragAndDrop(InternalClip(group[id].handle, "toolbar-group"),
		                                       Null, DND_MOVE);
		if(q != DND_MOVE) {
			left_order = CloneVec(drag_left_backup);
			right_order = CloneVec(drag_right_backup);
			toolbar.Set(THISBACK(BuildToolBar));
			status = "Drag canceled";
		}
		dragging_group = -1;
	}

	bool MoveGroupTo(int id, bool to_right, int pos)
	{
		Vector<int> old_left = CloneVec(left_order);
		Vector<int> old_right = CloneVec(right_order);
		RemoveGroup(left_order, id);
		RemoveGroup(right_order, id);
		Vector<int>& target = to_right ? right_order : left_order;
		pos = minmax(pos, 0, target.GetCount());
		target.Insert(pos, id);
		bool changed = old_left != left_order || old_right != right_order;
		group[id].right = to_right;
		return changed;
	}

	void HandleGroupDrop(Point p, PasteClip& d)
	{
		if(!AcceptInternal<GroupHandle>(d, "toolbar-group"))
			return;
		const GroupHandle& src = GetInternal<GroupHandle>(d);
		int id = src.group;
		if(id < 0 || id >= group.GetCount())
			return;

		Point ps = p + GetScreenRect().TopLeft();
		Rect tr = toolbar.GetScreenRect();
		if(!tr.Contains(ps))
			return;

		d.SetAction(DND_MOVE);

		bool to_right = ps.x - tr.left >= tr.GetWidth() / 2;
		Vector<int>& target = to_right ? right_order : left_order;
		int pos = FindInsertPos(target, ps.x, id);
		if(MoveGroupTo(id, to_right, pos)) {
			toolbar.Set(THISBACK(BuildToolBar));
			if(d.IsPaste())
				status = Format("Moved group '%s' to %s side at position %d",
				                group[id].name, to_right ? "right" : "left", pos);
			else
				status = Format("Preview: '%s' -> %s @%d",
				                group[id].name, to_right ? "right" : "left", pos);
		}
	}

	void DragAndDrop(Point p, PasteClip& d) override
	{
		HandleGroupDrop(p, d);
	}

	void FrameDragAndDrop(Point p, PasteClip& d) override
	{
		HandleGroupDrop(p, d);
	}

	void ToolbarContextMenu(Point p)
	{
		MenuBar::Execute(&toolbar, [=](Bar& bar) {
			bar.Add("Lock Group Handles", [=] {
				hide_handles = !hide_handles.Get();
				ToggleHandles();
			}).Check(hide_handles.Get());
			bar.Separator();
			bar.Add("Save Layout (JSON)", [=] { status = SaveLayoutJson() ? "Saved JSON layout" : "JSON save failed"; });
			bar.Add("Load Layout (JSON)", [=] {
				if(LoadLayoutJson()) {
					toolbar.Set(THISBACK(BuildToolBar));
					status = "Loaded JSON layout";
				}
				else
					status = "JSON load failed";
			});
			bar.Add("Save Layout (Binary)", [=] { status = SaveLayoutBin() ? "Saved binary layout" : "Binary save failed"; });
			bar.Add("Load Layout (Binary)", [=] {
				if(LoadLayoutBin()) {
					toolbar.Set(THISBACK(BuildToolBar));
					status = "Loaded binary layout";
				}
				else
					status = "Binary load failed";
			});
		}, toolbar.GetScreenRect().TopLeft() + p);
	}

	void ToggleHandles()
	{
		toolbar.Set(THISBACK(BuildToolBar));
		status = hide_handles.Get() ? "Handle visuals hidden" : "Handle visuals shown";
	}

	void ToggleSearchSide()
	{
		int id = 2; // Search group
		group[id].right = right_search;
		RemoveGroup(left_order, id);
		RemoveGroup(right_order, id);
		if(group[id].right)
			right_order.Insert(0, id);
		else
			left_order.Add(id);
		toolbar.Set(THISBACK(BuildToolBar));
		status = "Search group moved by option";
	}

	ToolBarApp()
	{
		Title("ToolBar: grouped, right aligned, draggable");
		Sizeable().Zoomable();

		AddGroup("File", { "New", "Open", "Save" }, false, true);
		AddGroup("Edit", { "Undo", "Redo", "Find" }, false, true);
		AddGroup("Search", { "Find", "Next", "Prev" }, true, true);
		AddGroup("Run", { "Build", "Run" }, true, true);
		AddGroup("Window", { "Split", "Focus" }, false, true);

		for(int i = 0; i < group.GetCount(); i++)
			group[i].handle.WhenStartDrag = THISBACK(StartGroupDrag);

		RebuildOrder();
		LoadLayoutJson() || LoadLayoutBin();

		AddFrame(toolbar);
		AddFrame(status);
		toolbar.WhenEmptyRightDown = THISBACK(ToolbarContextMenu);
		toolbar.ButtonKind(ToolButton::RIGHTLABEL);
		toolbar.Set(THISBACK(BuildToolBar));

		Add(hide_handles.SetLabel("Hide handle visuals")
		      .LeftPosZ(8, 180).TopPosZ(10, 20));
		hide_handles.WhenAction = THISBACK(ToggleHandles);

		Add(right_search.SetLabel("Search group on right")
		      .LeftPosZ(200, 180).TopPosZ(10, 20));
		right_search = true;
		right_search.WhenAction = THISBACK(ToggleSearchSide);

		status = "Drag a handle to reorder groups. Drop left/right to move side.";
	}

	void Close() override
	{
		SaveLayoutJson();
		SaveLayoutBin();
		TopWindow::Close();
	}
};

GUI_APP_MAIN
{
	ToolBarApp().Run();
}
