#include "MainWindow.h"

FrameCanvas::FrameCanvas()
{
	BackPaint();
}

void FrameCanvas::SetChangedRegions(const Vector<VsmChangedRect>& regions)
{
	regions_.Clear();
	for(const VsmChangedRect& r : regions)
		regions_.Add(r);
	selected_region_ = -1;
	Refresh();
}

// ---------------------------------------------------------------------------
// Coordinate helpers

Rect FrameCanvas::AnnotationToCanvas(int ax, int ay, int aw, int ah) const
{
	return Rect(ax, ay + kTopOffset, ax + aw, ay + ah + kTopOffset);
}

Rect FrameCanvas::CanvasToAnnotation(Rect r) const
{
	return Rect(r.left, r.top - kTopOffset, r.right, r.bottom - kTopOffset);
}

Rect FrameCanvas::DragRect() const
{
	Point a = drag_start_, b = drag_cur_;
	return Rect(min(a.x, b.x), min(a.y, b.y), max(a.x, b.x), max(a.y, b.y));
}

// ---------------------------------------------------------------------------
// Paint

void FrameCanvas::Paint(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, Color(20, 20, 20));

	// Draw empty-state placeholder if no session loaded on first run
	if(show_empty_state_placeholder_) {
		Font fnt = StdFont(12);
		Color txt_color = SColorText();

		String line1 = "No session loaded.";
		String line2 = "Use File \xE2\x86\x92 Open/Import Session\xE2\x80\xA6 to open a recorded session,";
		String line3 = "import an image sequence, or load the built-in sample data to explore";
		String line4 = "the workbench.";

		Size s1 = GetTextSize(line1, fnt);
		Size s2 = GetTextSize(line2, fnt);
		Size s3 = GetTextSize(line3, fnt);
		Size s4 = GetTextSize(line4, fnt);

		int max_width = max(max(max(s1.cx, s2.cx), s3.cx), s4.cx);
		int total_height = s1.cy + s2.cy + s3.cy + s4.cy + 12; // 12 = spacing

		int y = (sz.cy - total_height) / 2;
		int x1 = (sz.cx - s1.cx) / 2;
		int x2 = (sz.cx - s2.cx) / 2;
		int x3 = (sz.cx - s3.cx) / 2;
		int x4 = (sz.cx - s4.cx) / 2;

		w.DrawText(x1, y, line1, fnt, txt_color);
		y += s1.cy + 4;
		w.DrawText(x2, y, line2, fnt, txt_color);
		y += s2.cy + 2;
		w.DrawText(x3, y, line3, fnt, txt_color);
		y += s3.cy + 2;
		w.DrawText(x4, y, line4, fnt, txt_color);

		return;
	}

	// Real per-frame image (task 0131), drawn under the overlays at 1:1 scale,
	// anchored just below the top info line so overlay coordinates (which add
	// kTopOffset) map pixel-exact onto the frame.
	if(!frame_image_.IsEmpty())
		w.DrawImage(0, kTopOffset, frame_image_);

	// Session info line
	if(!info_text_.IsEmpty()) {
		w.DrawText(8, 4, info_text_, StdFont(), SColorText());
	} else if(session_) {
		String frame_str = current_frame_ >= 0 ? Format("  frame: %d", current_frame_) : "";
		String info = Format("Session: %s  %dx%d  src: %s%s",
		                     session_->session_id, session_->frame_width,
		                     session_->frame_height, session_->source_type, frame_str);
		w.DrawText(8, 4, info, StdFont(), SColorText());
	} else {
		w.DrawText(8, 4, "No session loaded — drag to create annotation", StdFont(), SColorShadow());
	}

	if(show_layout_)      DrawLayoutOverlay(w);
	if(show_regions_)     DrawRegionOverlay(w);
	if(show_annotations_) DrawAnnotationOverlay(w);
	if(show_template_)    DrawTemplateOverlay(w);
	if(show_ocr_)         DrawOcrOverlay(w);

	if(drag_mode_ != DRAG_NONE)
		DrawDragPreview(w);

	if(regions_.IsEmpty() && !ann_layer_ && session_)
		w.DrawText(8, kTopOffset + 8,
		           "No changed regions — step through replay or run pipeline",
		           StdFont(), SColorShadow());
}

void FrameCanvas::DrawRegionOverlay(Draw& w) const
{
	// Drawing logic itself lives in the shared, non-Ctrl VsmDrawRegionOverlay
	// helper (uppsrc/VisualStateModel/RegionOverlay.h) so the headless M03 CLI
	// (reference/VisualStateRegionDump) can render the identical overlay into
	// a PNG. Only the GUI-specific kTopOffset coordinate offset and the
	// current selection index are supplied here.
	VsmDrawRegionOverlay(w, regions_, Point(0, kTopOffset), selected_region_);
}

