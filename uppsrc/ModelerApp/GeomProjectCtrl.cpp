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

struct KeyframeCtrl : Ctrl {
	Button left;
	Button key;
	Button right;
	Event<> WhenPrev;
	Event<> WhenToggle;
	Event<> WhenNext;
	bool keyed = false;
	
	KeyframeCtrl() {
		left.SetLabel("<");
		key.SetLabel("o");
		right.SetLabel(">");
		Add(left);
		Add(key);
		Add(right);
		left.WhenAction = [=] { WhenPrev(); };
		key.WhenAction = [=] { WhenToggle(); };
		right.WhenAction = [=] { WhenNext(); };
	}
	
	void SetKeyed(bool v) {
		keyed = v;
		key.SetLabel(keyed ? "*" : "o");
	}
	
	virtual void Layout() {
		Rect r = GetSize();
		int w = max(1, r.Width() / 3);
		left.SetRect(r.left, r.top, w, r.Height());
		key.SetRect(r.left + w, r.top, w, r.Height());
		right.SetRect(r.left + w * 2, r.top, r.Width() - w * 2, r.Height());
	}
};

struct MaterialPreviewCtrl : Ctrl {
	vec3 base = vec3(1, 1, 1);
	vec3 emissive = vec3(0, 0, 0);
	Image preview_img;

	static Image ByteImageToImage(const ByteImage& src) {
		if (!src.data || src.sz.cx <= 0 || src.sz.cy <= 0)
			return Image();
		int ch = src.channels;
		if (ch <= 0)
			ch = 4;
		ImageBuffer ib(src.sz);
		int pitch = src.pitch ? src.pitch : src.sz.cx * ch;
		for (int y = 0; y < src.sz.cy; y++) {
			const byte* s = src.data + y * pitch;
			RGBA* d = ib[y];
			for (int x = 0; x < src.sz.cx; x++) {
				if (ch >= 4) {
					d[x].r = s[x * ch + 0];
					d[x].g = s[x * ch + 1];
					d[x].b = s[x * ch + 2];
					d[x].a = s[x * ch + 3];
				}
				else if (ch == 3) {
					d[x].r = s[x * ch + 0];
					d[x].g = s[x * ch + 1];
					d[x].b = s[x * ch + 2];
					d[x].a = 255;
				}
				else {
					byte v = s[x * ch + 0];
					d[x].r = v;
					d[x].g = v;
					d[x].b = v;
					d[x].a = 255;
				}
			}
		}
		return ib;
	}

	struct PreviewPopup : TopWindow {
		ImageCtrl img;

		PreviewPopup() {
			NoWantFocus();
			ToolWindow();
			FrameLess();
			Add(img.SizePos());
		}

		void SetImage(const Image& m) {
			img.SetImage(m);
			Refresh();
		}

		virtual void LeftDown(Point, dword) {
			Close();
		}

		virtual void Deactivate() {
			Close();
		}
	};

	One<PreviewPopup> popup;

	void SetMaterial(const Material& m) {
		base = m.params->base_clr_factor.Splice();
		emissive = m.params->emissive_factor;
		preview_img.Clear();
		if (m.owner) {
			int tex_id = m.tex_id[TEXTYPE_DIFFUSE];
			if (tex_id >= 0) {
				int idx = m.owner->textures.Find(tex_id);
				if (idx >= 0)
					preview_img = ByteImageToImage(m.owner->textures[idx].img);
			}
		}
		Refresh();
	}

	virtual void LeftDown(Point, dword) {
		if (preview_img.IsEmpty())
			return;
		if (popup && popup->IsOpen()) {
			popup->Close();
			return;
		}
		if (!popup)
			popup.Create();
		Size sz = preview_img.GetSize();
		const int max_dim = 256;
		if (sz.cx > max_dim || sz.cy > max_dim) {
			float scale = min((float)max_dim / (float)sz.cx, (float)max_dim / (float)sz.cy);
			sz = Size(max(1, (int)ceil(sz.cx * scale)), max(1, (int)ceil(sz.cy * scale)));
		}
		Image shown = (sz == preview_img.GetSize()) ? preview_img : CachedRescale(preview_img, sz, FILTER_BILINEAR);
		popup->SetImage(shown);
		Rect r = GetScreenRect();
		Rect wr = GetWorkArea();
		int x = r.left;
		int y = r.bottom + 2;
		if (y + sz.cy > wr.bottom)
			y = max(wr.top, r.top - sz.cy - 2);
		if (x + sz.cx > wr.right)
			x = max(wr.left, wr.right - sz.cx);
		popup->SetRect(x, y, sz.cx, sz.cy);
		popup->PopUp(this, true, false, GUI_DropShadows());
	}

	virtual void Paint(Draw& w) {
		Size sz = GetSize();
		w.DrawRect(sz, SColorPaper());
		int r = min(sz.cx, sz.cy) / 2 - 2;
		Point c(sz.cx / 2, sz.cy / 2);
		if (r <= 0)
			return;
		for (int y = -r; y <= r; y++) {
			for (int x = -r; x <= r; x++) {
				if (x * x + y * y > r * r)
					continue;
				float nx = (float)x / (float)r;
				float ny = (float)y / (float)r;
				float nz2 = max(0.0f, 1.0f - nx * nx - ny * ny);
				float nz = sqrt(nz2);
				float diff = max(0.0f, 0.4f * nx + 0.7f * ny + 0.5f * nz);
				float intensity = 0.2f + diff * 0.8f;
				vec3 col = base * intensity + emissive;
				col[0] = Clamp(col[0], 0.0f, 1.0f);
				col[1] = Clamp(col[1], 0.0f, 1.0f);
				col[2] = Clamp(col[2], 0.0f, 1.0f);
				w.DrawRect(c.x + x, c.y + y, 1, 1, Color((byte)(col[0] * 255), (byte)(col[1] * 255), (byte)(col[2] * 255)));
			}
		}
	}
};

struct TexturePreviewCtrl : Ctrl {
	Image preview_img;

	struct PreviewPopup : TopWindow {
		ImageCtrl img;

		PreviewPopup() {
			NoWantFocus();
			ToolWindow();
			FrameLess();
			Add(img.SizePos());
		}

		void SetImage(const Image& m) {
			img.SetImage(m);
			Refresh();
		}

		virtual void LeftDown(Point, dword) {
			Close();
		}

		virtual void Deactivate() {
			Close();
		}
	};

	One<PreviewPopup> popup;

	void SetImage(const Image& img) {
		preview_img = img;
		Refresh();
	}

	void SetPath(const String& path) {
		preview_img.Clear();
		if (!path.IsEmpty()) {
			String p = path;
			if (!FileExists(p)) {
				String base = GetCurrentDirectory();
				if (!base.IsEmpty()) {
					String alt = AppendFileName(base, path);
					if (FileExists(alt))
						p = alt;
				}
				if (!FileExists(p)) {
					String share = RealizeShareFile(path);
					if (FileExists(share))
						p = share;
				}
			}
			if (FileExists(p))
				preview_img = StreamRaster::LoadFileAny(p);
		}
		Refresh();
	}

	virtual void LeftDown(Point, dword) {
		if (preview_img.IsEmpty())
			return;
		if (popup && popup->IsOpen()) {
			popup->Close();
			return;
		}
		if (!popup)
			popup.Create();
		Size sz = preview_img.GetSize();
		const int max_dim = 256;
		if (sz.cx > max_dim || sz.cy > max_dim) {
			float scale = min((float)max_dim / (float)sz.cx, (float)max_dim / (float)sz.cy);
			sz = Size(max(1, (int)ceil(sz.cx * scale)), max(1, (int)ceil(sz.cy * scale)));
		}
		Image shown = (sz == preview_img.GetSize()) ? preview_img : CachedRescale(preview_img, sz, FILTER_BILINEAR);
		popup->SetImage(shown);
		Rect r = GetScreenRect();
		Rect wr = GetWorkArea();
		int x = r.left;
		int y = r.bottom + 2;
		if (y + sz.cy > wr.bottom)
			y = max(wr.top, r.top - sz.cy - 2);
		if (x + sz.cx > wr.right)
			x = max(wr.left, wr.right - sz.cx);
		popup->SetRect(x, y, sz.cx, sz.cy);
		popup->PopUp(this, true, false, GUI_DropShadows());
	}

	virtual void Paint(Draw& w) {
		Size sz = GetSize();
		w.DrawRect(sz, SColorPaper());
		if (preview_img.IsEmpty()) {
			w.DrawRect(0, 0, sz.cx, sz.cy, SColorFace());
			return;
		}
		Size isz = preview_img.GetSize();
		if (isz.cx <= 0 || isz.cy <= 0)
			return;
		float sx = (float)sz.cx / (float)isz.cx;
		float sy = (float)sz.cy / (float)isz.cy;
		float sc = min(sx, sy);
		Size tsz(max(1, (int)floor(isz.cx * sc)), max(1, (int)floor(isz.cy * sc)));
		Image res = (tsz == isz) ? preview_img : CachedRescale(preview_img, tsz, FILTER_BILINEAR);
		Point p((sz.cx - tsz.cx) / 2, (sz.cy - tsz.cy) / 2);
		w.DrawImage(p.x, p.y, res);
	}
};

}


