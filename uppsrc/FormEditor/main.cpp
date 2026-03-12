#include "FormEditor.h"

static void DumpFormPreview(const String& path)
{
	FormWindow fw;
	if(!fw.Load(path)) {
		printf("ERROR: could not load %s\n", ~path);
		Exit(1);
	}

	Form& form = fw.GetForm();
	Array<FormLayout>& layouts = form.GetLayouts();
	if(layouts.IsEmpty()) {
		printf("ERROR: no layouts in form\n");
		Exit(1);
	}

	String layout_name = layouts[0].Get("Form.Name");
	printf("Layout: %s\n", ~layout_name);

	fw.PreviewChrome();
	bool ok = fw.Layout(layout_name);
	if(!ok) {
		printf("ERROR: Layout() failed\n");
		Exit(1);
	}

	FormLayout& lay = layouts[0];
	Size sz = lay.GetFormSize();
	printf("Design size: %d x %d\n", sz.cx, sz.cy);
	int form_w = Ctrl::HorzLayoutZoom(sz.cx);
	int form_h = Ctrl::VertLayoutZoom(sz.cy);
	printf("Zoomed size: %d x %d\n\n", form_w, form_h);

	Array<FormObject>& objs = lay.GetObjects();
	ArrayMap<String, Ctrl>& ctrls = form.GetCtrls();

	int n = min(objs.GetCount(), ctrls.GetCount());
	for(int i = 0; i < n; i++) {
		FormObject& obj = objs[i];
		Ctrl& c = ctrls[i];

		String var    = obj.Get("Variable");
		String anchor = obj.Get("Anchor");
		Rect   dr     = obj.GetRect();
		dword  ha     = obj.GetHAlign();
		dword  va     = obj.GetVAlign();

		// Replicate resolution (must match Form.cpp ResolveAnchorLayout exactly)
		int x  = dr.left,  cx = dr.Width();
		int y  = dr.top,   cy = dr.Height();
		int ml = x, mt = y;  // margin left/top after resolution
		dword rha = ha, rva = va;

		if(!anchor.IsEmpty()) {
			if(anchor == "CENTER") {
				ml = x - (sz.cx - cx) / 2;
				mt = y - (sz.cy - cy) / 2;
				rha = Ctrl::CENTER; rva = Ctrl::CENTER;
			} else if(anchor == "BOTTOM_CENTER") {
				ml = x - (sz.cx - cx) / 2;
				mt = sz.cy - y - cy;
				rha = Ctrl::CENTER; rva = Ctrl::BOTTOM;
			} else if(anchor == "TOP_CENTER") {
				ml = x - (sz.cx - cx) / 2;
				mt = y;
				rha = Ctrl::CENTER; rva = Ctrl::TOP;
			} else if(anchor == "CENTER_LEFT") {
				ml = x;
				mt = y - (sz.cy - cy) / 2;
				rha = Ctrl::LEFT; rva = Ctrl::CENTER;
			} else if(anchor == "CENTER_RIGHT") {
				ml = sz.cx - x - cx;
				mt = y - (sz.cy - cy) / 2;
				rha = Ctrl::RIGHT; rva = Ctrl::CENTER;
			} else if(anchor == "BOTTOM_LEFT") {
				ml = x;
				mt = sz.cy - y - cy;
				rha = Ctrl::LEFT; rva = Ctrl::BOTTOM;
			} else if(anchor == "BOTTOM_RIGHT") {
				ml = sz.cx - x - cx;
				mt = sz.cy - y - cy;
				rha = Ctrl::RIGHT; rva = Ctrl::BOTTOM;
			} else if(anchor == "TOP_RIGHT") {
				ml = sz.cx - x - cx;
				mt = y;
				rha = Ctrl::RIGHT; rva = Ctrl::TOP;
			} else {
				ml = x; mt = y;
				rha = Ctrl::LEFT; rva = Ctrl::TOP;
			}
		}

		// Align name helper
		static const char* anames[] = { "CENTER", "LEFT", "RIGHT", "SIZE" };
		auto aname = [&](dword a) -> const char* { return (a < 4) ? anames[a] : "?"; };

		// Zoom margin values (as PosZ functions do: zoom a and b separately)
		int zml = Ctrl::HorzLayoutZoom(ml), zcx = Ctrl::HorzLayoutZoom(cx);
		int zmt = Ctrl::VertLayoutZoom(mt),  zcy = Ctrl::VertLayoutZoom(cy);
		// For SIZE: right margin = sz.cx - x - cx, bottom margin = sz.cy - y - cy
		// (SIZE not set by anchor resolution, so use original coords)
		int zright_margin  = Ctrl::HorzLayoutZoom(sz.cx - x - cx);
		int zbottom_margin = Ctrl::VertLayoutZoom(sz.cy - y - cy);

		// Compute expected rect using Lay1 logic
		int ex_l, ex_r, ex_t, ex_b;
		{
			int pos = zml, size = zcx;
			switch(rha) {
			case Ctrl::CENTER: pos = (form_w - zcx) / 2 + zml; break;
			case Ctrl::RIGHT:  pos = form_w - (zml + zcx);     break;
			case Ctrl::SIZE:   size = form_w - (zml + zright_margin); break;
			}
			ex_l = pos; ex_r = pos + max(size, 0);
		}
		{
			int pos = zmt, size = zcy;
			switch(rva) {
			case Ctrl::CENTER: pos = (form_h - zcy) / 2 + zmt; break;
			case Ctrl::BOTTOM: pos = form_h - (zmt + zcy);     break;
			case Ctrl::SIZE:   size = form_h - (zmt + zbottom_margin); break;
			}
			ex_t = pos; ex_b = pos + max(size, 0);
		}

		Rect actual = c.GetRect();

		printf("[%d] %s  anchor=%s\n", i, ~var, ~anchor);
		printf("  design:   x=%d y=%d cx=%d cy=%d\n", x, y, cx, cy);
		printf("  xml h=%s(%d) v=%s(%d)\n", aname(ha), ha, aname(va), va);
		printf("  resolved: h=%s(%d) v=%s(%d)  margin_l=%d margin_t=%d\n",
		       aname(rha), rha, aname(rva), rva, ml, mt);
		printf("  expected: x=%d y=%d  r=%d b=%d  (cx=%d cy=%d)\n",
		       ex_l, ex_t, ex_r, ex_b, ex_r-ex_l, ex_b-ex_t);
		printf("  actual:   x=%d y=%d  r=%d b=%d  (cx=%d cy=%d)\n",
		       actual.left, actual.top, actual.right, actual.bottom,
		       actual.Width(), actual.Height());
		if(actual.left != ex_l || actual.top != ex_t || actual.right != ex_r || actual.bottom != ex_b)
			printf("  *** MISMATCH ***\n");
		printf("\n");
	}
	Exit(0);
}

#ifdef flagMAIN
GUI_APP_MAIN
{
	const Vector<String>& cmd = CommandLine();

	bool dump = false;
	String open_path;
	for(int i = 0; i < cmd.GetCount(); i++) {
		if(cmd[i] == "--dump-form-preview")
			dump = true;
		else if(cmd[i][0] != '-')
			open_path = cmd[i];
	}

	if(dump) {
		if(open_path.IsEmpty()) {
			printf("Usage: FormEditor --dump-form-preview <file.form>\n");
			Exit(1);
		}
		DumpFormPreview(open_path);
		return;
	}

	int mode = 0;
	if (mode == 0) {
		DockableFormEdit editor;
		if(!open_path.IsEmpty())
			editor.OpenPath(open_path);
		editor.Run();
	}
	else if (mode == 1) {
		FormEditWindow editor;
		if(!open_path.IsEmpty())
			editor.OpenPath(open_path);
		editor.Run();
	}
	else {
		TopWindow tw;
		FormEditCtrl c;
		if(!open_path.IsEmpty())
			c.OpenPath(open_path);
		tw.Add(c.SizePos());
		tw.Run();
	}
}
#endif
