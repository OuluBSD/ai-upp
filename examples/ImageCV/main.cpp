#include <CtrlLib/CtrlLib.h>
#include <ComputerVision/ComputerVision.h>

using namespace Upp;

static String FindFromCurrentOrParents(const char* rel_path, int max_up = 8) {
	String base = GetCurrentDirectory();
	for (int i = 0; i <= max_up; i++) {
		String p = AppendFileName(base, rel_path);
		if (FileExists(p))
			return p;
		base = DirectoryUp(base);
	}
	return String();
}

static void ImageToGrayMat(const Image& img, ByteMat& out) {
	Size sz = img.GetSize();
	out.SetSize(sz.cx, sz.cy, 1);
	for (int y = 0; y < sz.cy; y++) {
		const RGBA* s = img[y];
		for (int x = 0; x < sz.cx; x++) {
			const RGBA& p = s[x];
			int g = (int)p.r + (int)p.g + (int)p.b;
			out.data[y * sz.cx + x] = (byte)(g / 3);
		}
	}
}

static Image FloatMatToGrayImage(const FloatMat& m, bool invert = false) {
	if (m.IsEmpty() || m.channels != 1)
		return Image();
	
	double min_v = 0.0, max_v = 0.0;
	MinMaxLoc(m, &min_v, &max_v, 0, 0);
	double span = max_v - min_v;
	if (span < 1e-20)
		span = 1.0;
	
	ImageBuffer ib(m.cols, m.rows);
	for (int y = 0; y < m.rows; y++) {
		RGBA* d = ib[y];
		for (int x = 0; x < m.cols; x++) {
			double v = m.data[y * m.cols + x];
			double t = (v - min_v) / span;
			if (invert)
				t = 1.0 - t;
			int g = (int)clamp(t * 255.0, 0.0, 255.0);
			RGBA px;
			px.r = (byte)g;
			px.g = (byte)g;
			px.b = (byte)g;
			px.a = 255;
			d[x] = px;
		}
	}
	return ib;
}

class MatchDrawer : public Ctrl {
public:
	Image full_img;
	Image templ_img;
	Image score_img;
	TemplateMatchMethod method = TM_CCOEFF_NORMED;
	Point best;
	double min_v = 0.0;
	double max_v = 0.0;
	bool ok = false;
	
	void SetInputs(const Image& full, const Image& templ) {
		full_img = full;
		templ_img = templ;
		Compute();
	}
	
	void SetMethod(TemplateMatchMethod m) {
		method = m;
		Compute();
	}
	
	virtual void Paint(Draw& w) override {
		Size sz = GetSize();
		w.DrawRect(sz, SColorPaper());
		
		if (!ok) {
			w.DrawText(10, 10, "Could not load ./tmp/full.jpg and ./tmp/small.jpg", Arial(14).Bold(), Red());
			return;
		}
		
		const int gap = 8;
		Rect left = RectC(gap, gap, max(10, sz.cx / 2 - (gap * 2)), max(10, sz.cy - 2 * gap));
		Rect right = RectC(sz.cx / 2 + gap, gap, max(10, sz.cx - (sz.cx / 2 + 2 * gap)), max(10, sz.cy - 2 * gap));
		
		DrawPanel(w, left, score_img, "Matching results (higher coefficient -> brighter)", true);
		DrawPanel(w, right, full_img, "Full image with detected match", false);
	}
	
private:
	void Compute() {
		ok = false;
		if (IsNull(full_img) || IsNull(templ_img)) {
			Refresh();
			return;
		}
		
		ByteMat full_m;
		ByteMat templ_m;
		ImageToGrayMat(full_img, full_m);
		ImageToGrayMat(templ_img, templ_m);
		
		if (full_m.cols < templ_m.cols || full_m.rows < templ_m.rows) {
			Refresh();
			return;
		}
		
		FloatMat res;
		MatchTemplate(full_m, templ_m, res, method);
		Point min_p, max_p;
		MinMaxLoc(res, &min_v, &max_v, &min_p, &max_p);
		bool sqdiff = (method == TM_SQDIFF || method == TM_SQDIFF_NORMED);
		best = sqdiff ? min_p : max_p;
		score_img = FloatMatToGrayImage(res, sqdiff);
		ok = !IsNull(score_img);
		Refresh();
	}
	