GeomProjectCtrl::GeomProjectCtrl(Edit3D* e) {
	this->e = e;
	
	time.WhenCursor << THISBACK(OnCursor);
	time.WhenRowMenu = THISBACK(TimelineRowMenu);
	time.WhenRowSelect = THISBACK(TimelineRowSelect);
	time.WhenRowToggle = THISBACK(TimelineRowToggle);
	tree.WhenCursor << THISBACK(TreeSelect);
	tree.WhenMenu = THISBACK(TreeMenu);
	props.WhenBar = THISBACK(PropsMenu);
	
	
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
	grid.SetGridSize(2,2);
	for(int i = 0; i < 4; i++) {
		rends_v1[i].ctx = &e->render_ctx;
		rends_v2[i].ctx = &e->render_ctx;
		rends_v1[i].WhenChanged = THISBACK1(RefreshRenderer, i);
		rends_v2[i].WhenChanged = THISBACK1(RefreshRenderer, i);
		rends_v1[i].WhenMenu = THISBACK1(BuildViewMenu, i);
		rends_v2[i].WhenMenu = THISBACK1(BuildViewMenu, i);
		rends_v1[i].WhenInput = [=](const String& type, const Point& p, dword flags, int key) {
			e->DispatchInputEvent(type, p, flags, key, i);
		};
		rends_v2[i].WhenInput = [=](const String& type, const Point& p, dword flags, int key) {
			e->DispatchInputEvent(type, p, flags, key, i);
		};
	}
	rends_v1[0].SetViewMode(VIEWMODE_YZ);
	rends_v1[1].SetViewMode(VIEWMODE_XZ);
	rends_v1[2].SetViewMode(VIEWMODE_XY);
	rends_v1[3].SetViewMode(VIEWMODE_PERSPECTIVE);
	rends_v2[0].SetViewMode(VIEWMODE_YZ);
	rends_v2[1].SetViewMode(VIEWMODE_XZ);
	rends_v2[2].SetViewMode(VIEWMODE_XY);
	rends_v2[3].SetViewMode(VIEWMODE_PERSPECTIVE);
	rends_v1[0].SetCameraSource(CAMSRC_FOCUS);
	rends_v1[1].SetCameraSource(CAMSRC_FOCUS);
	rends_v1[2].SetCameraSource(CAMSRC_PROGRAM);
	rends_v1[3].SetCameraSource(CAMSRC_FOCUS);
	rends_v2[0].SetCameraSource(CAMSRC_FOCUS);
	rends_v2[1].SetCameraSource(CAMSRC_FOCUS);
	rends_v2[2].SetCameraSource(CAMSRC_PROGRAM);
	rends_v2[3].SetCameraSource(CAMSRC_FOCUS);
	for (int i = 0; i < 4; i++)
		rends_v2[i].SetWireframeOnly(true);
	for(int i = 0; i < 4; i++)
		rends[i] = &rends_v2[i];
	RebuildGrid();
	
	props.NoHeader();
	props_col_value = props.GetColumnCount();
	props.AddColumn("Value", 300).NoEdit();
	props_col_keyframe = props.GetColumnCount();
	props.AddColumn("", 72).NoEdit();
	props.ColumnAt(0).FixedWidth(180);
	props.ColumnAt(1).FixedWidth(300);
	props.ColumnAt(1).SetWidth(300);
	props.ColumnAt(2).FixedWidth(72);
	props.ColumnAt(2).SetWidth(72);
	for (int i = 3; i < props.GetColumnCount(); i++) {
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
	if (i >= 0 && i < 4 && rends[i])
		rends[i]->Refresh();
}

void GeomProjectCtrl::RefreshAll() {
	for (int i = 0; i < 4; i++)
		if (rends[i])
			rends[i]->Refresh();
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
			if (rends[i])
				rends[i]->Refresh();
		}
	}
}

void GeomProjectCtrl::Data() {
	GeomProject& prj = *e->prj;

	Index<const VfsValue*> open_nodes;
	Value cursor_value;
	int old_cursor = tree.GetCursor();
	if (old_cursor >= 0)
		cursor_value = tree.Get(old_cursor);
	for (int i = 0; i < tree.GetCount(); i++) {
		if (!tree.IsOpen(i))
			continue;
		Value v = tree.Get(i);
		if (v.Is<VfsValue*>())
			open_nodes.FindAdd(ValueTo<VfsValue*>(v));
	}
	
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
	for (int i = 0; i < tree.GetCount(); i++) {
		Value v = tree.Get(i);
		if (v.Is<VfsValue*>()) {
			if (open_nodes.Find(ValueTo<VfsValue*>(v)) >= 0)
				tree.Open(i);
		}
	}
	tree.Open(0);
	
	if (cursor_value.Is<VfsValue*>()) {
		VfsValue* target = ValueTo<VfsValue*>(cursor_value);
		for (int i = 0; i < tree.GetCount(); i++) {
			Value v = tree.Get(i);
			if (v.Is<VfsValue*>() && ValueTo<VfsValue*>(v) == target) {
				tree.SetCursor(i);
				break;
			}
		}
	}
	else if (cursor_value.Is<TreeNodeRef*>()) {
		TreeNodeRef* ref = ValueTo<TreeNodeRef*>(cursor_value);
		for (int i = 0; i < tree.GetCount(); i++) {
			TreeNodeRef* cur = GetNodeRef(tree.Get(i));
			if (cur && ref && cur->kind == ref->kind) {
				tree.SetCursor(i);
				break;
			}
		}
	}
	
	TreeSelect();
}

