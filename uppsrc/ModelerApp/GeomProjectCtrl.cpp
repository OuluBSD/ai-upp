#include "ModelerApp.h"

NAMESPACE_UPP

namespace {

struct IconToggle : Ctrl {
	bool value = false;
	
	virtual void Paint(Draw& w) {
		Size sz = GetSize();
		Color bg = SColorPaper();
		w.DrawRect(sz, bg);
		DrawIcon(w, Rect(sz), value, IsEnabled() ? SColorText() : SColorDisabled());
	}
	
	virtual void LeftDown(Point, dword) {
		if (!IsEnabled())
			return;
		value = !value;
		Refresh();
		WhenAction();
	}
	
	virtual void SetData(const Value& v) {
		value = !IsNull(v) && (bool)v;
		Refresh();
	}
	
	virtual Value GetData() const {
		return value;
	}
	
	virtual Size GetMinSize() const {
		return Size(16, 16);
	}
	
protected:
	virtual void DrawIcon(Draw& w, const Rect& r, bool on, Color c) = 0;
};

struct EyeToggle : IconToggle {
	virtual void DrawIcon(Draw& w, const Rect& r, bool on, Color c) {
		Rect rr = r.Deflated(2, 4);
		if (rr.Width() <= 2 || rr.Height() <= 2)
			return;
		w.DrawEllipse(rr, Null, 1, c);
		if (on) {
			Rect pupil = rr.CenterRect(Size(4, 4));
			w.DrawEllipse(pupil, c);
		}
		else {
			w.DrawLine(rr.left, rr.bottom, rr.right, rr.top, 1, c);
		}
	}
};

struct LockToggle : IconToggle {
	virtual void DrawIcon(Draw& w, const Rect& r, bool on, Color c) {
		Rect body = r.Deflated(4, 5);
		if (body.Width() <= 2 || body.Height() <= 2)
			return;
		w.DrawRect(body, c);
		Rect shackle = RectC(body.left + body.Width() / 4, r.top + 2, body.Width() / 2, body.Height() / 2);
		w.DrawEllipse(shackle, Null, 1, c);
		if (!on)
			w.DrawLine(body.left, body.top, body.right, body.bottom, 1, SColorPaper());
	}
};

struct TextToggle : IconToggle {
	String label;
	TextToggle(const char* text = "") { label = text; }
	virtual void DrawIcon(Draw& w, const Rect& r, bool on, Color c) {
		Font fnt = StdFont().Bold();
		Color ink = on ? c : Blend(c, SColorPaper(), 120);
		Size tsz = GetTextSize(label, fnt);
		Point pt(r.left + (r.Width() - tsz.cx) / 2, r.top + (r.Height() - tsz.cy) / 2);
		w.DrawText(pt.x, pt.y, label, fnt, ink);
	}
};

struct DragValueEdit : EditDoubleSpin {
	double drag_start = 0;
	double drag_inc = 0.01;
	Point  drag_pt = Point(0, 0);
	bool   dragging = false;
	
	DragValueEdit() { ShowSpin(false); }
	
	void SetDragInc(double v) { drag_inc = v; }
	double GetDragInc() const { return drag_inc; }
	
	virtual void LeftDown(Point p, dword keyflags) {
		EditDoubleSpin::LeftDown(p, keyflags);
		if (!IsEditable())
			return;
		drag_pt = p;
		drag_start = IsNull(GetData()) ? 0.0 : (double)GetData();
		dragging = false;
		SetCapture();
	}
	
	virtual void LeftDrag(Point p, dword keyflags) {
		if (!IsEditable())
			return;
		if (!HasCapture())
			return;
		if (!dragging) {
			if (abs(p.x - drag_pt.x) + abs(p.y - drag_pt.y) < 3)
				return;
			dragging = true;
		}
		double delta = (double)(p.x - drag_pt.x);
		SetData(drag_start + delta * drag_inc);
		WhenAction();
	}
	
	virtual void LeftUp(Point p, dword keyflags) {
		if (HasCapture())
			ReleaseCapture();
		EditDoubleSpin::LeftUp(p, keyflags);
	}
};

struct Vec3EditCtrl : Ctrl {
	Label lx, ly, lz;
	DragValueEdit ex, ey, ez;
	DragValueEdit* drag_edit = 0;
	double drag_start = 0;
	double drag_inc = 0.01;
	Point drag_pt = Point(0, 0);
	bool dragging = false;
	
	Vec3EditCtrl() {
		lx.SetText("X");
		ly.SetText("Y");
		lz.SetText("Z");
		Add(lx);
		Add(ly);
		Add(lz);
		Add(ex);
		Add(ey);
		Add(ez);
		ex.SetDragInc(0.01);
		ey.SetDragInc(0.01);
		ez.SetDragInc(0.01);
		ex.WhenAction = ey.WhenAction = ez.WhenAction = [=] { WhenAction(); };
	}
	
	void SetValue(const vec3& v) {
		ex.SetData(v[0]);
		ey.SetData(v[1]);
		ez.SetData(v[2]);
	}
	
	vec3 GetValue() const {
		vec3 v;
		v[0] = (double)ex.GetData();
		v[1] = (double)ey.GetData();
		v[2] = (double)ez.GetData();
		return v;
	}
	
	void SetEditable(bool b) {
		ex.SetEditable(b);
		ey.SetEditable(b);
		ez.SetEditable(b);
	}
	
	virtual void LeftDown(Point p, dword keyflags) {
		if (lx.GetRect().Contains(p)) drag_edit = &ex;
		else if (ly.GetRect().Contains(p)) drag_edit = &ey;
		else if (lz.GetRect().Contains(p)) drag_edit = &ez;
		else drag_edit = 0;
		if (drag_edit && drag_edit->IsEditable()) {
			drag_pt = p;
			drag_start = IsNull(drag_edit->GetData()) ? 0.0 : (double)drag_edit->GetData();
			drag_inc = drag_edit->GetDragInc();
			dragging = false;
			SetCapture();
		}
		Ctrl::LeftDown(p, keyflags);
	}
	
	virtual void LeftDrag(Point p, dword keyflags) {
		if (!HasCapture() || !drag_edit || !drag_edit->IsEditable())
			return Ctrl::LeftDrag(p, keyflags);
		if (!dragging) {
			if (abs(p.x - drag_pt.x) + abs(p.y - drag_pt.y) < 3)
				return;
			dragging = true;
		}
		double delta = (double)(p.x - drag_pt.x);
		drag_edit->SetData(drag_start + delta * drag_inc);
		WhenAction();
	}
	
	virtual void LeftUp(Point p, dword keyflags) {
		if (HasCapture())
			ReleaseCapture();
		Ctrl::LeftUp(p, keyflags);
	}
	