	void DrawPanel(Draw& w, const Rect& panel, const Image& img, const char* title, bool mark_point) {
		w.DrawRect(panel, White());
		w.DrawRect(panel.left, panel.top, panel.GetWidth(), 1, SColorShadow());
		w.DrawRect(panel.left, panel.bottom - 1, panel.GetWidth(), 1, SColorShadow());
		w.DrawRect(panel.left, panel.top, 1, panel.GetHeight(), SColorShadow());
		w.DrawRect(panel.right - 1, panel.top, 1, panel.GetHeight(), SColorShadow());
		
		w.DrawText(panel.left + 6, panel.top + 4, title, Arial(12).Bold(), Black());
		
		Rect canvas = panel;
		canvas.top += 24;
		if (canvas.GetWidth() <= 4 || canvas.GetHeight() <= 4)
			return;
		
		Rect dr = FitRect(img.GetSize(), canvas);
		w.DrawImage(dr.left, dr.top, dr.GetWidth(), dr.GetHeight(), img);
		
		if (mark_point) {
			DrawBestMarker(w, dr, img.GetSize(), best, LtYellow());
		}
		else {
			DrawMatchRect(w, dr, img.GetSize(), best, templ_img.GetSize(), Yellow());
		}
		
		String info = Format("best=(%d,%d), min=%.6g, max=%.6g", best.x, best.y, min_v, max_v);
		w.DrawText(panel.left + 6, panel.bottom - 18, info, Arial(11), Black());
	}
	
	static Rect FitRect(Size isz, const Rect& area) {
		if (isz.cx <= 0 || isz.cy <= 0 || area.GetWidth() <= 0 || area.GetHeight() <= 0)
			return area;
		double sx = (double)area.GetWidth() / isz.cx;
		double sy = (double)area.GetHeight() / isz.cy;
		double sc = min(sx, sy);
		int w = max(1, (int)(isz.cx * sc));
		int h = max(1, (int)(isz.cy * sc));
		int x = area.left + (area.GetWidth() - w) / 2;
		int y = area.top + (area.GetHeight() - h) / 2;
		return RectC(x, y, w, h);
	}
	
	static void DrawBestMarker(Draw& w, const Rect& draw_rect, Size src_sz, Point p, Color c) {
		double sx = (double)draw_rect.GetWidth() / src_sz.cx;
		double sy = (double)draw_rect.GetHeight() / src_sz.cy;
		int x = draw_rect.left + (int)((p.x + 0.5) * sx);
		int y = draw_rect.top + (int)((p.y + 0.5) * sy);
		w.DrawLine(x - 6, y, x + 6, y, 2, c);
		w.DrawLine(x, y - 6, x, y + 6, 2, c);
	}
	
	static void DrawMatchRect(Draw& w, const Rect& draw_rect, Size src_sz, Point top_left, Size templ_sz, Color c) {
		double sx = (double)draw_rect.GetWidth() / src_sz.cx;
		double sy = (double)draw_rect.GetHeight() / src_sz.cy;
		int x = draw_rect.left + (int)(top_left.x * sx);
		int y = draw_rect.top + (int)(top_left.y * sy);
		int rw = max(1, (int)(templ_sz.cx * sx));
		int rh = max(1, (int)(templ_sz.cy * sy));
		w.DrawRect(x, y, rw, 2, c);
		w.DrawRect(x, y + rh - 2, rw, 2, c);
		w.DrawRect(x, y, 2, rh, c);
		w.DrawRect(x + rw - 2, y, 2, rh, c);
	}
};

class ImageCVWindow : public TopWindow {
public:
	typedef ImageCVWindow CLASSNAME;
	
	Splitter split;
	ArrayCtrl demos;
	MatchDrawer drawer;
	
	ImageCVWindow() {
		Title("ImageCV - Template Matching");
		Sizeable().Zoomable();
		
		demos.AddColumn("Demo");
		demos.Add("TM_CCOEFF");
		demos.Add("TM_CCOEFF_NORMED");
		demos.Add("TM_CCORR");
		demos.Add("TM_CCORR_NORMED");
		demos.Add("TM_SQDIFF");
		demos.Add("TM_SQDIFF_NORMED");
		demos.SetCursor(1);
		
		split.Horz(demos, drawer);
		split.SetPos(1500);
		Add(split.SizePos());
		
		demos.WhenSel = [=] { OnDemo(); };
		
		LoadImages();
		OnDemo();
	}
	
private:
	void LoadImages() {
		String full_path = FindFromCurrentOrParents("tmp/full.jpg");
		String small_path = FindFromCurrentOrParents("tmp/small.jpg");
		Image full = full_path.IsEmpty() ? Image() : StreamRaster::LoadFileAny(full_path);
		Image small = small_path.IsEmpty() ? Image() : StreamRaster::LoadFileAny(small_path);
		drawer.SetInputs(full, small);
	}
	
	void OnDemo() {
		int c = demos.GetCursor();
		if (c < 0)
			c = 0;
		TemplateMatchMethod m = TM_CCOEFF_NORMED;
		switch (c) {
		case 0: m = TM_CCOEFF; break;
		case 1: m = TM_CCOEFF_NORMED; break;
		case 2: m = TM_CCORR; break;
		case 3: m = TM_CCORR_NORMED; break;
		case 4: m = TM_SQDIFF; break;
		case 5: m = TM_SQDIFF_NORMED; break;
		}
		drawer.SetMethod(m);
	}
};

GUI_APP_MAIN {
	ImageCVWindow().Run();
}