void FrameCanvas::DrawAnnotationOverlay(Draw& w) const
{
	if(!ann_layer_) return;
	for(int i = 0; i < ann_layer_->annotations.GetCount(); i++) {
		const VsmRegionAnnotation& a = ann_layer_->annotations[i];
		bool sel = (i == selected_ann_);
		Color border = sel ? Color(80, 255, 120) : Color(0, 200, 80);
		Rect rr = AnnotationToCanvas(a.x, a.y, a.w, a.h);
		DrawFrame(w, rr, border);
		DrawFrame(w, rr.Deflated(1), Color(border.GetR() / 2, border.GetG() / 2, border.GetB() / 2));
		w.DrawText(rr.left + 2, rr.top + 2, a.name, StdFont(8), Color(80, 255, 120));
		// Selection handles (corners)
		if(sel) {
			const int hs = 4;
			auto DrawHandle = [&](int cx, int cy) {
				w.DrawRect(cx - hs/2, cy - hs/2, hs, hs, Color(255, 255, 255));
			};
			DrawHandle(rr.left,  rr.top);
			DrawHandle(rr.right, rr.top);
			DrawHandle(rr.left,  rr.bottom);
			DrawHandle(rr.right, rr.bottom);
		}
	}
}

void FrameCanvas::DrawTemplateOverlay(Draw& w) const
{
	if(!tmpl_results_ || !ann_layer_) return;
	for(const VsmTemplateMatchResult& res : *tmpl_results_) {
		if(ann_layer_->annotations.IsEmpty()) continue;
		const VsmRegionAnnotation& a = ann_layer_->annotations[0];
		Rect rr = AnnotationToCanvas(a.x, a.y, a.w, a.h);
		Color c = res.matched ? Color(100, 180, 255) : Color(80, 80, 160);
		DrawFrame(w, rr.Deflated(2), c);
		String lbl = (res.matched ? String("T+ ") : String("T- ")) + res.matched_label;
		w.DrawText(rr.left + 3, rr.bottom - 14, lbl, StdFont(8), c);
	}
}

void FrameCanvas::DrawOcrOverlay(Draw& w) const
{
	if(!ocr_results_) return;
	if(!ann_layer_) return;
	int badge_y = kTopOffset + 4;
	for(const VsmOcrResult& res : *ocr_results_) {
		String badge = "OCR[" + res.rule_id + "]: " + res.text;
		Size sz = GetSize();
		int tx = sz.cx - 8 - GetTextSize(badge, StdFont(9)).cx;
		w.DrawText(tx, badge_y, badge, StdFont(9), Color(255, 220, 100));
		badge_y += 14;
	}
}

void FrameCanvas::DrawLayoutOverlay(Draw& w) const
{
	// Layout-binding overlay (task 0132): draws the `.form`-driven layout model
	// (M04) distinctly from the plain changed-region overlay. Candidate rects
	// are in frame-pixel space already (VsmBuildCandidates scaled them), so they
	// map onto the frame with the same +kTopOffset the region/annotation
	// overlays use. Two layers:
	//   1. All candidate rects, faint, elements vs sub-slots in distinct colors
	//      (context — where every declared element/sub-slot sits this frame).
	//   2. This frame's matched bindings: the matched candidate rect emphasized
	//      with its role label; the selected binding highlighted.
	auto ToCanvas = [&](const Rect& r) {
		return Rect(r.left, r.top + kTopOffset, r.right, r.bottom + kTopOffset);
	};

	const Color kElement = Color(200, 140, 0);    // amber — top-level elements
	const Color kSubSlot = Color(0, 170, 190);    // cyan  — resolved sub-slots
	const Color kMatched = Color(120, 220, 255);  // bright cyan — matched candidate
	const Color kSelected = Color(255, 235, 60);  // yellow — selected binding

	if(layout_model_ && layout_model_->loaded) {
		for(const VsmLayoutCandidate& c : layout_model_->candidates) {
			Rect rr = ToCanvas(c.rect);
			if(rr.Width() <= 0 || rr.Height() <= 0) continue;
			DrawFrame(w, rr, c.kind == "subslot" ? kSubSlot : kElement);
		}
	}

	if(layout_bindings_) {
		for(const VsmLayoutBinding& b : *layout_bindings_) {
			if(!b.matched) continue;
			bool sel = (b.region_index == selected_region_);
			Rect rr = ToCanvas(b.candidate_rect);
			if(rr.Width() <= 0 || rr.Height() <= 0) continue;
			Color c = sel ? kSelected : kMatched;
			DrawFrame(w, rr, c);
			DrawFrame(w, rr.Deflated(1), c);
			// Role label (+ seat/card index when present) just inside the rect.
			String lbl = b.role;
			if(b.seat_index >= 0) lbl << " s" << b.seat_index;
			if(b.card_index >= 0) lbl << " c" << b.card_index;
			w.DrawText(rr.left + 2, rr.top + 2, lbl, StdFont(8), c);
		}
	}
}