	virtual void Layout() {
		Rect r = GetSize();
		int labelw = 12;
		int gap = 4;
		int fields = 3;
		int total_gap = gap * (fields - 1);
		int cellw = (r.Width() - fields * labelw - total_gap) / fields;
		int x = r.left;
		lx.SetRect(x, r.top, labelw, r.Height()); x += labelw;
		ex.SetRect(x, r.top, cellw, r.Height()); x += cellw + gap;
		ly.SetRect(x, r.top, labelw, r.Height()); x += labelw;
		ey.SetRect(x, r.top, cellw, r.Height()); x += cellw + gap;
		lz.SetRect(x, r.top, labelw, r.Height()); x += labelw;
		ez.SetRect(x, r.top, cellw, r.Height());
	}
};

struct QuatEditCtrl : Ctrl {
	Label lx, ly, lz, lw;
	DragValueEdit ex, ey, ez, ew;
	DragValueEdit* drag_edit = 0;
	double drag_start = 0;
	double drag_inc = 0.001;
	Point drag_pt = Point(0, 0);
	bool dragging = false;
	
	QuatEditCtrl() {
		lx.SetText("X");
		ly.SetText("Y");
		lz.SetText("Z");
		lw.SetText("W");
		Add(lx);
		Add(ly);
		Add(lz);
		Add(lw);
		Add(ex);
		Add(ey);
		Add(ez);
		Add(ew);
		ex.SetDragInc(0.001);
		ey.SetDragInc(0.001);
		ez.SetDragInc(0.001);
		ew.SetDragInc(0.001);
		ex.WhenAction = ey.WhenAction = ez.WhenAction = ew.WhenAction = [=] { WhenAction(); };
	}
	
	void SetValue(const quat& q) {
		ex.SetData(q[0]);
		ey.SetData(q[1]);
		ez.SetData(q[2]);
		ew.SetData(q[3]);
	}
	
	quat GetValue() const {
		quat q;
		q[0] = (double)ex.GetData();
		q[1] = (double)ey.GetData();
		q[2] = (double)ez.GetData();
		q[3] = (double)ew.GetData();
		return q;
	}
	
	void SetEditable(bool b) {
		ex.SetEditable(b);
		ey.SetEditable(b);
		ez.SetEditable(b);
		ew.SetEditable(b);
	}
	
	virtual void LeftDown(Point p, dword keyflags) {
		if (lx.GetRect().Contains(p)) drag_edit = &ex;
		else if (ly.GetRect().Contains(p)) drag_edit = &ey;
		else if (lz.GetRect().Contains(p)) drag_edit = &ez;
		else if (lw.GetRect().Contains(p)) drag_edit = &ew;
		else drag_edit = 0;
		if (drag_edit && drag_edit->IsEditable()) {
			drag_pt = p;
			drag_start = IsNull(drag_edit->GetData()) ? 0.0 : (double)drag_edit->GetData();
			drag_inc = drag_edit->GetDragInc();
			dragging = false;
			SetCapture();
		}
		Ctrl::LeftDown(p, keyflags);
	}
	
	virtual void LeftDrag(Point p, dword keyflags) {
		if (!HasCapture() || !drag_edit || !drag_edit->IsEditable())
			return Ctrl::LeftDrag(p, keyflags);
		if (!dragging) {
			if (abs(p.x - drag_pt.x) + abs(p.y - drag_pt.y) < 3)
				return;
			dragging = true;
		}
		double delta = (double)(p.x - drag_pt.x);
		drag_edit->SetData(drag_start + delta * drag_inc);
		WhenAction();
	}
	
	virtual void LeftUp(Point p, dword keyflags) {
		if (HasCapture())
			ReleaseCapture();
		Ctrl::LeftUp(p, keyflags);
	}
	
	virtual void Layout() {
		Rect r = GetSize();
		int labelw = 12;
		int gap = 4;
		int fields = 4;
		int total_gap = gap * (fields - 1);
		int cellw = (r.Width() - fields * labelw - total_gap) / fields;
		int x = r.left;
		lx.SetRect(x, r.top, labelw, r.Height()); x += labelw;
		ex.SetRect(x, r.top, cellw, r.Height()); x += cellw + gap;
		ly.SetRect(x, r.top, labelw, r.Height()); x += labelw;
		ey.SetRect(x, r.top, cellw, r.Height()); x += cellw + gap;
		lz.SetRect(x, r.top, labelw, r.Height()); x += labelw;
		ez.SetRect(x, r.top, cellw, r.Height()); x += cellw + gap;
		lw.SetRect(x, r.top, labelw, r.Height()); x += labelw;
		ew.SetRect(x, r.top, cellw, r.Height());
	}
};

struct ToggleRowCtrl : Ctrl {
	EyeToggle eye;
	LockToggle lock;
	Event<bool> WhenVisible;
	Event<bool> WhenLocked;
	
	ToggleRowCtrl() {
		Add(eye);
		Add(lock);
		eye.WhenAction << [=] { WhenVisible((bool)eye.GetData()); };
		lock.WhenAction << [=] { WhenLocked((bool)lock.GetData()); };
	}
	
	void SetVisible(bool v) { eye.SetData(v); }
	void SetLocked(bool v)  { lock.SetData(v); }
	
	virtual void Layout() {
		Rect r = GetSize();
		int w = r.Height();
		eye.SetRect(r.left, r.top, w, r.Height());
		lock.SetRect(r.left + w + 4, r.top, w, r.Height());
	}
};

}