void GeomProjectCtrl::TreeSelect() {
	int cursor = tree.GetCursor();
	if (cursor < 0)
		return;
	Value v = tree.Get(cursor);
	e->selected_bone = nullptr;
	TreeNodeRef* ref = GetNodeRef(v);
	GeomObject* obj = GetNodeObject(v);
	GeomPointcloudDataset* ds = GetNodeDataset(v);
	selected_dataset = ds;
	if (!obj && v.Is<VfsValue*>()) {
		VfsValue* node = ValueTo<VfsValue*>(v);
		if (node && IsVfsType(*node, AsTypeHash<GeomBone>())) {
			e->selected_bone = node;
			for (VfsValue* p = node->owner; p; p = p->owner) {
				if (IsVfsType(*p, AsTypeHash<GeomObject>())) {
					obj = &p->GetExt<GeomObject>();
					break;
				}
			}
			if (obj) {
				e->state->focus_mode = 1;
				e->state->focus_object_key = obj->key;
			}
		}
	}
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
		e->state->UpdateObjects();
		RefreshAll();
		TimelineData();
		return;
	}
	if (obj) {
		e->timeline_scope = Edit3D::TS_OBJECT;
		e->timeline_component = Edit3D::TC_TRANSFORM;
		e->timeline_transform_field = Edit3D::TT_NONE;
		e->timeline_object_key = obj->key;
		e->state->focus_mode = 1;
		e->state->focus_object_key = obj->key;
		UpdateTreeFocus(cursor);
		PropsData();
		e->UpdateTextureEditor(obj);
		e->state->UpdateObjects();
		RefreshAll();
		TimelineData();
		return;
	}
	if (ds) {
		e->state->focus_mode = 0;
		e->state->focus_object_key = 0;
		UpdateTreeFocus(cursor);
		PropsData();
		e->state->UpdateObjects();
		RefreshAll();
		TimelineData();
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
				e->timeline_scope = Edit3D::TS_SCENE;
				e->timeline_component = Edit3D::TC_NONE;
				e->timeline_transform_field = Edit3D::TT_NONE;
				e->timeline_object_key = 0;
				e->state->active_scene = idx;
				e->state->UpdateObjects();
				RefreshAll();
				break;
			}
			idx++;
		}
	}
	else if (IsVfsType(*node, AsTypeHash<GeomDirectory>())) {
		e->state->focus_mode = 0;
		e->state->focus_object_key = 0;
		e->state->UpdateObjects();
		RefreshAll();
	}
	UpdateTreeFocus(cursor);
	PropsData();
	e->UpdateTextureEditor(obj);
	e->render_ctx.selected_bone = e->selected_bone;
	e->render_ctx.show_weights = e->weight_paint_mode;
	e->render_ctx.weight_bone.Clear();
	if (e->selected_bone && IsVfsType(*e->selected_bone, AsTypeHash<GeomBone>())) {
		GeomBone& bone = e->selected_bone->GetExt<GeomBone>();
		e->render_ctx.weight_bone = bone.name.IsEmpty() ? e->selected_bone->id : bone.name;
	}
	TimelineData();
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
	hash_t dataset_hash = TypedStringHasher<GeomPointcloudDataset>("GeomPointcloudDataset");
	if (!IsVfsType(*node, dataset_hash))
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
	if (cursor < 0 && focus_tree_id >= 0)
		cursor = focus_tree_id;
	if (cursor < 0) {
		int line0 = tree.GetLineCount() > 0 ? tree.GetItemAtLine(0) : -1;
		if (tree.IsValid(line0))
			cursor = line0;
	}
	if (cursor < 0)
		return;
	Value v = tree.Get(cursor);
	TreeNodeRef* ref = GetNodeRef(v);
	GeomObject* obj = GetNodeObject(v);
	VfsValue* node = v.Is<VfsValue*>() ? ValueTo<VfsValue*>(v) : 0;
	GeomDirectory* dir = 0;
	if (node && IsVfsType(*node, AsTypeHash<GeomDirectory>()))
		dir = &node->GetExt<GeomDirectory>();
	if (!dir && node && IsVfsType(*node, AsTypeHash<GeomScene>()))
		dir = &node->GetExt<GeomScene>();
	if (!dir && node && IsVfsType(*node, AsTypeHash<GeomObject>()) && node->owner) {
		if (IsVfsType(*node->owner, AsTypeHash<GeomDirectory>()))
			dir = &node->owner->GetExt<GeomDirectory>();
		else if (IsVfsType(*node->owner, AsTypeHash<GeomScene>()))
			dir = &node->owner->GetExt<GeomScene>();
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
			bar.Sub(t_("Preset"), [=](Bar& bar) {
				bar.Add(t_("Sphere"), [=] {
					String name = unique_name(*dir, "sphere");
					GeomObject& obj = dir->GetAddModel(name);
					ModelBuilder mb;
					mb.AddSphere(vec3(0, 0, 0), 1.0f, 24, 16);
					obj.mdl = mb.Detach();
					obj.asset_ref = "preset:sphere";
					e->state->UpdateObjects();
					e->RefreshData();
				});
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
	int cursor = tree.GetCursor();
	Value v = (cursor >= 0) ? tree.Get(cursor) : Value();
	GeomObject* obj = GetNodeObject(v);
	GeomPointcloudDataset* ds = GetNodeDataset(v);
	VfsValue* node = v.Is<VfsValue*>() ? ValueTo<VfsValue*>(v) : 0;
	GeomDirectory* dir = 0;
	GeomScene* scene = 0;
	if (node && IsVfsType(*node, AsTypeHash<GeomDirectory>()))
		dir = &node->GetExt<GeomDirectory>();
	if (node && IsVfsType(*node, AsTypeHash<GeomScene>()))
		scene = &node->GetExt<GeomScene>();
	if (!obj && !dir && !scene && !ds)
		return;
	bar.Sub(t_("Add Component"), [=](Bar& bar) {
		bar.Add(t_("Script"), [=] {
			if (obj)
				e->AddScriptComponent(*obj);
			else if (dir)
				e->AddScriptComponent(*dir);
			else if (scene)
				e->AddScriptComponent(*scene);
			e->state->UpdateObjects();
			e->RefreshData();
		});
		if (obj) {
			bar.Add(t_("Texture Edit"), [=] {
				obj->GetTextureEdit();
				e->UpdateTextureEditor(obj);
				e->state->UpdateObjects();
				e->RefreshData();
			});
		}
		if (obj && obj->IsOctree()) {
			bar.Add(t_("Pointcloud Effect (Transform)"), [=] {
				String base = "effect";
				String name = base;
				int idx = 1;
				Vector<GeomPointcloudEffectTransform*> effects;
				obj->GetPointcloudEffects(effects);
				auto exists = [&](const String& n) {
					for (GeomPointcloudEffectTransform* fx : effects) {
						if (!fx)
							continue;
						String fx_name = fx->name.IsEmpty() ? fx->val.id : fx->name;
						if (fx_name == n)
							return true;
					}
					return false;
				};
				while (exists(name))
					name = base + IntStr(idx++);
				obj->GetAddPointcloudEffect(name);
				e->state->UpdateObjects();
				e->RefreshData();
			});
		}
	});
}

void GeomProjectCtrl::PropsData() {
	if (props_refreshing)
		return;
	props_refreshing = true;
	props.Clear();
	props_nodes.Clear();
	props_ctrls.Clear();
	int cursor = tree.GetCursor();
	Value v = (cursor >= 0) ? tree.Get(cursor) : Value();
	selected_obj = GetNodeObject(v);
	GeomScene* selected_scene = 0;
	if (!selected_obj && v.Is<VfsValue*>()) {
		VfsValue* node = ValueTo<VfsValue*>(v);
		if (node && IsVfsType(*node, AsTypeHash<GeomScene>()))
			selected_scene = &node->GetExt<GeomScene>();
		if (node && IsVfsType(*node, AsTypeHash<GeomBone>())) {
			for (VfsValue* p = node->owner; p; p = p->owner) {
				if (IsVfsType(*p, AsTypeHash<GeomObject>())) {
					selected_obj = &p->GetExt<GeomObject>();
					break;
				}
			}
		}
	}
	selected_ref = GetNodeRef(v);
	selected_dataset = GetNodeDataset(v);
	props.SetRoot(ImagesImg::Root(), "Properties");
	int root = 0;
	int effects = -1;
	struct EffectItem {
		GeomPointcloudEffectTransform* fx = 0;
		int effect_id = -1;
		int pos_id = -1;
		int ori_id = -1;
	};
	Vector<EffectItem> effect_items;
	if (selected_obj && selected_obj->IsOctree()) {
		PropRef& efx = props_nodes.Add();
		efx.kind = PropRef::P_EFFECTS;
		effects = props.Add(root, ImagesImg::Directory(), RawToValue(&efx), t_("Effects"));
		Vector<GeomPointcloudEffectTransform*> fx_list;
		selected_obj->GetPointcloudEffects(fx_list);
		for (GeomPointcloudEffectTransform* fx : fx_list) {
			if (!fx)
				continue;
			String name = fx->name.IsEmpty() ? fx->val.id : fx->name;
			PropRef& fxnode = props_nodes.Add();
			fxnode.kind = PropRef::P_TRANSFORM;
			int fx_id = props.Add(effects, ImagesImg::Object(), RawToValue(&fxnode), name);
			PropRef& epos = props_nodes.Add();
			epos.kind = PropRef::P_POSITION;
			int pos_id = props.Add(fx_id, ImagesImg::Object(), RawToValue(&epos), t_("Position"));
			PropRef& eori = props_nodes.Add();
			eori.kind = PropRef::P_ORIENTATION;
			int ori_id = props.Add(fx_id, ImagesImg::Object(), RawToValue(&eori), t_("Orientation"));
			EffectItem& item = effect_items.Add();
			item.fx = fx;
			item.effect_id = fx_id;
			item.pos_id = pos_id;
			item.ori_id = ori_id;
		}
	}
	PropRef& comps = props_nodes.Add();
	comps.kind = PropRef::P_COMPONENTS;
	int components = props.Add(root, ImagesImg::Directory(), RawToValue(&comps), t_("Components"));
	int materials = -1;
	int meshes = -1;
	Model* model_ptr = 0;
	Geom2DLayer* layer_ptr = 0;
	int layer_material = -1;
	int layer_stroke_id = -1;
	int layer_fill_id = -1;
	int layer_width_id = -1;
	int layer_opacity_id = -1;
	int layer_style_id = -1;
	int layer_texture_id = -1;
	int layer_texture_preview_id = -1;
	int layer_blend_id = -1;
	int layer_tex_offset_x_id = -1;
	int layer_tex_offset_y_id = -1;
	int layer_tex_scale_x_id = -1;
	int layer_tex_scale_y_id = -1;
	int layer_tex_rotate_id = -1;
	if (selected_obj && selected_obj->IsModel() && selected_obj->mdl) {
		model_ptr = selected_obj->mdl.Get();
		PropRef& mats = props_nodes.Add();
		mats.kind = PropRef::P_MATERIALS;
		materials = props.Add(root, ImagesImg::Directory(), RawToValue(&mats), t_("Materials"));
		PropRef& meshref = props_nodes.Add();
		meshref.kind = PropRef::P_MESH_MATERIAL;
		meshes = props.Add(root, ImagesImg::Directory(), RawToValue(&meshref), t_("Meshes"));
	}
	if (selected_obj)
		layer_ptr = selected_obj->Find2DLayer();
	if (layer_ptr) {
		PropRef& lnode = props_nodes.Add();
		lnode.kind = PropRef::P_MATERIALS;
		layer_material = props.Add(root, ImagesImg::Directory(), RawToValue(&lnode), t_("2D Material"));
		PropRef& sref = props_nodes.Add();
		sref.kind = PropRef::P_MAT_BASE_COLOR;
		layer_stroke_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&sref), t_("Stroke"));
		PropRef& fref = props_nodes.Add();
		fref.kind = PropRef::P_MAT_EMISSIVE;
		layer_fill_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&fref), t_("Fill"));
		PropRef& wref = props_nodes.Add();
		wref.kind = PropRef::P_MAT_ROUGHNESS;
		layer_width_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&wref), t_("Width"));
		PropRef& oref = props_nodes.Add();
		oref.kind = PropRef::P_MAT_METALLIC;
		layer_opacity_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&oref), t_("Opacity"));
		PropRef& uref = props_nodes.Add();
		uref.kind = PropRef::P_MAT_OCCLUSION;
		layer_style_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&uref), t_("Use Layer Style"));
		PropRef& tref = props_nodes.Add();
		tref.kind = PropRef::P_MAT_NORMAL_SCALE;
		layer_texture_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&tref), t_("Texture"));
		PropRef& tpref = props_nodes.Add();
		tpref.kind = PropRef::P_MAT_NORMAL_SCALE;
		layer_texture_preview_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&tpref), t_("Texture Preview"));
		PropRef& bref = props_nodes.Add();
		bref.kind = PropRef::P_MAT_BASE_ALPHA;
		layer_blend_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&bref), t_("Blend Mode"));
		PropRef& toxref = props_nodes.Add();
		toxref.kind = PropRef::P_MAT_NORMAL_SCALE;
		layer_tex_offset_x_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&toxref), t_("Tex Offset X"));
		PropRef& toyref = props_nodes.Add();
		toyref.kind = PropRef::P_MAT_NORMAL_SCALE;
		layer_tex_offset_y_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&toyref), t_("Tex Offset Y"));
		PropRef& tsxref = props_nodes.Add();
		tsxref.kind = PropRef::P_MAT_NORMAL_SCALE;
		layer_tex_scale_x_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&tsxref), t_("Tex Scale X"));
		PropRef& tsyref = props_nodes.Add();
		tsyref.kind = PropRef::P_MAT_NORMAL_SCALE;
		layer_tex_scale_y_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&tsyref), t_("Tex Scale Y"));
		PropRef& trotref = props_nodes.Add();
		trotref.kind = PropRef::P_MAT_NORMAL_SCALE;
		layer_tex_rotate_id = props.Add(layer_material, ImagesImg::Object(), RawToValue(&trotref), t_("Tex Rotate"));
	}
	int scene_timeline = -1;
	int scene_tl_length_id = -1;
	int scene_tl_pos_id = -1;
	int scene_tl_play_id = -1;
	int scene_tl_repeat_id = -1;
	int scene_tl_speed_id = -1;
	GeomSceneTimeline* scene_tl_ptr = 0;
	if (selected_scene) {
		scene_tl_ptr = &selected_scene->GetTimeline();
		PropRef& tl = props_nodes.Add();
		tl.kind = PropRef::P_SCENE_TIMELINE;
		scene_timeline = props.Add(root, ImagesImg::Directory(), RawToValue(&tl), t_("Scene Timeline"));
		PropRef& len = props_nodes.Add();
		len.kind = PropRef::P_SCENE_TIMELINE_LENGTH;
		scene_tl_length_id = props.Add(scene_timeline, ImagesImg::Object(), RawToValue(&len), t_("Length"));
		PropRef& pos = props_nodes.Add();
		pos.kind = PropRef::P_SCENE_TIMELINE_POSITION;
		scene_tl_pos_id = props.Add(scene_timeline, ImagesImg::Object(), RawToValue(&pos), t_("Position"));
		PropRef& play = props_nodes.Add();
		play.kind = PropRef::P_SCENE_TIMELINE_PLAY;
		scene_tl_play_id = props.Add(scene_timeline, ImagesImg::Object(), RawToValue(&play), t_("Play"));
		PropRef& rep = props_nodes.Add();
		rep.kind = PropRef::P_SCENE_TIMELINE_REPEAT;
		scene_tl_repeat_id = props.Add(scene_timeline, ImagesImg::Object(), RawToValue(&rep), t_("Repeat"));
		PropRef& speed = props_nodes.Add();
		speed.kind = PropRef::P_SCENE_TIMELINE_SPEED;
		scene_tl_speed_id = props.Add(scene_timeline, ImagesImg::Object(), RawToValue(&speed), t_("Speed"));
	}
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
	struct MaterialItemIds {
		Material* mat = 0;
		int mat_node_id = -1;
		int base_id = -1;
		int alpha_id = -1;
		int metallic_id = -1;
		int roughness_id = -1;
		int emissive_id = -1;
		int normal_id = -1;
		int occlusion_id = -1;
	};
	Vector<MaterialItemIds> material_items;
	Vector<int> mesh_rows;
	if (model_ptr && materials >= 0) {
		for (int i = 0; i < model_ptr->materials.GetCount(); i++) {
			int mat_id = model_ptr->materials.GetKey(i);
			Material& mat = model_ptr->materials[i];
			String name = "Material #" + IntStr(mat_id);
			PropRef& mnode = props_nodes.Add();
			mnode.kind = PropRef::P_MATERIAL;
			mnode.material_id = mat_id;
			int mat_node = props.Add(materials, ImagesImg::Object(), RawToValue(&mnode), name);
			PropRef& bnode = props_nodes.Add();
			bnode.kind = PropRef::P_MAT_BASE_COLOR;
			bnode.material_id = mat_id;
			int base_id = props.Add(mat_node, ImagesImg::Object(), RawToValue(&bnode), t_("Base Color"));
			PropRef& anode = props_nodes.Add();
			anode.kind = PropRef::P_MAT_BASE_ALPHA;
			anode.material_id = mat_id;
			int alpha_id = props.Add(mat_node, ImagesImg::Object(), RawToValue(&anode), t_("Alpha"));
			PropRef& mtnode = props_nodes.Add();
			mtnode.kind = PropRef::P_MAT_METALLIC;
			mtnode.material_id = mat_id;
			int metallic_id = props.Add(mat_node, ImagesImg::Object(), RawToValue(&mtnode), t_("Metallic"));
			PropRef& rnode = props_nodes.Add();
			rnode.kind = PropRef::P_MAT_ROUGHNESS;
			rnode.material_id = mat_id;
			int roughness_id = props.Add(mat_node, ImagesImg::Object(), RawToValue(&rnode), t_("Roughness"));
			PropRef& enode = props_nodes.Add();
			enode.kind = PropRef::P_MAT_EMISSIVE;
			enode.material_id = mat_id;
			int emissive_id = props.Add(mat_node, ImagesImg::Object(), RawToValue(&enode), t_("Emissive"));
			PropRef& nnode = props_nodes.Add();
			nnode.kind = PropRef::P_MAT_NORMAL_SCALE;
			nnode.material_id = mat_id;
			int normal_id = props.Add(mat_node, ImagesImg::Object(), RawToValue(&nnode), t_("Normal Scale"));
			PropRef& ocnode = props_nodes.Add();
			ocnode.kind = PropRef::P_MAT_OCCLUSION;
			ocnode.material_id = mat_id;
			int occlusion_id = props.Add(mat_node, ImagesImg::Object(), RawToValue(&ocnode), t_("Occlusion"));
			MaterialItemIds& ids = material_items.Add();
			ids.mat = &mat;
			ids.mat_node_id = mat_node;
			ids.base_id = base_id;
			ids.alpha_id = alpha_id;
			ids.metallic_id = metallic_id;
			ids.roughness_id = roughness_id;
			ids.emissive_id = emissive_id;
			ids.normal_id = normal_id;
			ids.occlusion_id = occlusion_id;
		}
	}
	if (model_ptr && meshes >= 0) {
		for (int i = 0; i < model_ptr->meshes.GetCount(); i++) {
			PropRef& mrow = props_nodes.Add();
			mrow.kind = PropRef::P_MESH_MATERIAL;
			mrow.mesh_index = i;
			String name = "Mesh #" + IntStr(i);
			int row_id = props.Add(meshes, ImagesImg::Object(), RawToValue(&mrow), name);
			mesh_rows.Add(row_id);
		}
	}

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
	if (selected_obj) {
		for (auto& sub : selected_obj->val.sub) {
			hash_t script_hash = TypedStringHasher<GeomScript>("GeomScript");
			if (!IsVfsType(sub, script_hash))
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
	if (!selected_obj) {
		int cursor = tree.GetCursor();
		Value v = (cursor >= 0) ? tree.Get(cursor) : Value();
		VfsValue* node = v.Is<VfsValue*>() ? ValueTo<VfsValue*>(v) : 0;
		if (node && (IsVfsType(*node, AsTypeHash<GeomDirectory>()) || IsVfsType(*node, AsTypeHash<GeomScene>()))) {
			for (auto& sub : node->sub) {
				hash_t script_hash = TypedStringHasher<GeomScript>("GeomScript");
				if (!IsVfsType(sub, script_hash))
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
	}
	if (effects >= 0)
		props.Open(effects);
	props.Open(components);
	if (materials >= 0)
		props.Open(materials);
	if (meshes >= 0)
		props.Open(meshes);
	if (layer_material >= 0)
		props.Open(layer_material);
	if (pointcloud >= 0)
		props.Open(pointcloud);
	if (scene_timeline >= 0)
		props.Open(scene_timeline);

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
	
	auto set_ctrl_col = [&](int id, int col, One<Ctrl>&& c) {
		int line = props.GetLineAtItem(id);
		if (line >= 0)
			props.SetCtrl(line, col, *props_ctrls.Add(pick(c)), false);
	};
	auto set_value_ctrl = [&](int id, One<Ctrl>&& c) {
		set_ctrl_col(id, props_col_value, pick(c));
	};
	auto set_key_ctrl = [&](int id, One<Ctrl>&& c) {
		if (props_col_keyframe >= 0)
			set_ctrl_col(id, props_col_keyframe, pick(c));
	};

	auto clamp01 = [](double v) {
		if (v < 0.0) return 0.0;
		if (v > 1.0) return 1.0;
		return v;
	};
	auto to_color = [&](const vec3& v) -> Color {
		int r = (int)Clamp(v[0] * 255.0f, 0.0f, 255.0f);
		int g = (int)Clamp(v[1] * 255.0f, 0.0f, 255.0f);
		int b = (int)Clamp(v[2] * 255.0f, 0.0f, 255.0f);
		return Color(r, g, b);
	};
	auto update_material = [&](Material* mat, const Function<void(MaterialParameters&)>& fn) {
		if (!mat)
			return;
		mat->params.Set([&](MaterialParameters& p) { fn(p); });
		RefreshAll();
	};
	for (const MaterialItemIds& ids : material_items) {
		Material* mat = ids.mat;
		if (!mat)
			continue;
		if (ids.mat_node_id >= 0) {
			One<MaterialPreviewCtrl> prev = MakeOne<MaterialPreviewCtrl>();
			prev->SetMaterial(*mat);
			set_value_ctrl(ids.mat_node_id, pick(prev));
		}
		vec4 base = mat->params->base_clr_factor;
		vec3 emissive = mat->params->emissive_factor;
		One<ColorPusher> base_ctrl = MakeOne<ColorPusher>();
		base_ctrl->SetData(to_color(base.Splice()));
		ColorPusher* base_ptr = base_ctrl.Get();
		base_ctrl->WhenAction << [=] {
			if (!base_ptr || !mat)
				return;
			RGBA c = (Color)base_ptr->GetData();
			update_material(mat, [&](MaterialParameters& p) {
				p.base_clr_factor = vec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, p.base_clr_factor[3]);
			});
		};
		set_value_ctrl(ids.base_id, pick(base_ctrl));

		One<EditDoubleSpin> alpha_ctrl = MakeOne<EditDoubleSpin>();
		alpha_ctrl->Min(0).Max(1);
		alpha_ctrl->SetData(base[3]);
		EditDoubleSpin* alpha_ptr = alpha_ctrl.Get();
		alpha_ctrl->WhenAction << [=] {
			if (!alpha_ptr || !mat)
				return;
			double a = clamp01((double)~*alpha_ptr);
			update_material(mat, [&](MaterialParameters& p) {
				p.base_clr_factor[3] = a;
			});
		};
		set_value_ctrl(ids.alpha_id, pick(alpha_ctrl));

		One<EditDoubleSpin> metallic_ctrl = MakeOne<EditDoubleSpin>();
		metallic_ctrl->Min(0).Max(1);
		metallic_ctrl->SetData(mat->params->metallic_factor);
		EditDoubleSpin* metallic_ptr = metallic_ctrl.Get();
		metallic_ctrl->WhenAction << [=] {
			if (!metallic_ptr || !mat)
				return;
			double v = clamp01((double)~*metallic_ptr);
			update_material(mat, [&](MaterialParameters& p) { p.metallic_factor = v; });
		};
		set_value_ctrl(ids.metallic_id, pick(metallic_ctrl));

		One<EditDoubleSpin> rough_ctrl = MakeOne<EditDoubleSpin>();
		rough_ctrl->Min(0).Max(1);
		rough_ctrl->SetData(mat->params->roughness_factor);
		EditDoubleSpin* rough_ptr = rough_ctrl.Get();
		rough_ctrl->WhenAction << [=] {
			if (!rough_ptr || !mat)
				return;
			double v = clamp01((double)~*rough_ptr);
			update_material(mat, [&](MaterialParameters& p) { p.roughness_factor = v; });
		};
		set_value_ctrl(ids.roughness_id, pick(rough_ctrl));

		One<ColorPusher> emissive_ctrl = MakeOne<ColorPusher>();
		emissive_ctrl->SetData(to_color(emissive));
		ColorPusher* emissive_ptr = emissive_ctrl.Get();
		emissive_ctrl->WhenAction << [=] {
			if (!emissive_ptr || !mat)
				return;
			RGBA c = (Color)emissive_ptr->GetData();
			update_material(mat, [&](MaterialParameters& p) {
				p.emissive_factor = vec3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);
			});
		};
		set_value_ctrl(ids.emissive_id, pick(emissive_ctrl));

		One<EditDoubleSpin> normal_ctrl = MakeOne<EditDoubleSpin>();
		normal_ctrl->Min(0).Max(4);
		normal_ctrl->SetData(mat->params->normal_scale);
		EditDoubleSpin* normal_ptr = normal_ctrl.Get();
		normal_ctrl->WhenAction << [=] {
			if (!normal_ptr || !mat)
				return;
			double v = max(0.0, (double)~*normal_ptr);
			update_material(mat, [&](MaterialParameters& p) { p.normal_scale = v; });
		};
		set_value_ctrl(ids.normal_id, pick(normal_ctrl));

		One<EditDoubleSpin> occ_ctrl = MakeOne<EditDoubleSpin>();
		occ_ctrl->Min(0).Max(1);
		occ_ctrl->SetData(mat->params->occlusion_strength);
		EditDoubleSpin* occ_ptr = occ_ctrl.Get();
		occ_ctrl->WhenAction << [=] {
			if (!occ_ptr || !mat)
				return;
			double v = clamp01((double)~*occ_ptr);
			update_material(mat, [&](MaterialParameters& p) { p.occlusion_strength = v; });
		};
		set_value_ctrl(ids.occlusion_id, pick(occ_ctrl));
	}
	if (model_ptr && meshes >= 0) {
		for (int i = 0; i < mesh_rows.GetCount() && i < model_ptr->meshes.GetCount(); i++) {
			int row_id = mesh_rows[i];
			Mesh* mesh_ptr = &model_ptr->meshes[i];
			One<DropList> dl = MakeOne<DropList>();
			dl->Add(-1, t_("None"));
			for (int k = 0; k < model_ptr->materials.GetCount(); k++) {
				int mat_id = model_ptr->materials.GetKey(k);
				dl->Add(mat_id, "Material #" + IntStr(mat_id));
			}
			dl->SetData(mesh_ptr ? mesh_ptr->material : -1);
			DropList* dl_ptr = dl.Get();
			dl->WhenAction << [=] {
				if (dl_ptr && mesh_ptr) {
					mesh_ptr->material = (int)dl_ptr->GetData();
					RefreshAll();
				}
			};
			set_value_ctrl(row_id, pick(dl));
		}
	}
	if (layer_ptr && layer_material >= 0) {
		One<ColorPusher> stroke_ctrl = MakeOne<ColorPusher>();
		stroke_ctrl->SetData(layer_ptr->stroke);
		ColorPusher* stroke_ptr = stroke_ctrl.Get();
		stroke_ctrl->WhenAction << [=] {
			if (stroke_ptr && layer_ptr) {
				layer_ptr->stroke = (Color)stroke_ptr->GetData();
				RefreshAll();
			}
		};
		set_value_ctrl(layer_stroke_id, pick(stroke_ctrl));

		One<ColorPusher> fill_ctrl = MakeOne<ColorPusher>();
		fill_ctrl->SetData(layer_ptr->fill);
		ColorPusher* fill_ptr = fill_ctrl.Get();
		fill_ctrl->WhenAction << [=] {
			if (fill_ptr && layer_ptr) {
				layer_ptr->fill = (Color)fill_ptr->GetData();
				RefreshAll();
			}
		};
		set_value_ctrl(layer_fill_id, pick(fill_ctrl));

		One<EditDoubleSpin> width_ctrl = MakeOne<EditDoubleSpin>();
		width_ctrl->Min(0).Max(20);
		width_ctrl->SetData(layer_ptr->width);
		EditDoubleSpin* width_ptr = width_ctrl.Get();
		width_ctrl->WhenAction << [=] {
			if (width_ptr && layer_ptr) {
				layer_ptr->width = max(0.0, (double)~*width_ptr);
				RefreshAll();
			}
		};
		set_value_ctrl(layer_width_id, pick(width_ctrl));

		One<EditDoubleSpin> opacity_ctrl = MakeOne<EditDoubleSpin>();
		opacity_ctrl->Min(0).Max(1);
		opacity_ctrl->SetData(layer_ptr->opacity);
		EditDoubleSpin* opacity_ptr = opacity_ctrl.Get();
		opacity_ctrl->WhenAction << [=] {
			if (opacity_ptr && layer_ptr) {
				layer_ptr->opacity = clamp01((double)~*opacity_ptr);
				RefreshAll();
			}
		};
		set_value_ctrl(layer_opacity_id, pick(opacity_ctrl));

		One<Option> style_ctrl = MakeOne<Option>();
		style_ctrl->SetData(layer_ptr->use_layer_style);
		Option* style_ptr = style_ctrl.Get();
		style_ctrl->WhenAction << [=] {
			if (style_ptr && layer_ptr) {
				layer_ptr->use_layer_style = (bool)style_ptr->GetData();
				RefreshAll();
			}
		};
		set_value_ctrl(layer_style_id, pick(style_ctrl));

		One<EditString> tex_ctrl = MakeOne<EditString>();
		tex_ctrl->SetData(layer_ptr->texture_ref);
		EditString* tex_ptr = tex_ctrl.Get();
		One<TexturePreviewCtrl> tex_preview = MakeOne<TexturePreviewCtrl>();
		tex_preview->SetPath(layer_ptr->texture_ref);
		TexturePreviewCtrl* tex_preview_ptr = tex_preview.Get();
		tex_ctrl->WhenAction << [=] {
			if (tex_ptr && layer_ptr) {
				layer_ptr->texture_ref = ~*tex_ptr;
				if (tex_preview_ptr)
					tex_preview_ptr->SetPath(layer_ptr->texture_ref);
			}
		};
		set_value_ctrl(layer_texture_id, pick(tex_ctrl));
		set_value_ctrl(layer_texture_preview_id, pick(tex_preview));

		One<DropList> blend_ctrl = MakeOne<DropList>();
		blend_ctrl->Add(0, t_("Normal"));
		blend_ctrl->Add(1, t_("Add"));
		blend_ctrl->Add(2, t_("Multiply"));
		blend_ctrl->SetData(layer_ptr->blend_mode);
		DropList* blend_ptr = blend_ctrl.Get();
		blend_ctrl->WhenAction << [=] {
			if (blend_ptr && layer_ptr) {
				layer_ptr->blend_mode = (int)blend_ptr->GetData();
				RefreshAll();
			}
		};
		set_value_ctrl(layer_blend_id, pick(blend_ctrl));

		One<EditDoubleSpin> tex_off_x = MakeOne<EditDoubleSpin>();
		tex_off_x->Min(-10).Max(10);
		tex_off_x->SetData(layer_ptr->tex_offset_x);
		EditDoubleSpin* tex_off_x_ptr = tex_off_x.Get();
		tex_off_x->WhenAction << [=] {
			if (tex_off_x_ptr && layer_ptr) {
				layer_ptr->tex_offset_x = (double)~*tex_off_x_ptr;
				RefreshAll();
			}
		};
		set_value_ctrl(layer_tex_offset_x_id, pick(tex_off_x));

		One<EditDoubleSpin> tex_off_y = MakeOne<EditDoubleSpin>();
		tex_off_y->Min(-10).Max(10);
		tex_off_y->SetData(layer_ptr->tex_offset_y);
		EditDoubleSpin* tex_off_y_ptr = tex_off_y.Get();
		tex_off_y->WhenAction << [=] {
			if (tex_off_y_ptr && layer_ptr) {
				layer_ptr->tex_offset_y = (double)~*tex_off_y_ptr;
				RefreshAll();
			}
		};
		set_value_ctrl(layer_tex_offset_y_id, pick(tex_off_y));

		One<EditDoubleSpin> tex_scale_x = MakeOne<EditDoubleSpin>();
		tex_scale_x->Min(0.01).Max(10);
		tex_scale_x->SetData(layer_ptr->tex_scale_x);
		EditDoubleSpin* tex_scale_x_ptr = tex_scale_x.Get();
		tex_scale_x->WhenAction << [=] {
			if (tex_scale_x_ptr && layer_ptr) {
				layer_ptr->tex_scale_x = max(0.01, (double)~*tex_scale_x_ptr);
				RefreshAll();
			}
		};
		set_value_ctrl(layer_tex_scale_x_id, pick(tex_scale_x));

		One<EditDoubleSpin> tex_scale_y = MakeOne<EditDoubleSpin>();
		tex_scale_y->Min(0.01).Max(10);
		tex_scale_y->SetData(layer_ptr->tex_scale_y);
		EditDoubleSpin* tex_scale_y_ptr = tex_scale_y.Get();
		tex_scale_y->WhenAction << [=] {
			if (tex_scale_y_ptr && layer_ptr) {
				layer_ptr->tex_scale_y = max(0.01, (double)~*tex_scale_y_ptr);
				RefreshAll();
			}
		};
		set_value_ctrl(layer_tex_scale_y_id, pick(tex_scale_y));

		One<EditDoubleSpin> tex_rotate = MakeOne<EditDoubleSpin>();
		tex_rotate->Min(-360).Max(360);
		tex_rotate->SetData(layer_ptr->tex_rotate);
		EditDoubleSpin* tex_rotate_ptr = tex_rotate.Get();
		tex_rotate->WhenAction << [=] {
			if (tex_rotate_ptr && layer_ptr) {
				layer_ptr->tex_rotate = (double)~*tex_rotate_ptr;
				RefreshAll();
			}
		};
		set_value_ctrl(layer_tex_rotate_id, pick(tex_rotate));
	}
	
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
		set_value_ctrl(root, pick(t));
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
			set_value_ctrl(effects, pick(tfx));
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
		set_value_ctrl(dataset_id, pick(ds_ctrl));
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
		set_value_ctrl(dataset_name_id, pick(name_ctrl));

		One<EditString> source_ctrl = MakeOne<EditString>();
		source_ctrl->SetData(props_dataset->source_ref);
		EditString* source_ptr = source_ctrl.Get();
		source_ctrl->WhenAction << [=] {
			if (source_ptr)
				props_dataset->source_ref = ~*source_ptr;
			e->state->UpdateObjects();
			e->RefreshData();
		};
		set_value_ctrl(dataset_source_id, pick(source_ctrl));
	}

	for (const EffectItem& item : effect_items) {
		GeomPointcloudEffectTransform* fx_ptr = item.fx;
		if (!fx_ptr)
			continue;
		One<ToggleRowCtrl> tfx = MakeOne<ToggleRowCtrl>();
		tfx->SetVisible(fx_ptr->enabled);
		tfx->SetLocked(fx_ptr->locked);
		tfx->WhenVisible << [=](bool v) {
			if (!fx_ptr)
				return;
			fx_ptr->enabled = v;
			RefreshAll();
		};
		tfx->WhenLocked << [=](bool v) {
			if (!fx_ptr)
				return;
			fx_ptr->locked = v;
		};
		if (item.effect_id >= 0)
			set_value_ctrl(item.effect_id, pick(tfx));
		
		One<Vec3EditCtrl> fpos = MakeOne<Vec3EditCtrl>();
		fpos->SetValue(fx_ptr->position);
		fpos->SetEditable(!fx_ptr->locked);
		Vec3EditCtrl* fpos_ptr = fpos.Get();
		fpos->WhenAction << [=] {
			if (!fx_ptr || !fpos_ptr)
				return;
			fx_ptr->position = fpos_ptr->GetValue();
			RefreshAll();
		};
		set_value_ctrl(item.pos_id, pick(fpos));
		
		One<QuatEditCtrl> fori = MakeOne<QuatEditCtrl>();
		fori->SetValue(fx_ptr->orientation);
		fori->SetEditable(!fx_ptr->locked);
		QuatEditCtrl* fori_ptr = fori.Get();
		fori->WhenAction << [=] {
			if (!fx_ptr || !fori_ptr)
				return;
			fx_ptr->orientation = fori_ptr->GetValue();
			RefreshAll();
		};
		set_value_ctrl(item.ori_id, pick(fori));
	}

	for (const ScriptItemIds& ids : script_items) {
		GeomScript* script_ptr = ids.script;
		if (!script_ptr)
			continue;
		One<EditString> file_ctrl = MakeOne<EditString>();
		file_ctrl->SetData(script_ptr->file);
		EditString* file_ctrl_ptr = file_ctrl.Get();
		file_ctrl->WhenAction << [=] {
			if (file_ctrl_ptr && script_ptr)
				script_ptr->file = ~*file_ctrl_ptr;
			e->EnsureScriptInstances();
		};
		set_value_ctrl(ids.file_id, pick(file_ctrl));

		One<Option> enabled = MakeOne<Option>();
		enabled->SetData(script_ptr->enabled);
		Option* enabled_ptr = enabled.Get();
		enabled->WhenAction << [=] {
			if (enabled_ptr && script_ptr)
				script_ptr->enabled = (bool)enabled_ptr->GetData();
			e->EnsureScriptInstances();
		};
		set_value_ctrl(ids.enabled_id, pick(enabled));

		One<Option> run_on_load = MakeOne<Option>();
		run_on_load->SetData(script_ptr->run_on_load);
		Option* run_on_load_ptr = run_on_load.Get();
		run_on_load->WhenAction << [=] {
			if (run_on_load_ptr && script_ptr)
				script_ptr->run_on_load = (bool)run_on_load_ptr->GetData();
		};
		set_value_ctrl(ids.run_on_load_id, pick(run_on_load));

		One<Option> run_frame = MakeOne<Option>();
		run_frame->SetData(script_ptr->run_every_frame);
		Option* run_frame_ptr = run_frame.Get();
		run_frame->WhenAction << [=] {
			if (run_frame_ptr && script_ptr)
				script_ptr->run_every_frame = (bool)run_frame_ptr->GetData();
		};
		set_value_ctrl(ids.run_frame_id, pick(run_frame));

		One<Button> edit = MakeOne<Button>();
		edit->SetLabel(t_("Edit..."));
		edit->WhenAction << [=] {
			if (script_ptr)
				e->OpenScriptEditor(*script_ptr);
		};
		set_value_ctrl(ids.edit_id, pick(edit));

		One<Button> run = MakeOne<Button>();
		run->SetLabel(t_("Run"));
		run->WhenAction << [=] {
			if (script_ptr)
				e->RunScriptOnce(*script_ptr);
		};
		set_value_ctrl(ids.run_id, pick(run));
	}

	if (scene_tl_ptr) {
		One<EditIntSpin> len_ctrl = MakeOne<EditIntSpin>();
		len_ctrl->Min(0);
		len_ctrl->SetData(scene_tl_ptr->length);
		EditIntSpin* len_ptr = len_ctrl.Get();
		len_ctrl->WhenAction << [=] {
			if (!len_ptr || !scene_tl_ptr || !selected_scene)
				return;
			int len = (int)~*len_ptr;
			if (len < 0)
				len = 0;
			scene_tl_ptr->length = len;
			selected_scene->length = len;
			if (scene_tl_ptr->position >= len && len > 0)
				scene_tl_ptr->position = len - 1;
			scene_tl_ptr->time = scene_tl_ptr->position / (double)e->prj->kps;
			RefreshAll();
		};
		set_value_ctrl(scene_tl_length_id, pick(len_ctrl));

		One<EditIntSpin> pos_ctrl_tl = MakeOne<EditIntSpin>();
		pos_ctrl_tl->Min(0);
		pos_ctrl_tl->SetData(scene_tl_ptr->position);
		EditIntSpin* pos_ptr = pos_ctrl_tl.Get();
		pos_ctrl_tl->WhenAction << [=] {
			if (!pos_ptr || !scene_tl_ptr)
				return;
			int pos = (int)~*pos_ptr;
			if (pos < 0)
				pos = 0;
			if (scene_tl_ptr->length > 0 && pos >= scene_tl_ptr->length)
				pos = scene_tl_ptr->length - 1;
			scene_tl_ptr->position = pos;
			scene_tl_ptr->time = pos / (double)e->prj->kps;
			RefreshAll();
		};
		set_value_ctrl(scene_tl_pos_id, pick(pos_ctrl_tl));

		One<Option> play_ctrl = MakeOne<Option>();
		play_ctrl->SetData(scene_tl_ptr->is_playing);
		Option* play_ptr = play_ctrl.Get();
		play_ctrl->WhenAction << [=] {
			if (!play_ptr || !scene_tl_ptr || !selected_scene)
				return;
			bool on = (bool)play_ptr->GetData();
			if (on)
				scene_tl_ptr->Play(selected_scene->length);
			else
				scene_tl_ptr->Pause();
			RefreshAll();
		};
		set_value_ctrl(scene_tl_play_id, pick(play_ctrl));

		One<Option> repeat_ctrl = MakeOne<Option>();
		repeat_ctrl->SetData(scene_tl_ptr->repeat);
		Option* repeat_ptr = repeat_ctrl.Get();
		repeat_ctrl->WhenAction << [=] {
			if (repeat_ptr && scene_tl_ptr)
				scene_tl_ptr->repeat = (bool)repeat_ptr->GetData();
		};
		set_value_ctrl(scene_tl_repeat_id, pick(repeat_ctrl));

		One<EditDoubleSpin> speed_ctrl = MakeOne<EditDoubleSpin>();
		speed_ctrl->Min(0.01);
		speed_ctrl->SetData(scene_tl_ptr->speed);
		EditDoubleSpin* speed_ptr = speed_ctrl.Get();
		speed_ctrl->WhenAction << [=] {
			if (speed_ptr && scene_tl_ptr) {
				double v = (double)~*speed_ptr;
				if (v < 0.01)
					v = 0.01;
				scene_tl_ptr->speed = v;
			}
		};
		set_value_ctrl(scene_tl_speed_id, pick(speed_ctrl));
	}
	
	One<Vec3EditCtrl> pos_ctrl = MakeOne<Vec3EditCtrl>();
	pos_ctrl->SetValue(pos);
	pos_ctrl->SetEditable(true);
	pos_ctrl->WhenAction << [=] {
		props.SetCursor(pos_id);
		PropsApply();
	};
	set_value_ctrl(pos_id, pick(pos_ctrl));
	
	One<QuatEditCtrl> ori_ctrl = MakeOne<QuatEditCtrl>();
	ori_ctrl->SetValue(ori);
	ori_ctrl->SetEditable(true);
	ori_ctrl->WhenAction << [=] {
		props.SetCursor(ori_id);
		PropsApply();
	};
	set_value_ctrl(ori_id, pick(ori_ctrl));

	auto set_frame = [&](int frame) {
		if (!e || !e->anim)
			return;
		int max_frame = max(e->state->GetActiveScene().length - 1, 0);
		frame = min(max(frame, 0), max_frame);
		e->anim->position = frame;
		time.SetSelectedColumn(frame);
		time.Refresh();
		RefreshAll();
		PostCallback([=] { PropsData(); });
	};
	auto find_prev_frame = [&](GeomObject* obj, bool pos, bool ori, int frame) -> int {
		if (!obj)
			return 0;
		GeomTimeline* tl = obj->FindTimeline();
		if (!tl || tl->keypoints.IsEmpty())
			return 0;
		int idx = pos ? tl->FindPrePosition(frame) : tl->FindPreOrientation(frame);
		if (!pos && !ori)
			idx = tl->FindPre(frame);
		if (idx < 0)
			return 0;
		return tl->keypoints.GetKey(idx);
	};
	auto find_next_frame = [&](GeomObject* obj, bool pos, bool ori, int frame) -> int {
		int last = max(e->state->GetActiveScene().length - 1, 0);
		if (!obj)
			return last;
		GeomTimeline* tl = obj->FindTimeline();
		if (!tl || tl->keypoints.IsEmpty())
			return last;
		int idx = pos ? tl->FindPostPosition(frame) : tl->FindPostOrientation(frame);
		if (!pos && !ori)
			idx = tl->FindPost(frame);
		if (idx < 0)
			return last;
		return tl->keypoints.GetKey(idx);
	};
	auto has_kp_at = [&](GeomObject* obj, bool pos, bool ori, int frame) -> bool {
		if (!obj)
			return false;
		GeomTimeline* tl = obj->FindTimeline();
		if (!tl)
			return false;
		int idx = tl->keypoints.Find(frame);
		if (idx < 0)
			return false;
		GeomKeypoint& kp = tl->keypoints[idx];
		if (pos && !kp.has_position)
			return false;
		if (ori && !kp.has_orientation)
			return false;
		return true;
	};
	auto toggle_kp = [&](GeomObject* obj, bool pos, bool ori, int frame) {
		if (!obj)
			return;
		GeomTimeline& tl = obj->GetTimeline();
		int idx = tl.keypoints.Find(frame);
		if (idx >= 0) {
			GeomKeypoint& kp = tl.keypoints[idx];
			if (pos) kp.has_position = !kp.has_position;
			if (ori) kp.has_orientation = !kp.has_orientation;
			if (pos && kp.has_position && obj->FindTransform())
				kp.position = obj->FindTransform()->position;
			if (ori && kp.has_orientation && obj->FindTransform())
				kp.orientation = obj->FindTransform()->orientation;
			if (!kp.has_position && !kp.has_orientation)
				tl.keypoints.Remove(idx);
		}
		else {
			GeomKeypoint& kp = tl.GetAddKeypoint(frame);
			kp.frame_id = frame;
			if (GeomTransform* tr = obj->FindTransform()) {
				if (pos) kp.position = tr->position;
				if (ori) kp.orientation = tr->orientation;
			}
			kp.has_position = pos;
			kp.has_orientation = ori;
		}
		obj->read_enabled = true;
		tree.SetRowValue(focus_tree_id, tree_col_read, obj->read_enabled);
		TimelineData();
		time.Refresh();
		RefreshAll();
		PostCallback([=] { PropsData(); });
	};

	if (selected_obj) {
		int frame = e->anim ? e->anim->position : 0;
		One<KeyframeCtrl> kp_pos = MakeOne<KeyframeCtrl>();
		kp_pos->SetKeyed(has_kp_at(selected_obj, true, false, frame));
		kp_pos->WhenPrev = [=] { set_frame(find_prev_frame(selected_obj, true, false, frame)); };
		kp_pos->WhenNext = [=] { set_frame(find_next_frame(selected_obj, true, false, frame)); };
		kp_pos->WhenToggle = [=] { toggle_kp(selected_obj, true, false, frame); };
		set_key_ctrl(pos_id, pick(kp_pos));

		One<KeyframeCtrl> kp_ori = MakeOne<KeyframeCtrl>();
		kp_ori->SetKeyed(has_kp_at(selected_obj, false, true, frame));
		kp_ori->WhenPrev = [=] { set_frame(find_prev_frame(selected_obj, false, true, frame)); };
		kp_ori->WhenNext = [=] { set_frame(find_next_frame(selected_obj, false, true, frame)); };
		kp_ori->WhenToggle = [=] { toggle_kp(selected_obj, false, true, frame); };
		set_key_ctrl(ori_id, pick(kp_ori));
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
		bool do_key = selected_obj->write_enabled;
		if (e->auto_key && e->timeline_object_key == selected_obj->key &&
		    (e->timeline_component == Edit3D::TC_TRANSFORM || e->timeline_component == Edit3D::TC_NONE))
			do_key = true;
		if (do_key) {
			GeomTimeline& tl = selected_obj->GetTimeline();
			int frame = e->anim->position;
			GeomKeypoint& kp = tl.GetAddKeypoint(frame);
			if (pr->kind == PropRef::P_POSITION)
				kp.position = pos;
			if (pr->kind == PropRef::P_ORIENTATION)
				kp.orientation = ori;
			if (pr->kind == PropRef::P_POSITION)
				kp.has_position = true;
			if (pr->kind == PropRef::P_ORIENTATION)
				kp.has_orientation = true;
			selected_obj->read_enabled = true;
			tree.SetRowValue(focus_tree_id, tree_col_read, selected_obj->read_enabled);
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
	bar.Add(t_("View: YZ"), [=] { if (rends[i]) rends[i]->SetViewMode(VIEWMODE_YZ); RefreshRenderer(i); });
	bar.Add(t_("View: XZ"), [=] { if (rends[i]) rends[i]->SetViewMode(VIEWMODE_XZ); RefreshRenderer(i); });
	bar.Add(t_("View: XY"), [=] { if (rends[i]) rends[i]->SetViewMode(VIEWMODE_XY); RefreshRenderer(i); });
	bar.Add(t_("View: Perspective"), [=] { if (rends[i]) rends[i]->SetViewMode(VIEWMODE_PERSPECTIVE); RefreshRenderer(i); });
	bar.Separator();
	bar.Add(t_("Camera: Focus"), [=] { if (rends[i]) rends[i]->SetCameraSource(CAMSRC_FOCUS); RefreshRenderer(i); });
	bar.Add(t_("Camera: Program"), [=] { if (rends[i]) rends[i]->SetCameraSource(CAMSRC_PROGRAM); RefreshRenderer(i); });
	bar.Separator();
	bar.Add(t_("Renderer: V1"), [=] { SetRendererVersion(i, 1); }).Check(rend_version[i] == 1);
	bar.Add(t_("Renderer: V2"), [=] { SetRendererVersion(i, 2); }).Check(rend_version[i] == 2);
	bar.Add(t_("Renderer: Wireframe"), [=] {
		if (rends[i]) {
			rends[i]->SetWireframeOnly(!rends[i]->IsWireframeOnly());
			RefreshRenderer(i);
		}
	}).Check(rends[i] && rends[i]->IsWireframeOnly());
	bar.Separator();
	bar.Add(t_("Reset Camera"), [=] {
		if (!rends[i])
			return;
		GeomCamera& cam = rends[i]->GetGeomCamera();
		cam.position = vec3(0, 0, 0);
		cam.orientation = Identity<quat>();
		cam.scale = 1.0f;
		RefreshRenderer(i);
	});
}

void GeomProjectCtrl::SetRendererVersion(int i, int version) {
	if (i < 0 || i >= 4)
		return;
	if (version != 1 && version != 2)
		return;
	EditRendererBase* prev = rends[i];
	rend_version[i] = version;
	EditRendererBase* next = (version == 1) ? static_cast<EditRendererBase*>(&rends_v1[i])
	                                        : static_cast<EditRendererBase*>(&rends_v2[i]);
	if (prev && next) {
		next->view_mode = prev->view_mode;
		next->cam_src = prev->cam_src;
		next->ctx = prev->ctx;
	}
	rends[i] = next;
	RebuildGrid();
	RefreshRenderer(i);
}

void GeomProjectCtrl::RebuildGrid() {
	for (int i = 0; i < 4; i++) {
		grid.RemoveChild(&rends_v1[i]);
		grid.RemoveChild(&rends_v2[i]);
	}
	for (int i = 0; i < 4; i++) {
		if (rends[i])
			grid.Add(*rends[i]);
	}
	grid.Layout();
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
	auto add_bone = [&](auto&& add_bone, int parent_id, VfsValue& bone_node) -> void {
		GeomBone& bone = bone_node.GetExt<GeomBone>();
		String name = bone.name.IsEmpty() ? bone_node.id : bone.name;
		int j = tree.Add(parent_id, ImagesImg::Object(), RawToValue(&bone_node), name);
		tree.SetRowValue(j, tree_col_visible, Null);
		tree.SetRowValue(j, tree_col_locked, Null);
		tree.SetRowValue(j, tree_col_read, Null);
		tree.SetRowValue(j, tree_col_write, Null);
		for (auto& sub : bone_node.sub) {
			if (IsVfsType(sub, AsTypeHash<GeomBone>()))
				add_bone(add_bone, j, sub);
		}
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
		for (auto& sub : s->sub) {
			if (!IsVfsType(sub, AsTypeHash<GeomSkeleton>()))
				continue;
			GeomSkeleton& sk = sub.GetExt<GeomSkeleton>();
			String sk_name = sk.name.IsEmpty() ? sub.id : sk.name;
			int sk_id = tree.Add(j, ImagesImg::Directory(), RawToValue(&sub), sk_name);
			tree.SetRowValue(sk_id, tree_col_visible, Null);
			tree.SetRowValue(sk_id, tree_col_locked, Null);
			tree.SetRowValue(sk_id, tree_col_read, Null);
			tree.SetRowValue(sk_id, tree_col_write, Null);
			for (auto& b : sub.sub) {
				if (IsVfsType(b, AsTypeHash<GeomBone>()))
					add_bone(add_bone, sk_id, b);
			}
		}
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

	timeline_rows.Clear();
	
	Vector<TimelineRowInfo> rows;
	TimelineRowInfo scene_row;
	scene_row.kind = TimelineRowInfo::R_SCENE;
	scene_row.indent = 0;
	scene_row.has_children = true;
	scene_row.expanded = true;
	scene_row.active = (e->timeline_scope == Edit3D::TS_SCENE);
	rows.Add(scene_row);

	for (GeomObject* obj : objects) {
		TimelineRowInfo obj_row;
		obj_row.kind = TimelineRowInfo::R_OBJECT;
		obj_row.object_key = obj->key;
		obj_row.indent = 1;
		obj_row.has_children = true;
		int idx = timeline_expanded.Find(obj->key);
		obj_row.expanded = (idx >= 0);
		obj_row.active = (e->timeline_scope != Edit3D::TS_SCENE && e->timeline_object_key == obj->key && e->timeline_component == Edit3D::TC_NONE);
		rows.Add(obj_row);

		if (obj_row.has_children && obj_row.expanded) {
			TimelineRowInfo tr;
			tr.kind = TimelineRowInfo::R_TRANSFORM;
			tr.object_key = obj->key;
			tr.indent = 2;
			tr.active = (e->timeline_object_key == obj->key && e->timeline_component == Edit3D::TC_TRANSFORM && e->timeline_transform_field == Edit3D::TT_NONE);
			rows.Add(tr);
			TimelineRowInfo pr;
			pr.kind = TimelineRowInfo::R_POSITION;
			pr.object_key = obj->key;
			pr.indent = 3;
			pr.active = (e->timeline_object_key == obj->key && e->timeline_component == Edit3D::TC_TRANSFORM && e->timeline_transform_field == Edit3D::TT_POSITION);
			rows.Add(pr);
			TimelineRowInfo orow;
			orow.kind = TimelineRowInfo::R_ORIENTATION;
			orow.object_key = obj->key;
			orow.indent = 3;
			orow.active = (e->timeline_object_key == obj->key && e->timeline_component == Edit3D::TC_TRANSFORM && e->timeline_transform_field == Edit3D::TT_ORIENTATION);
			rows.Add(orow);
			if (obj->FindEditableMesh()) {
				TimelineRowInfo mr;
				mr.kind = TimelineRowInfo::R_MESH;
				mr.object_key = obj->key;
				mr.indent = 2;
				mr.active = (e->timeline_object_key == obj->key && e->timeline_component == Edit3D::TC_MESH);
				rows.Add(mr);
			}
			if (obj->Find2DLayer()) {
				TimelineRowInfo r2d;
				r2d.kind = TimelineRowInfo::R_2D;
				r2d.object_key = obj->key;
				r2d.indent = 2;
				r2d.active = (e->timeline_object_key == obj->key && e->timeline_component == Edit3D::TC_2D);
				rows.Add(r2d);
			}
		}
	}
	timeline_rows = pick(rows);
	time.SetCount(timeline_rows.GetCount());
	time.SetKeypointRate(prj.kps);
	time.SetLength(scene.length);
	time.SetKeypointColumnWidth(13);
	
	for (int i = 0; i < timeline_rows.GetCount(); i++) {
		TimelineRowInfo& info = timeline_rows[i];
		TimelineRowCtrl& row = time.GetRowIndex(i);
		row.SetIndent(info.indent);
		row.SetHasChildren(info.has_children);
		row.SetExpanded(info.expanded);
		row.SetActive(info.active);
		Index<int> keys;
		String name;
		if (info.kind == TimelineRowInfo::R_SCENE) {
			name = scene.name.IsEmpty() ? "Scene" : scene.name;
		}
		else if (GeomObject* obj = e->state->FindObjectByKey(info.object_key)) {
			name = obj->name.IsEmpty() ? "Object" : obj->name;
			if (info.kind == TimelineRowInfo::R_TRANSFORM) name << " / Transform";
			if (info.kind == TimelineRowInfo::R_POSITION) name << " / Position";
			if (info.kind == TimelineRowInfo::R_ORIENTATION) name << " / Orientation";
			if (info.kind == TimelineRowInfo::R_MESH) name << " / Mesh";
			if (info.kind == TimelineRowInfo::R_2D) name << " / 2D";
			if (info.kind == TimelineRowInfo::R_OBJECT) {
				if (GeomTimeline* tl = obj->FindTimeline()) {
					for (int k = 0; k < tl->keypoints.GetCount(); k++) {
						int key = tl->keypoints.GetKey(k);
						if ((tl->keypoints[k].has_position || tl->keypoints[k].has_orientation) && keys.Find(key) < 0)
							keys.Add(key);
					}
				}
				if (GeomMeshAnimation* ma = obj->FindMeshAnimation()) {
					for (int k = 0; k < ma->keyframes.GetCount(); k++) {
						int key = ma->keyframes.GetKey(k);
						if (keys.Find(key) < 0)
							keys.Add(key);
					}
				}
				if (Geom2DAnimation* a2d = obj->Find2DAnimation()) {
					for (int k = 0; k < a2d->keyframes.GetCount(); k++) {
						int key = a2d->keyframes.GetKey(k);
						if (keys.Find(key) < 0)
							keys.Add(key);
					}
				}
			}
			else if (info.kind == TimelineRowInfo::R_TRANSFORM) {
				if (GeomTimeline* tl = obj->FindTimeline()) {
					for (int k = 0; k < tl->keypoints.GetCount(); k++) {
						int key = tl->keypoints.GetKey(k);
						if ((tl->keypoints[k].has_position || tl->keypoints[k].has_orientation) && keys.Find(key) < 0)
							keys.Add(key);
					}
				}
			}
			else if (info.kind == TimelineRowInfo::R_POSITION) {
				if (GeomTimeline* tl = obj->FindTimeline()) {
					for (int k = 0; k < tl->keypoints.GetCount(); k++) {
						if (!tl->keypoints[k].has_position)
							continue;
						int key = tl->keypoints.GetKey(k);
						if (keys.Find(key) < 0)
							keys.Add(key);
					}
				}
			}
			else if (info.kind == TimelineRowInfo::R_ORIENTATION) {
				if (GeomTimeline* tl = obj->FindTimeline()) {
					for (int k = 0; k < tl->keypoints.GetCount(); k++) {
						if (!tl->keypoints[k].has_orientation)
							continue;
						int key = tl->keypoints.GetKey(k);
						if (keys.Find(key) < 0)
							keys.Add(key);
					}
				}
			}
			else if (info.kind == TimelineRowInfo::R_MESH) {
				if (GeomMeshAnimation* ma = obj->FindMeshAnimation()) {
					for (int k = 0; k < ma->keyframes.GetCount(); k++) {
						int key = ma->keyframes.GetKey(k);
						if (keys.Find(key) < 0)
							keys.Add(key);
					}
				}
			}
			else if (info.kind == TimelineRowInfo::R_2D) {
				if (Geom2DAnimation* a2d = obj->Find2DAnimation()) {
					for (int k = 0; k < a2d->keyframes.GetCount(); k++) {
						int key = a2d->keyframes.GetKey(k);
						if (keys.Find(key) < 0)
							keys.Add(key);
					}
				}
			}
		}
		row.SetTitle(name);
		Vector<int> row_keys;
		row_keys.SetCount(keys.GetCount());
		for (int k = 0; k < keys.GetCount(); k++)
			row_keys[k] = keys[k];
		Sort(row_keys);
		row.SetKeypoints(row_keys);
		row.Refresh();
	}
	
	time.Refresh();
}

void GeomProjectCtrl::TimelineRowMenu(Bar& bar, int row) {
	if (!e || !e->state)
		return;
	if (row < 0 || row >= timeline_rows.GetCount())
		return;
	const TimelineRowInfo& info = timeline_rows[row];
	if (info.kind == TimelineRowInfo::R_SCENE) {
		bar.Add(t_("Make Active Timeline"), [=] {
			e->timeline_scope = Edit3D::TS_SCENE;
			e->timeline_component = Edit3D::TC_NONE;
			e->timeline_object_key = 0;
			TimelineData();
		});
		bar.Add(t_("Auto-key"), [=] { e->auto_key = !e->auto_key; }).Check(e->auto_key);
		return;
	}
	hash_t key = info.object_key;
	GeomObject* obj = e->state->FindObjectByKey(key);
	if (!obj)
		return;
	int frame = e->anim ? e->anim->position : 0;
	bar.Add(t_("Make Active Timeline"), [=] {
		e->timeline_scope = Edit3D::TS_OBJECT;
		e->timeline_object_key = obj->key;
		e->timeline_component = Edit3D::TC_NONE;
		e->timeline_transform_field = Edit3D::TT_NONE;
		if (info.kind == TimelineRowInfo::R_TRANSFORM) {
			e->timeline_scope = Edit3D::TS_COMPONENT;
			e->timeline_component = Edit3D::TC_TRANSFORM;
		}
		else if (info.kind == TimelineRowInfo::R_POSITION) {
			e->timeline_scope = Edit3D::TS_COMPONENT;
			e->timeline_component = Edit3D::TC_TRANSFORM;
			e->timeline_transform_field = Edit3D::TT_POSITION;
		}
		else if (info.kind == TimelineRowInfo::R_ORIENTATION) {
			e->timeline_scope = Edit3D::TS_COMPONENT;
			e->timeline_component = Edit3D::TC_TRANSFORM;
			e->timeline_transform_field = Edit3D::TT_ORIENTATION;
		}
		else if (info.kind == TimelineRowInfo::R_MESH) {
			e->timeline_scope = Edit3D::TS_COMPONENT;
			e->timeline_component = Edit3D::TC_MESH;
		}
		else if (info.kind == TimelineRowInfo::R_2D) {
			e->timeline_scope = Edit3D::TS_COMPONENT;
			e->timeline_component = Edit3D::TC_2D;
		}
		TimelineData();
	});
	bar.Add(t_("Auto-key"), [=] { e->auto_key = !e->auto_key; }).Check(e->auto_key);
	bar.Separator();
	auto add_transform_kp = [&](bool pos, bool ori) {
		if (!e || !e->state)
			return;
		GeomTimeline& tl = obj->GetTimeline();
		GeomKeypoint& kp = tl.GetAddKeypoint(frame);
		kp.frame_id = frame;
		if (const GeomObjectState* os = e->state->FindObjectStateByKey(obj->key)) {
			if (pos) kp.position = os->position;
			if (ori) kp.orientation = os->orientation;
		}
		else if (GeomTransform* tr = obj->FindTransform()) {
			if (pos) kp.position = tr->position;
			if (ori) kp.orientation = tr->orientation;
		}
		if (pos) kp.has_position = true;
		if (ori) kp.has_orientation = true;
		TimelineData();
	};
	auto clear_transform_kp = [&](bool pos, bool ori) {
		if (GeomTimeline* tl = obj->FindTimeline()) {
			for (int i = tl->keypoints.GetCount() - 1; i >= 0; i--) {
				GeomKeypoint& kp = tl->keypoints[i];
				if (pos) kp.has_position = false;
				if (ori) kp.has_orientation = false;
				if (!kp.has_position && !kp.has_orientation)
					tl->keypoints.Remove(i);
			}
		}
		TimelineData();
	};
	if (info.kind == TimelineRowInfo::R_POSITION) {
		bar.Add(t_("Add Position Keyframe"), [=] { add_transform_kp(true, false); });
		bar.Add(t_("Clear Position Keyframes"), [=] { clear_transform_kp(true, false); });
	}
	else if (info.kind == TimelineRowInfo::R_ORIENTATION) {
		bar.Add(t_("Add Orientation Keyframe"), [=] { add_transform_kp(false, true); });
		bar.Add(t_("Clear Orientation Keyframes"), [=] { clear_transform_kp(false, true); });
	}
	else {
		bar.Add(t_("Add Transform Keyframe"), [=] { add_transform_kp(true, true); });
		if (info.kind == TimelineRowInfo::R_TRANSFORM) {
			bar.Add(t_("Add Position Keyframe"), [=] { add_transform_kp(true, false); });
			bar.Add(t_("Add Orientation Keyframe"), [=] { add_transform_kp(false, true); });
		}
		if (obj->FindEditableMesh()) {
			bar.Add(t_("Add Mesh Keyframe"), [=] {
				if (GeomEditableMesh* mesh = obj->FindEditableMesh()) {
					GeomMeshAnimation& anim = obj->GetMeshAnimation();
					GeomMeshKeyframe& kf = anim.GetAddKeyframe(frame);
					kf.frame_id = frame;
					kf.points.SetCount(mesh->points.GetCount());
					for (int i = 0; i < mesh->points.GetCount(); i++)
						kf.points[i] = mesh->points[i];
					TimelineData();
				}
			});
		}
		if (obj->Find2DLayer()) {
			bar.Add(t_("Add 2D Keyframe"), [=] {
				if (Geom2DLayer* layer = obj->Find2DLayer()) {
					Geom2DAnimation& anim = obj->Get2DAnimation();
					Geom2DKeyframe& kf = anim.GetAddKeyframe(frame);
					kf.frame_id = frame;
					kf.shapes.SetCount(layer->shapes.GetCount());
					for (int i = 0; i < layer->shapes.GetCount(); i++) {
						const Geom2DShape& s = layer->shapes[i];
						Geom2DShape& d = kf.shapes[i];
						d.type = s.type;
						d.radius = s.radius;
						d.stroke = s.stroke;
						d.width = s.width;
						d.closed = s.closed;
						d.points.SetCount(s.points.GetCount());
						for (int k = 0; k < s.points.GetCount(); k++)
							d.points[k] = s.points[k];
					}
					TimelineData();
				}
			});
		}
	}
	bar.Separator();
	if (info.kind == TimelineRowInfo::R_POSITION) {
		bar.Add(t_("Clear Position Keyframes"), [=] { clear_transform_kp(true, false); });
	}
	else if (info.kind == TimelineRowInfo::R_ORIENTATION) {
		bar.Add(t_("Clear Orientation Keyframes"), [=] { clear_transform_kp(false, true); });
	}
	else {
		bar.Add(t_("Clear Transform Keyframes"), [=] { clear_transform_kp(true, true); });
	}
	if (GeomMeshAnimation* ma = obj->FindMeshAnimation()) {
		bar.Add(t_("Clear Mesh Keyframes"), [=] {
			ma->keyframes.Clear();
			TimelineData();
		});
	}
	if (Geom2DAnimation* a2d = obj->Find2DAnimation()) {
		bar.Add(t_("Clear 2D Keyframes"), [=] {
			a2d->keyframes.Clear();
			TimelineData();
		});
	}
	bar.Add(t_("Clear All Keyframes"), [=] {
		if (GeomTimeline* tl = obj->FindTimeline()) {
			for (int i = tl->keypoints.GetCount() - 1; i >= 0; i--)
				tl->keypoints.Remove(i);
		}
		if (GeomMeshAnimation* ma = obj->FindMeshAnimation())
			ma->keyframes.Clear();
		if (Geom2DAnimation* a2d = obj->Find2DAnimation())
			a2d->keyframes.Clear();
		TimelineData();
	});
}

void GeomProjectCtrl::TimelineRowSelect(int row) {
	if (!e || row < 0 || row >= timeline_rows.GetCount())
		return;
	const TimelineRowInfo& info = timeline_rows[row];
	if (info.kind == TimelineRowInfo::R_SCENE) {
		e->timeline_scope = Edit3D::TS_SCENE;
		e->timeline_component = Edit3D::TC_NONE;
		e->timeline_transform_field = Edit3D::TT_NONE;
		e->timeline_object_key = 0;
	}
	else {
		e->timeline_scope = Edit3D::TS_OBJECT;
		e->timeline_object_key = info.object_key;
		e->timeline_component = Edit3D::TC_NONE;
		e->timeline_transform_field = Edit3D::TT_NONE;
		if (info.kind == TimelineRowInfo::R_TRANSFORM) {
			e->timeline_scope = Edit3D::TS_COMPONENT;
			e->timeline_component = Edit3D::TC_TRANSFORM;
		}
		else if (info.kind == TimelineRowInfo::R_POSITION) {
			e->timeline_scope = Edit3D::TS_COMPONENT;
			e->timeline_component = Edit3D::TC_TRANSFORM;
			e->timeline_transform_field = Edit3D::TT_POSITION;
		}
		else if (info.kind == TimelineRowInfo::R_ORIENTATION) {
			e->timeline_scope = Edit3D::TS_COMPONENT;
			e->timeline_component = Edit3D::TC_TRANSFORM;
			e->timeline_transform_field = Edit3D::TT_ORIENTATION;
		}
		else if (info.kind == TimelineRowInfo::R_MESH) {
			e->timeline_scope = Edit3D::TS_COMPONENT;
			e->timeline_component = Edit3D::TC_MESH;
		}
		else if (info.kind == TimelineRowInfo::R_2D) {
			e->timeline_scope = Edit3D::TS_COMPONENT;
			e->timeline_component = Edit3D::TC_2D;
		}
	}
	TimelineData();
}

void GeomProjectCtrl::TimelineRowToggle(int row) {
	if (row < 0 || row >= timeline_rows.GetCount())
		return;
	const TimelineRowInfo& info = timeline_rows[row];
	if (info.kind != TimelineRowInfo::R_OBJECT)
		return;
	int idx = timeline_expanded.Find(info.object_key);
	if (idx >= 0)
		timeline_expanded.Remove(idx);
	else
		timeline_expanded.Add(info.object_key);
	TimelineData();
}

END_UPP_NAMESPACE