void FrameCanvas::DrawDragPreview(Draw& w) const
{
	if(drag_mode_ == DRAG_CREATE) {
		Rect dr = DragRect();
		if(dr.Width() < 2 || dr.Height() < 2) return;
		DrawFrame(w, dr, Color(0, 200, 80));
		DrawFrame(w, dr.Deflated(1), Color(0, 100, 40));
		w.DrawText(dr.left + 3, dr.top + 3, "new annotation", StdFont(8), Color(0, 200, 80));
	}
}

// ---------------------------------------------------------------------------
// Interaction

void FrameCanvas::LeftDown(Point p, dword)
{
	SetCapture();
	int ann_hit = HitTestAnnotation(p);
	int reg_hit = HitTestRegion(p);

	if(ann_hit >= 0 && ann_layer_) {
		// Select existing annotation and prepare to move it
		selected_ann_    = ann_hit;
		selected_region_ = -1;
		const VsmRegionAnnotation& a = ann_layer_->annotations[ann_hit];
		Rect rr = AnnotationToCanvas(a.x, a.y, a.w, a.h);
		drag_mode_   = DRAG_MOVE;
		drag_start_  = p;
		drag_cur_    = p;
		drag_offset_ = Point(p.x - rr.left, p.y - rr.top);
		Refresh();
		WhenRegionSelected(a.id);
	} else if(reg_hit >= 0) {
		selected_region_ = reg_hit;
		selected_ann_    = -1;
		drag_mode_ = DRAG_NONE;
		Refresh();
		WhenRegionSelected(Format("region-%d", reg_hit));
	} else {
		// Begin drag-to-create
		selected_ann_    = -1;
		selected_region_ = -1;
		drag_mode_  = DRAG_CREATE;
		drag_start_ = p;
		drag_cur_   = p;
		Refresh();
		WhenRegionSelected(String());
	}
}

void FrameCanvas::MouseMove(Point p, dword keyflags)
{
	if(drag_mode_ == DRAG_NONE) return;
	drag_cur_ = p;
	if(drag_mode_ == DRAG_MOVE && ann_layer_ && selected_ann_ >= 0) {
		// Live-move the annotation
		VsmRegionAnnotation& a = ann_layer_->annotations[selected_ann_];
		Rect ar = CanvasToAnnotation(
		    Rect(p.x - drag_offset_.x, p.y - drag_offset_.y,
		         p.x - drag_offset_.x + a.w, p.y - drag_offset_.y + a.h));
		a.x = max(0, ar.left);
		a.y = max(0, ar.top);
	}
	Refresh();
}

void FrameCanvas::LeftUp(Point p, dword)
{
	ReleaseCapture();
	drag_cur_ = p;

	if(drag_mode_ == DRAG_CREATE && ann_layer_) {
		Rect dr = DragRect();
		Rect ar = CanvasToAnnotation(dr);
		if(ar.Width() >= kMinSize && ar.Height() >= kMinSize) {
			// Create new annotation
			VsmRegionAnnotation& a = ann_layer_->annotations.Add();
			a.id   = "ann-" + IntStr(GetTickCount() & 0xFFFF);
			a.name = "New";
			a.x    = ar.left;
			a.y    = ar.top;
			a.w    = ar.Width();
			a.h    = ar.Height();
			selected_ann_ = ann_layer_->annotations.GetCount() - 1;
			WhenAnnotationCreated();
		}
	} else if(drag_mode_ == DRAG_MOVE) {
		if(ann_layer_ && selected_ann_ >= 0)
			WhenAnnotationMoved();
	}

	drag_mode_ = DRAG_NONE;
	Refresh();
}

int FrameCanvas::HitTestRegion(Point p) const
{
	for(int i = regions_.GetCount() - 1; i >= 0; i--) {
		const VsmChangedRect& r = regions_[i];
		Rect rr = AnnotationToCanvas(r.x, r.y, r.w, r.h);
		if(rr.Contains(p)) return i;
	}
	return -1;
}

int FrameCanvas::HitTestAnnotation(Point p) const
{
	if(!ann_layer_) return -1;
	for(int i = ann_layer_->annotations.GetCount() - 1; i >= 0; i--) {
		const VsmRegionAnnotation& a = ann_layer_->annotations[i];
		Rect rr = AnnotationToCanvas(a.x, a.y, a.w, a.h);
		if(rr.Contains(p)) return i;
	}
	return -1;
}