GeomProjectCtrl::GeomProjectCtrl(Edit3D* e) {
	this->e = e;
	
	time.WhenCursor << THISBACK(OnCursor);
	tree.WhenCursor << THISBACK(TreeSelect);
	tree.WhenBar = THISBACK(TreeMenu);
	props.WhenBar = THISBACK(PropsMenu);
	
	hsplit.Horz().SetPos(2000) << metasplit << vsplit,
	metasplit.Vert() << tree << props;
	
	tree.NoHeader();
	tree_col_visible = tree.GetColumnCount();
	tree.AddColumn("", 18).NoEdit().FixedWidth(18).NoPadding().NoVertGrid().Ctrls([=](int line, One<Ctrl>& ctrl) {
		EyeToggle* t = new EyeToggle();
		t->NoWantFocus();
		int id = tree.GetItemAtLine(line);
		{
			Value v = tree.Get(id);
			VfsValue* node = v.Is<VfsValue*>() ? ValueTo<VfsValue*>(v) : 0;
			bool is_dir = node && IsVfsType(*node, AsTypeHash<GeomDirectory>());
			if (!GetNodeObject(tree.Get(id)) && !GetNodeRef(tree.Get(id)) && !is_dir)
				t->Disable();
		}
		int linei = line;
		t->WhenAction << [=] {
			int id = tree.GetItemAtLine(linei);
			TreeNodeRef* ref = GetNodeRef(tree.Get(id));
			GeomObject* obj = GetNodeObject(tree.Get(id));
			if (obj) {
				obj->is_visible = (bool)t->GetData();
				tree.SetRowValue(id, tree_col_visible, obj->is_visible);
			}
			else if (ref) {
				if (ref->kind == TreeNodeRef::K_PROGRAM) {
					e->state->program_visible = (bool)t->GetData();
					tree.SetRowValue(id, tree_col_visible, e->state->program_visible);
				}
				if (ref->kind == TreeNodeRef::K_FOCUS) {
					e->state->focus_visible = (bool)t->GetData();
					tree.SetRowValue(id, tree_col_visible, e->state->focus_visible);
				}
			}
			else {
				Value v = tree.Get(id);
				VfsValue* node = v.Is<VfsValue*>() ? ValueTo<VfsValue*>(v) : 0;
				if (node && IsVfsType(*node, AsTypeHash<GeomDirectory>())) {
					GeomDirectory& dir = node->GetExt<GeomDirectory>();
					bool any_visible = false;
					for (auto& s : dir.val.sub) {
						if (IsVfsType(s, AsTypeHash<GeomObject>())) {
							GeomObject& o = s.GetExt<GeomObject>();
							if (o.is_visible) {
								any_visible = true;
								break;
							}
						}
					}
					bool new_visible = !any_visible;
					for (auto& s : dir.val.sub) {
						if (IsVfsType(s, AsTypeHash<GeomObject>())) {
							GeomObject& o = s.GetExt<GeomObject>();
							o.is_visible = new_visible;
						}
					}
					Data();
				}
			}
			RefreshAll();
		};
		ctrl = t;
	});
	tree_col_locked = tree.GetColumnCount();
	tree.AddColumn("", 18).NoEdit().FixedWidth(18).NoPadding().NoVertGrid().Ctrls([=](int line, One<Ctrl>& ctrl) {
		LockToggle* t = new LockToggle();
		t->NoWantFocus();
		int id = tree.GetItemAtLine(line);
		if (!GetNodeObject(tree.Get(id)))
			t->Disable();
		int linei = line;
		t->WhenAction << [=] {
			int id = tree.GetItemAtLine(linei);
			GeomObject* obj = GetNodeObject(tree.Get(id));
			if (!obj)
				return;
			obj->is_locked = (bool)t->GetData();
			tree.SetRowValue(id, tree_col_locked, obj->is_locked);
		};
		ctrl = t;
	});
	tree_col_read = tree.GetColumnCount();
	tree.AddColumn("R", 18).NoEdit().FixedWidth(18).NoPadding().NoVertGrid().Ctrls([=](int line, One<Ctrl>& ctrl) {
		TextToggle* t = new TextToggle("R");
		t->NoWantFocus();
		int linei = line;
		t->WhenAction << [=] {
			int id = tree.GetItemAtLine(linei);
			TreeNodeRef* ref = GetNodeRef(tree.Get(id));
			GeomObject* obj = GetNodeObject(tree.Get(id));
			if (obj) {
				obj->read_enabled = (bool)t->GetData();
				if (!obj->read_enabled) {
					obj->write_enabled = false;
					tree.SetRowValue(id, tree_col_write, obj->write_enabled);
				}
				tree.SetRowValue(id, tree_col_read, obj->read_enabled);
			}
			else if (ref) {
				if (ref->kind == TreeNodeRef::K_PROGRAM) {
					program_read = (bool)t->GetData();
					if (!program_read) {
						program_write = false;
						tree.SetRowValue(id, tree_col_write, program_write);
					}
					tree.SetRowValue(id, tree_col_read, program_read);
				}
				if (ref->kind == TreeNodeRef::K_FOCUS) {
					focus_read = (bool)t->GetData();
					if (!focus_read) {
						focus_write = false;
						tree.SetRowValue(id, tree_col_write, focus_write);
					}
					tree.SetRowValue(id, tree_col_read, focus_read);
				}
			}
		};
		ctrl = t;
	});
	tree_col_write = tree.GetColumnCount();
	tree.AddColumn("W", 18).NoEdit().FixedWidth(18).NoPadding().NoVertGrid().Ctrls([=](int line, One<Ctrl>& ctrl) {
		TextToggle* t = new TextToggle("W");
		t->NoWantFocus();
		int linei = line;
		t->WhenAction << [=] {
			int id = tree.GetItemAtLine(linei);
			TreeNodeRef* ref = GetNodeRef(tree.Get(id));
			GeomObject* obj = GetNodeObject(tree.Get(id));
			if (obj) {
				bool w = (bool)t->GetData();
				if (w && !obj->read_enabled) {
					obj->read_enabled = true;
					tree.SetRowValue(id, tree_col_read, obj->read_enabled);
				}
				obj->write_enabled = w;
				tree.SetRowValue(id, tree_col_write, obj->write_enabled);
			}
			else if (ref) {
				if (ref->kind == TreeNodeRef::K_PROGRAM) {
					bool w = (bool)t->GetData();
					if (w && !program_read) {
						program_read = true;
						tree.SetRowValue(id, tree_col_read, program_read);
					}
					program_write = w;
					tree.SetRowValue(id, tree_col_write, program_write);
				}
				if (ref->kind == TreeNodeRef::K_FOCUS) {
					bool w = (bool)t->GetData();
					if (w && !focus_read) {
						focus_read = true;
						tree.SetRowValue(id, tree_col_read, focus_read);
					}
					focus_write = w;
					tree.SetRowValue(id, tree_col_write, focus_write);
				}
			}
		};
		ctrl = t;
	});
	vsplit.Vert().SetPos(7500) << grid << time;
	
	grid.SetGridSize(2,2);
	for(int i = 0; i < 4; i++) {
		rends[i].ctx = &e->render_ctx;
		rends[i].WhenChanged = THISBACK1(RefreshRenderer, i);
		rends[i].WhenMenu = THISBACK1(BuildViewMenu, i);
	}
	rends[0].SetViewMode(VIEWMODE_YZ);
	rends[1].SetViewMode(VIEWMODE_XZ);
	rends[2].SetViewMode(VIEWMODE_XY);
	rends[3].SetViewMode(VIEWMODE_PERSPECTIVE);
	rends[0].SetCameraSource(CAMSRC_FOCUS);
	rends[1].SetCameraSource(CAMSRC_FOCUS);
	rends[2].SetCameraSource(CAMSRC_PROGRAM);
	rends[3].SetCameraSource(CAMSRC_FOCUS);

	grid.Add(rends[0]);
	grid.Add(rends[1]);
	grid.Add(rends[2]);
	grid.Add(rends[3]);
	
	props.NoHeader();
	props_col_value = props.GetColumnCount();
	props.AddColumn("Value", 300).NoEdit();
	props.ColumnAt(0).FixedWidth(180);
	props.ColumnAt(1).FixedWidth(300);
	for (int i = 2; i < props.GetColumnCount(); i++) {
		props.HeaderTab(i).Hide();
		props.ColumnAt(i).SetWidth(0);
	}
	props.WhenOpen << [=](int) {
		if (props_refreshing)
			return;
		PostCallback([=] { PropsData(); });
	};
	props.WhenClose << [=](int) {
		if (props_refreshing)
			return;
		PostCallback([=] { PropsData(); });
	};
	
	
}

void GeomProjectCtrl::RefreshRenderer(int i) {
	if (i >= 0 && i < 4)
		rends[i].Refresh();
}

void GeomProjectCtrl::RefreshAll() {
	for (int i = 0; i < 4; i++)
		rends[i].Refresh();
}

void GeomProjectCtrl::Update(double dt) {
	GeomAnim& anim = *e->anim;
	GeomVideo& video = e->video;
	bool was_playing = anim.is_playing || video.is_importing;
	
	if (video.is_importing) {
		video.Update(dt);
		TimelineData();
	}
	else {
		anim.Update(dt);
	}
	
	time.SetSelectedColumn(anim.position);
	time.Refresh();
	
	if (anim.is_playing || was_playing) {
		for(int i = 0; i < 4; i++) {
			rends[i].Refresh();
		}
	}
}

void GeomProjectCtrl::Data() {
	GeomProject& prj = *e->prj;
	
	tree.Clear();
	tree.SetRoot(ImagesImg::Root(), "Project");
	
	tree_nodes.Clear();
	int builtin = tree.Add(0, ImagesImg::Cameras(), "Builtin");
	TreeNodeRef& program = tree_nodes.Add();
	program.kind = TreeNodeRef::K_PROGRAM;
	TreeNodeRef& focus = tree_nodes.Add();
	focus.kind = TreeNodeRef::K_FOCUS;
	int program_id = tree.Add(builtin, ImagesImg::Camera(), RawToValue(&program), "Program Camera");
	int focus_id = tree.Add(builtin, ImagesImg::Camera(), RawToValue(&focus), "Focus Camera");
	tree.SetRowValue(program_id, tree_col_visible, e->state->program_visible);
	tree.SetRowValue(program_id, tree_col_locked, false);
	tree.SetRowValue(program_id, tree_col_read, program_read);
	tree.SetRowValue(program_id, tree_col_write, program_write);
	tree.SetRowValue(focus_id, tree_col_visible, e->state->focus_visible);
	tree.SetRowValue(focus_id, tree_col_locked, false);
	tree.SetRowValue(focus_id, tree_col_read, focus_read);
	tree.SetRowValue(focus_id, tree_col_write, focus_write);

	
	tree_scenes = tree.Add(0, ImagesImg::Scenes(), "Scenes");
	
	int scene_idx = 0;
	for (auto& s : prj.val.sub) {
		if (!IsVfsType(s, AsTypeHash<GeomScene>()))
			continue;
		GeomScene& scene = s.GetExt<GeomScene>();
		String name = scene.name.IsEmpty() ? "Scene #" + IntStr(scene_idx) : scene.name;
		int j = tree.Add(tree_scenes, ImagesImg::Scene(), RawToValue(&scene.val), name);
		tree.SetRowValue(j, tree_col_visible, Null);
		tree.SetRowValue(j, tree_col_locked, Null);
		tree.SetRowValue(j, tree_col_read, Null);
		tree.SetRowValue(j, tree_col_write, Null);
		
		TreeValue(j, scene.val);
		
		if (scene_idx == 0 && !tree.HasFocus())
			tree.SetCursor(j);
		scene_idx++;
	}
	
	/*for(int i = 0; i < prj.octrees.GetCount(); i++) {
		OctreePointModel& o = prj.octrees[i];
		String name = prj.dictionary[o.id];
		tree.Add(tree_octrees, ImagesImg::Octree(), o.id, name);
	}*/
	
	tree.Open(0);
	
	TreeSelect();
}

void GeomProjectCtrl::TreeSelect() {
	int cursor = tree.GetCursor();
	if (cursor < 0)
		return;
	Value v = tree.Get(cursor);
	TreeNodeRef* ref = GetNodeRef(v);
	GeomObject* obj = GetNodeObject(v);
	GeomPointcloudDataset* ds = GetNodeDataset(v);
	selected_dataset = ds;
	if (ref) {
		if (ref->kind == TreeNodeRef::K_PROGRAM) {
			e->state->focus_mode = 2;
			e->state->focus_object_key = 0;
		}
		if (ref->kind == TreeNodeRef::K_FOCUS) {
			e->state->focus_mode = 3;
			e->state->focus_object_key = 0;
		}
		UpdateTreeFocus(cursor);
		PropsData();
		RefreshAll();
		return;
	}
	if (obj) {
		e->state->focus_mode = 1;
		e->state->focus_object_key = obj->key;
		UpdateTreeFocus(cursor);
		PropsData();
		RefreshAll();
		return;
	}
	if (ds) {
		UpdateTreeFocus(cursor);
		PropsData();
		return;
	}
	if (!v.Is<VfsValue*>())
		return;
	VfsValue* node = ValueTo<VfsValue*>(v);
	if (!node)
		return;
	if (IsVfsType(*node, AsTypeHash<GeomScene>())) {
		int idx = 0;
		for (auto& s : e->prj->val.sub) {
			if (!IsVfsType(s, AsTypeHash<GeomScene>()))
				continue;
			if (&s == node) {
				e->state->active_scene = idx;
				e->state->UpdateObjects();
				RefreshAll();
				break;
			}
			idx++;
		}
	}
	UpdateTreeFocus(cursor);
	PropsData();
}

GeomProjectCtrl::TreeNodeRef* GeomProjectCtrl::GetNodeRef(const Value& v) {
	if (v.Is<TreeNodeRef*>())
		return ValueTo<TreeNodeRef*>(v);
	return 0;
}

GeomObject* GeomProjectCtrl::GetNodeObject(const Value& v) {
	if (!v.Is<VfsValue*>())
		return 0;
	VfsValue* node = ValueTo<VfsValue*>(v);
	if (!node)
		return 0;
	if (!IsVfsType(*node, AsTypeHash<GeomObject>()))
		return 0;
	return &node->GetExt<GeomObject>();
}

GeomPointcloudDataset* GeomProjectCtrl::GetNodeDataset(const Value& v) {
	if (!v.Is<VfsValue*>())
		return 0;
	VfsValue* node = ValueTo<VfsValue*>(v);
	if (!node)
		return 0;
	if (!IsVfsType(*node, AsTypeHash<GeomPointcloudDataset>()))
		return 0;
	return &node->GetExt<GeomPointcloudDataset>();
}

void GeomProjectCtrl::UpdateTreeFocus(int new_id) {
	if (focus_tree_id >= 0) {
		int linei = tree.GetLineAtItem(focus_tree_id);
		if (linei >= 0)
			tree.SetLineColor(linei, Null);
	}
	focus_tree_id = new_id;
	if (focus_tree_id >= 0) {
		int linei = tree.GetLineAtItem(focus_tree_id);
		if (linei >= 0)
			tree.SetLineColor(linei, Blend(SColorHighlight(), SColorPaper(), 200));
	}
}

void GeomProjectCtrl::TreeMenu(Bar& bar) {
	int cursor = tree.GetCursor();
	if (cursor < 0)
		return;
	Value v = tree.Get(cursor);
	TreeNodeRef* ref = GetNodeRef(v);
	GeomObject* obj = GetNodeObject(v);
	VfsValue* node = v.Is<VfsValue*>() ? ValueTo<VfsValue*>(v) : 0;
	GeomDirectory* dir = 0;
	if (node && IsVfsType(*node, AsTypeHash<GeomDirectory>()))
		dir = &node->GetExt<GeomDirectory>();
	if (!dir && node && IsVfsType(*node, AsTypeHash<GeomObject>()) && node->owner) {
		if (IsVfsType(*node->owner, AsTypeHash<GeomDirectory>()))
			dir = &node->owner->GetExt<GeomDirectory>();
	}
	if (!ref && !obj && !dir)
		return;
	bar.Add(t_("Go to"), [=] {
		PointcloudPose pose;
		bool ok = false;
		if (obj) {
			const GeomObjectState* os = e->state->FindObjectStateByKey(obj->key);
			if (os) {
				pose.position = os->position;
				pose.orientation = os->orientation;
				ok = true;
			}
		}
		if (!ok && ref) {
			if (ref->kind == TreeNodeRef::K_PROGRAM) {
				GeomCamera& cam = e->state->GetProgram();
				pose.position = cam.position;
				pose.orientation = cam.orientation;
				ok = true;
			}
			if (ref->kind == TreeNodeRef::K_FOCUS) {
				GeomCamera& cam = e->state->GetFocus();
				pose.position = cam.position;
				pose.orientation = cam.orientation;
				ok = true;
			}
		}
		if (ok) {
			GeomCamera& focus = e->state->GetFocus();
			focus.position = pose.position;
			focus.orientation = pose.orientation;
			RefreshAll();
		}
	});
	if (dir) {
		auto find_dir = [&](GeomDirectory& parent, const String& name) -> GeomDirectory* {
			for (auto& sub : parent.val.sub) {
				if (!IsVfsType(sub, AsTypeHash<GeomDirectory>()))
					continue;
				GeomDirectory& d = sub.GetExt<GeomDirectory>();
				if (d.name == name || d.val.id == name)
					return &d;
			}
			return 0;
		};
		auto unique_name = [&](GeomDirectory& parent, const String& base) {
			String name = base;
			int idx = 1;
			while (parent.FindObject(name) || find_dir(parent, name)) {
				name = base + IntStr(idx++);
			}
			return name;
		};
		bar.Sub(t_("Add"), [=](Bar& bar) {
			bar.Add(t_("Directory"), [=] {
				String name = unique_name(*dir, "dir");
				dir->GetAddDirectory(name);
				e->state->UpdateObjects();
				e->RefreshData();
			});
			bar.Add(t_("Camera"), [=] {
				String name = unique_name(*dir, "camera");
				dir->GetAddCamera(name);
				e->state->UpdateObjects();
				e->RefreshData();
			});
			bar.Add(t_("Model"), [=] {
				String name = unique_name(*dir, "model");
				dir->GetAddModel(name);
				e->state->UpdateObjects();
				e->RefreshData();
			});
			bar.Add(t_("Pointcloud"), [=] {
				String name = unique_name(*dir, "pointcloud");
				dir->GetAddOctree(name);
				e->state->UpdateObjects();
				e->RefreshData();
			});
			bar.Add(t_("Pointcloud Dataset"), [=] {
				String name = unique_name(*dir, "dataset");
				dir->GetAddPointcloudDataset(name);
				e->state->UpdateObjects();
				e->RefreshData();
			});
		});
	}
	if (obj && obj->IsOctree()) {
		bar.Separator();
		bar.Add(t_("Generate Synthetic Pointcloud"), [=] {
			e->GenerateSyntheticPointcloudFor(*obj);
		});
		bar.Add(t_("Clear Pointcloud"), [=] {
			obj->octree.octree.Initialize(-3, 8);
			obj->octree_ptr = 0;
			e->state->UpdateObjects();
			RefreshAll();
		});
	}
}

void GeomProjectCtrl::PropsMenu(Bar& bar) {
	GeomObject* obj = GetNodeObject(tree.Get(tree.GetCursor()));
	if (!obj)
		return;
	bar.Sub(t_("Add Component"), [=](Bar& bar) {
		bar.Add(t_("Script"), [=] {
			e->AddScriptComponent(*obj);
			e->state->UpdateObjects();
			e->RefreshData();
		});
	});
}

void GeomProjectCtrl::PropsData() {
	if (props_refreshing)
		return;
	props_refreshing = true;
	props.Clear();
	props_nodes.Clear();
	props_ctrls.Clear();
	selected_obj = GetNodeObject(tree.Get(tree.GetCursor()));
	selected_ref = GetNodeRef(tree.Get(tree.GetCursor()));
	selected_dataset = GetNodeDataset(tree.Get(tree.GetCursor()));
	props.SetRoot(ImagesImg::Root(), "Properties");
	int root = 0;
	PropRef& efx = props_nodes.Add();
	efx.kind = PropRef::P_EFFECTS;
	int effects = props.Add(root, ImagesImg::Directory(), RawToValue(&efx), "Effects");
	PropRef& comps = props_nodes.Add();
	comps.kind = PropRef::P_COMPONENTS;
	int components = props.Add(root, ImagesImg::Directory(), RawToValue(&comps), t_("Components"));
	int pointcloud = -1;
	int dataset_id = -1;
	GeomPointcloudDataset* props_dataset = 0;
	int dataset_name_id = -1;
	int dataset_source_id = -1;
	if (selected_obj && selected_obj->IsOctree()) {
		PropRef& pc = props_nodes.Add();
		pc.kind = PropRef::P_POINTCLOUD;
		pointcloud = props.Add(root, ImagesImg::Directory(), RawToValue(&pc), t_("Pointcloud"));
		PropRef& ds = props_nodes.Add();
		ds.kind = PropRef::P_DATASET;
		dataset_id = props.Add(pointcloud, ImagesImg::Object(), RawToValue(&ds), t_("Dataset"));
	}
	else if (selected_dataset) {
		props_dataset = selected_dataset;
		PropRef& pc = props_nodes.Add();
		pc.kind = PropRef::P_POINTCLOUD;
		pointcloud = props.Add(root, ImagesImg::Directory(), RawToValue(&pc), t_("Pointcloud Dataset"));
		PropRef& name_ref = props_nodes.Add();
		name_ref.kind = PropRef::P_DATASET;
		dataset_name_id = props.Add(pointcloud, ImagesImg::Object(), RawToValue(&name_ref), t_("Name"));
		PropRef& src_ref = props_nodes.Add();
		src_ref.kind = PropRef::P_DATASET;
		dataset_source_id = props.Add(pointcloud, ImagesImg::Object(), RawToValue(&src_ref), t_("Source"));
	}
	PropRef& tnode = props_nodes.Add();
	tnode.kind = PropRef::P_TRANSFORM;
	int transform = props.Add(root, ImagesImg::Object(), RawToValue(&tnode), "Transform");
	PropRef& ppos = props_nodes.Add();
	ppos.kind = PropRef::P_POSITION;
	int pos_id = props.Add(transform, ImagesImg::Object(), RawToValue(&ppos), "Position");
	PropRef& pori = props_nodes.Add();
	pori.kind = PropRef::P_ORIENTATION;
	int ori_id = props.Add(transform, ImagesImg::Object(), RawToValue(&pori), "Orientation");

	int effect_pos_id = -1;
	int effect_ori_id = -1;
	int effect_trans_id = -1;
	PointcloudPose effect_pose = PointcloudPose::MakeIdentity();
	bool has_effect_pose = false;
	struct ScriptItemIds {
		GeomScript* script = 0;
		int file_id = -1;
		int enabled_id = -1;
		int run_on_load_id = -1;
		int run_frame_id = -1;
		int edit_id = -1;
		int run_id = -1;
	};
	Vector<ScriptItemIds> script_items;
	if (selected_obj && selected_obj == e->sim_observation_obj) {
		has_effect_pose = true;
		effect_pose = e->sim_localized_pose;
		PropRef& efxnode = props_nodes.Add();
		efxnode.kind = PropRef::P_TRANSFORM;
		int etrans = props.Add(effects, ImagesImg::Object(), RawToValue(&efxnode), "Localization");
		effect_trans_id = etrans;
		PropRef& epos = props_nodes.Add();
		epos.kind = PropRef::P_EFFECT_POSITION;
		effect_pos_id = props.Add(etrans, ImagesImg::Object(), RawToValue(&epos), "Position");
		PropRef& eori = props_nodes.Add();
		eori.kind = PropRef::P_EFFECT_ORIENTATION;
		effect_ori_id = props.Add(etrans, ImagesImg::Object(), RawToValue(&eori), "Orientation");
	}
	if (selected_obj) {
		for (auto& sub : selected_obj->val.sub) {
			if (!IsVfsType(sub, AsTypeHash<GeomScript>()))
				continue;
			GeomScript& script = sub.GetExt<GeomScript>();
			String label = script.file.IsEmpty() ? "Script" : GetFileName(script.file);
			PropRef& snode = props_nodes.Add();
			snode.kind = PropRef::P_SCRIPT;
			snode.script = &script;
			int script_id = props.Add(components, ImagesImg::Object(), RawToValue(&snode), label);
			PropRef& sfile = props_nodes.Add();
			sfile.kind = PropRef::P_SCRIPT_FILE;
			sfile.script = &script;
			int file_id = props.Add(script_id, ImagesImg::Object(), RawToValue(&sfile), t_("File"));
			PropRef& sen = props_nodes.Add();
			sen.kind = PropRef::P_SCRIPT_ENABLED;
			sen.script = &script;
			int enabled_id = props.Add(script_id, ImagesImg::Object(), RawToValue(&sen), t_("Enabled"));
			PropRef& sload = props_nodes.Add();
			sload.kind = PropRef::P_SCRIPT_RUN_ON_LOAD;
			sload.script = &script;
			int run_on_load_id = props.Add(script_id, ImagesImg::Object(), RawToValue(&sload), t_("Run on load"));
			PropRef& sframe = props_nodes.Add();
			sframe.kind = PropRef::P_SCRIPT_RUN_EACH_FRAME;
			sframe.script = &script;
			int run_frame_id = props.Add(script_id, ImagesImg::Object(), RawToValue(&sframe), t_("Run every frame"));
			PropRef& sedit = props_nodes.Add();
			sedit.kind = PropRef::P_SCRIPT_EDIT;
			sedit.script = &script;
			int edit_id = props.Add(script_id, ImagesImg::Object(), RawToValue(&sedit), t_("Edit"));
			PropRef& srun = props_nodes.Add();
			srun.kind = PropRef::P_SCRIPT_RUN;
			srun.script = &script;
			int run_id = props.Add(script_id, ImagesImg::Object(), RawToValue(&srun), t_("Run"));
			ScriptItemIds& ids = script_items.Add();
			ids.script = &script;
			ids.file_id = file_id;
			ids.enabled_id = enabled_id;
			ids.run_on_load_id = run_on_load_id;
			ids.run_frame_id = run_frame_id;
			ids.edit_id = edit_id;
			ids.run_id = run_id;
		}
	}
	props.Open(effects);
	props.Open(components);
	if (pointcloud >= 0)
		props.Open(pointcloud);

	vec3 pos(0, 0, 0);
	quat ori = Identity<quat>();
	if (selected_obj) {
		if (GeomTransform* tr = selected_obj->FindTransform()) {
			pos = tr->position;
			ori = tr->orientation;
		}
		else {
			const GeomObjectState* os = e->state->FindObjectStateByKey(selected_obj->key);
			if (os) {
				pos = os->position;
				ori = os->orientation;
			}
		}
	}
	else if (selected_ref) {
		if (selected_ref->kind == TreeNodeRef::K_PROGRAM) {
			GeomCamera& cam = e->state->GetProgram();
			pos = cam.position;
			ori = cam.orientation;
		}
		if (selected_ref->kind == TreeNodeRef::K_FOCUS) {
			GeomCamera& cam = e->state->GetFocus();
			pos = cam.position;
			ori = cam.orientation;
		}
	}

	props.OpenDeep(root);
	
	auto set_ctrl = [&](int id, One<Ctrl>&& c) {
		int line = props.GetLineAtItem(id);
		if (line >= 0)
			props.SetCtrl(line, props_col_value, *props_ctrls.Add(pick(c)), false);
	};
	
	if (selected_obj) {
		One<ToggleRowCtrl> t = MakeOne<ToggleRowCtrl>();
		t->SetVisible(selected_obj->is_visible);
		t->SetLocked(selected_obj->is_locked);
		t->WhenVisible << [=](bool v) {
			selected_obj->is_visible = v;
			tree.SetRowValue(focus_tree_id, tree_col_visible, selected_obj->is_visible);
			RefreshAll();
		};
		t->WhenLocked << [=](bool v) {
			selected_obj->is_locked = v;
			tree.SetRowValue(focus_tree_id, tree_col_locked, selected_obj->is_locked);
		};
		set_ctrl(root, pick(t));
		if (selected_obj == e->sim_observation_obj) {
			One<ToggleRowCtrl> tfx = MakeOne<ToggleRowCtrl>();
			tfx->SetVisible(e->sim_observation_effect_visible);
			tfx->SetLocked(e->sim_observation_effect_locked);
			tfx->WhenVisible << [=](bool v) {
				e->sim_observation_effect_visible = v;
				e->RefreshSimObservation();
			};
			tfx->WhenLocked << [=](bool v) {
				e->sim_observation_effect_locked = v;
			};
			set_ctrl(effects, pick(tfx));
		}
	}

	if (selected_obj && selected_obj->IsOctree() && dataset_id >= 0) {
		One<DropList> ds_ctrl = MakeOne<DropList>();
		ds_ctrl->Add(String(), t_("None"));
		GeomScene& scene = e->state->GetActiveScene();
		for (auto& s : scene.val.sub) {
			if (!IsVfsType(s, AsTypeHash<GeomPointcloudDataset>()))
				continue;
			GeomPointcloudDataset& ds = s.GetExt<GeomPointcloudDataset>();
			String id = ds.GetId();
			ds_ctrl->Add(id, id);
		}
		ds_ctrl->SetData(selected_obj->pointcloud_ref);
		DropList* ds_ptr = ds_ctrl.Get();
		ds_ctrl->WhenAction << [=] {
			if (ds_ptr)
				selected_obj->pointcloud_ref = AsString(ds_ptr->GetData());
			e->state->UpdateObjects();
			RefreshAll();
		};
		set_ctrl(dataset_id, pick(ds_ctrl));
	}

	if (props_dataset) {
		One<EditString> name_ctrl = MakeOne<EditString>();
		name_ctrl->SetData(props_dataset->GetId());
		EditString* name_ptr = name_ctrl.Get();
		name_ctrl->WhenAction << [=] {
			if (!name_ptr)
				return;
			String old_id = props_dataset->GetId();
			String new_id = ~*name_ptr;
			if (new_id.IsEmpty() || new_id == old_id)
				return;
			props_dataset->name = new_id;
			props_dataset->val.id = new_id;
			GeomScene& scene = e->state->GetActiveScene();
			GeomObjectCollection objs(scene);
			for (GeomObject& o : objs) {
				if (o.pointcloud_ref == old_id)
					o.pointcloud_ref = new_id;
			}
			e->state->UpdateObjects();
			e->RefreshData();
		};
		set_ctrl(dataset_name_id, pick(name_ctrl));

		One<EditString> source_ctrl = MakeOne<EditString>();
		source_ctrl->SetData(props_dataset->source_ref);
		EditString* source_ptr = source_ctrl.Get();
		source_ctrl->WhenAction << [=] {
			if (source_ptr)
				props_dataset->source_ref = ~*source_ptr;
			e->state->UpdateObjects();
			e->RefreshData();
		};
		set_ctrl(dataset_source_id, pick(source_ctrl));
	}

	for (const ScriptItemIds& ids : script_items) {
		if (!ids.script)
			continue;
		GeomScript& script = *ids.script;
		One<EditString> file_ctrl = MakeOne<EditString>();
		file_ctrl->SetData(script.file);
		EditString* file_ctrl_ptr = file_ctrl.Get();
		file_ctrl->WhenAction << [=] {
			if (file_ctrl_ptr)
				script.file = ~*file_ctrl_ptr;
			e->EnsureScriptInstances();
		};
		set_ctrl(ids.file_id, pick(file_ctrl));

		One<Option> enabled = MakeOne<Option>();
		enabled->SetData(script.enabled);
		Option* enabled_ptr = enabled.Get();
		enabled->WhenAction << [=] {
			if (enabled_ptr)
				script.enabled = (bool)enabled_ptr->GetData();
			e->EnsureScriptInstances();
		};
		set_ctrl(ids.enabled_id, pick(enabled));

		One<Option> run_on_load = MakeOne<Option>();
		run_on_load->SetData(script.run_on_load);
		Option* run_on_load_ptr = run_on_load.Get();
		run_on_load->WhenAction << [=] {
			if (run_on_load_ptr)
				script.run_on_load = (bool)run_on_load_ptr->GetData();
		};
		set_ctrl(ids.run_on_load_id, pick(run_on_load));

		One<Option> run_frame = MakeOne<Option>();
		run_frame->SetData(script.run_every_frame);
		Option* run_frame_ptr = run_frame.Get();
		run_frame->WhenAction << [=] {
			if (run_frame_ptr)
				script.run_every_frame = (bool)run_frame_ptr->GetData();
		};
		set_ctrl(ids.run_frame_id, pick(run_frame));

		One<Button> edit = MakeOne<Button>();
		edit->SetLabel(t_("Edit..."));
		edit->WhenAction << [=] { e->OpenScriptEditor(script); };
		set_ctrl(ids.edit_id, pick(edit));

		One<Button> run = MakeOne<Button>();
		run->SetLabel(t_("Run"));
		run->WhenAction << [=] { e->RunScriptOnce(script); };
		set_ctrl(ids.run_id, pick(run));
	}
	
	One<Vec3EditCtrl> pos_ctrl = MakeOne<Vec3EditCtrl>();
	pos_ctrl->SetValue(pos);
	pos_ctrl->SetEditable(true);
	pos_ctrl->WhenAction << [=] {
		props.SetCursor(pos_id);
		PropsApply();
	};
	set_ctrl(pos_id, pick(pos_ctrl));
	
	One<QuatEditCtrl> ori_ctrl = MakeOne<QuatEditCtrl>();
	ori_ctrl->SetValue(ori);
	ori_ctrl->SetEditable(true);
	ori_ctrl->WhenAction << [=] {
		props.SetCursor(ori_id);
		PropsApply();
	};
	set_ctrl(ori_id, pick(ori_ctrl));
	
	if (has_effect_pose) {
		One<Vec3EditCtrl> epos_ctrl = MakeOne<Vec3EditCtrl>();
		epos_ctrl->SetValue(effect_pose.position);
		epos_ctrl->SetEditable(false);
		set_ctrl(effect_pos_id, pick(epos_ctrl));
		
		One<QuatEditCtrl> eori_ctrl = MakeOne<QuatEditCtrl>();
		eori_ctrl->SetValue(effect_pose.orientation);
		eori_ctrl->SetEditable(false);
		set_ctrl(effect_ori_id, pick(eori_ctrl));
		
		if (selected_obj == e->sim_observation_obj) {
			One<ToggleRowCtrl> tfx = MakeOne<ToggleRowCtrl>();
			tfx->SetVisible(e->sim_observation_effect_visible);
			tfx->SetLocked(e->sim_observation_effect_locked);
			tfx->WhenVisible << [=](bool v) {
				e->sim_observation_effect_visible = v;
				e->RefreshSimObservation();
			};
			tfx->WhenLocked << [=](bool v) {
				e->sim_observation_effect_locked = v;
			};
			if (effect_trans_id >= 0)
				set_ctrl(effect_trans_id, pick(tfx));
		}
	}
	props_refreshing = false;
}

void GeomProjectCtrl::PropsApply() {
	int linei = props.GetCursor();
	if (linei < 0)
		return;
	int id = props.GetItemAtLine(linei);
	if (id < 0)
		return;
	Value v = props.Get(id);
	if (!v.Is<PropRef*>())
		return;
	PropRef* pr = ValueTo<PropRef*>(v);
	if (!pr)
		return;

	auto get_vec = [&](int line) {
		if (Ctrl* c = props.GetCtrl(line, props_col_value)) {
			if (Vec3EditCtrl* vc = dynamic_cast<Vec3EditCtrl*>(c))
				return vc->GetValue();
		}
		return vec3(0, 0, 0);
	};
	auto get_quat = [&](int line) {
		if (Ctrl* c = props.GetCtrl(line, props_col_value)) {
			if (QuatEditCtrl* qc = dynamic_cast<QuatEditCtrl*>(c))
				return qc->GetValue();
		}
		return Identity<quat>();
	};

	bool allow_write = true;
	if (selected_obj)
		allow_write = !selected_obj->is_locked;
	if (selected_ref) {
		if (selected_ref->kind == TreeNodeRef::K_PROGRAM)
			allow_write = program_write;
		if (selected_ref->kind == TreeNodeRef::K_FOCUS)
			allow_write = focus_write;
	}
	if (!allow_write) {
		PropsData();
		return;
	}

	if (pr->kind == PropRef::P_EFFECT_POSITION || pr->kind == PropRef::P_EFFECT_ORIENTATION) {
		PropsData();
		return;
	}
	if (selected_obj) {
		vec3 pos = pr->kind == PropRef::P_POSITION ? get_vec(linei) : vec3(0);
		quat ori = pr->kind == PropRef::P_ORIENTATION ? get_quat(linei) : Identity<quat>();
		if (GeomTransform* tr = selected_obj->FindTransform()) {
			if (pr->kind == PropRef::P_POSITION)
				tr->position = pos;
			if (pr->kind == PropRef::P_ORIENTATION)
				tr->orientation = ori;
		}
		if (selected_obj->write_enabled) {
			GeomTimeline& tl = selected_obj->GetTimeline();
			int frame = e->anim->position;
			GeomKeypoint& kp = tl.GetAddKeypoint(frame);
			if (pr->kind == PropRef::P_POSITION)
				kp.position = pos;
			if (pr->kind == PropRef::P_ORIENTATION)
				kp.orientation = ori;
		}
		else {
			selected_obj->read_enabled = false;
			tree.SetRowValue(focus_tree_id, tree_col_read, selected_obj->read_enabled);
		}
		e->state->UpdateObjects();
		RefreshAll();
		return;
	}
	if (selected_ref) {
		if (selected_ref->kind == TreeNodeRef::K_PROGRAM) {
			GeomCamera& cam = e->state->GetProgram();
			if (pr->kind == PropRef::P_POSITION)
				cam.position = get_vec(linei);
			if (pr->kind == PropRef::P_ORIENTATION)
				cam.orientation = get_quat(linei);
		}
		if (selected_ref->kind == TreeNodeRef::K_FOCUS) {
			GeomCamera& cam = e->state->GetFocus();
			if (pr->kind == PropRef::P_POSITION)
				cam.position = get_vec(linei);
			if (pr->kind == PropRef::P_ORIENTATION)
				cam.orientation = get_quat(linei);
		}
		RefreshAll();
	}
}

void GeomProjectCtrl::BuildViewMenu(Bar& bar, int i) {
	if (i < 0 || i >= 4)
		return;
	bar.Add(t_("View: YZ"), [=] { rends[i].SetViewMode(VIEWMODE_YZ); RefreshRenderer(i); });
	bar.Add(t_("View: XZ"), [=] { rends[i].SetViewMode(VIEWMODE_XZ); RefreshRenderer(i); });
	bar.Add(t_("View: XY"), [=] { rends[i].SetViewMode(VIEWMODE_XY); RefreshRenderer(i); });
	bar.Add(t_("View: Perspective"), [=] { rends[i].SetViewMode(VIEWMODE_PERSPECTIVE); RefreshRenderer(i); });
	bar.Separator();
	bar.Add(t_("Camera: Focus"), [=] { rends[i].SetCameraSource(CAMSRC_FOCUS); RefreshRenderer(i); });
	bar.Add(t_("Camera: Program"), [=] { rends[i].SetCameraSource(CAMSRC_PROGRAM); RefreshRenderer(i); });
	bar.Separator();
	bar.Add(t_("Reset Camera"), [=] {
		GeomCamera& cam = rends[i].GetGeomCamera();
		cam.position = vec3(0, 0, 0);
		cam.orientation = Identity<quat>();
		cam.scale = 1.0f;
		RefreshRenderer(i);
	});
}

void GeomProjectCtrl::OnCursor(int i) {
	e->anim->position = i;
}

void GeomProjectCtrl::TreeValue(int id, VfsValue& node) {
	auto warn_unknown = [&](const VfsValue& n) {
		hash_t type_hash = n.ext ? n.ext->GetTypeHash() : n.type_hash;
		if (warned_tree_types.Find(type_hash) >= 0)
			return;
		warned_tree_types.Add(type_hash);
		LOG("GeomProjectCtrl: unexpected VfsValue type in tree: " + n.GetTypeString());
	};
	Vector<VfsValue*> dirs;
	Vector<VfsValue*> objs;
	Vector<VfsValue*> datasets;
	for (auto& s : node.sub) {
		if (IsVfsType(s, AsTypeHash<GeomDirectory>()))
			dirs.Add(&s);
		else if (IsVfsType(s, AsTypeHash<GeomObject>()))
			objs.Add(&s);
		else if (IsVfsType(s, AsTypeHash<GeomPointcloudDataset>()))
			datasets.Add(&s);
		else
			warn_unknown(s);
	}
	for (VfsValue* s : dirs) {
		GeomDirectory& dir = s->GetExt<GeomDirectory>();
		String name = dir.name.IsEmpty() ? dir.val.id : dir.name;
		int j = tree.Add(id, ImagesImg::Directory(), RawToValue(s), name);
		tree.SetRowValue(j, tree_col_visible, Null);
		tree.SetRowValue(j, tree_col_locked, Null);
		tree.SetRowValue(j, tree_col_read, Null);
		tree.SetRowValue(j, tree_col_write, Null);
		TreeValue(j, *s);
	}
	for (VfsValue* s : objs) {
		GeomObject& o = s->GetExt<GeomObject>();
		Image img;
		switch (o.type) {
			case GeomObject::O_CAMERA: img = ImagesImg::Camera(); break;
			case GeomObject::O_MODEL:  img = ImagesImg::Model(); break;
			case GeomObject::O_OCTREE: img = ImagesImg::Octree(); break;
			default: img = ImagesImg::Object();
		}
		String name = o.name.IsEmpty() ? s->id : o.name;
		int j = tree.Add(id, img, RawToValue(s), name);
		tree.SetRowValue(j, tree_col_visible, o.is_visible);
		tree.SetRowValue(j, tree_col_locked, o.is_locked);
		tree.SetRowValue(j, tree_col_read, o.read_enabled);
		tree.SetRowValue(j, tree_col_write, o.write_enabled);
	}
	for (VfsValue* s : datasets) {
		GeomPointcloudDataset& ds = s->GetExt<GeomPointcloudDataset>();
		String name = ds.name.IsEmpty() ? s->id : ds.name;
		int j = tree.Add(id, ImagesImg::Octree(), RawToValue(s), name);
		tree.SetRowValue(j, tree_col_visible, Null);
		tree.SetRowValue(j, tree_col_locked, Null);
		tree.SetRowValue(j, tree_col_read, Null);
		tree.SetRowValue(j, tree_col_write, Null);
	}
}

void GeomProjectCtrl::TimelineData() {
	GeomProject& prj = *e->prj;
	GeomScene& scene = e->state->GetActiveScene();
	Vector<GeomObject*> objects;
	GeomObjectCollection collection(scene);
	for (GeomObject& o : collection)
		objects.Add(&o);
	
	time.SetCount(objects.GetCount());
	time.SetKeypointRate(prj.kps);
	time.SetLength(scene.length);
	time.SetKeypointColumnWidth(13);
	
	for(int i = 0; i < objects.GetCount(); i++) {
		GeomObject& o = *objects[i];
		/*int j = prj.list[i];
		int id = j / GeomProject::O_COUNT;
		int type = j % GeomProject::O_COUNT;*/
		
		String name = o.name.IsEmpty() ? IntStr(i) : o.name;
		
		TimelineRowCtrl& row = time.GetRowIndex(i);
		row.SetTitle(name);
		
		GeomTimeline* tl = o.FindTimeline();
		if (tl)
			row.SetKeypoints(tl->keypoints.GetKeys());
		else
			row.SetKeypoints(Vector<int>());
		
		row.Refresh();
	}
	
	time.Refresh();
}

END_UPP_NAMESPACE
