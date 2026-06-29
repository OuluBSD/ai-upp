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
// Paint

void FrameCanvas::Paint(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, Color(20, 20, 20));

	// Session info line
	if(session_) {
		String info = Format("Session: %s  %dx%d  src: %s",
		                     session_->session_id, session_->frame_width,
		                     session_->frame_height, session_->source_type);
		w.DrawText(8, 4, info, StdFont(), SColorText());
	} else {
		w.DrawText(8, 4, "No session loaded", StdFont(), SColorShadow());
	}

	if(show_regions_)     DrawRegionOverlay(w);
	if(show_annotations_) DrawAnnotationOverlay(w);
	if(show_template_)    DrawTemplateOverlay(w);
	if(show_ocr_)         DrawOcrOverlay(w);

	if(regions_.IsEmpty() && !ann_layer_ && session_)
		w.DrawText(8, kTopOffset + 8,
		           "No changed regions — step through replay or run pipeline",
		           StdFont(), SColorShadow());
}

void FrameCanvas::DrawRegionOverlay(Draw& w) const
{
	for(int i = 0; i < regions_.GetCount(); i++) {
		const VsmChangedRect& r = regions_[i];
		bool sel = (i == selected_region_);
		Color fill   = sel ? Color(255, 220, 50)  : Color(255, 80,  0);
		Color border = sel ? Color(255, 255, 120) : Color(255, 140, 0);
		Rect rr(r.x, r.y + kTopOffset, r.x + r.w, r.y + r.h + kTopOffset);
		w.DrawRect(rr, Color(fill.GetR() / 4, fill.GetG() / 4, fill.GetB() / 4)); // 25% fill
		DrawFrame(w, rr, border);
		String label = Format("%.0f%%", r.score * 100);
		w.DrawText(rr.left + 2, rr.top + 2, label, StdFont(9), White());
	}
}

void FrameCanvas::DrawAnnotationOverlay(Draw& w) const
{
	if(!ann_layer_) return;
	for(int i = 0; i < ann_layer_->annotations.GetCount(); i++) {
		const VsmRegionAnnotation& a = ann_layer_->annotations[i];
		bool sel = (i == selected_ann_);
		Color border = sel ? Color(80, 255, 120) : Color(0, 200, 80);
		Rect rr(a.x, a.y + kTopOffset, a.x + a.w, a.y + a.h + kTopOffset);
		DrawFrame(w, rr, border);
		DrawFrame(w, rr.Deflated(1), Color(border.GetR() / 2, border.GetG() / 2, border.GetB() / 2));
		w.DrawText(rr.left + 2, rr.top + 2, a.name, StdFont(8), Color(80, 255, 120));
	}
}

void FrameCanvas::DrawTemplateOverlay(Draw& w) const
{
	if(!tmpl_results_ || !ann_layer_) return;
	// Each result only knows rule_id; we match it visually to all annotations
	for(const VsmTemplateMatchResult& res : *tmpl_results_) {
		if(ann_layer_->annotations.IsEmpty()) continue;
		// Draw on first annotation as a visual badge
		const VsmRegionAnnotation& a = ann_layer_->annotations[0];
		Rect rr(a.x, a.y + kTopOffset, a.x + a.w, a.y + a.h + kTopOffset);
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
		// Draw OCR badge near top-right of canvas
		String badge = "OCR[" + res.rule_id + "]: " + res.text;
		Size sz = GetSize();
		int tx = sz.cx - 8 - GetTextSize(badge, StdFont(9)).cx;
		w.DrawText(tx, badge_y, badge, StdFont(9), Color(255, 220, 100));
		badge_y += 14;
	}
}

// ---------------------------------------------------------------------------
// Interaction

void FrameCanvas::LeftDown(Point p, dword)
{
	int ann_hit = HitTestAnnotation(p);
	int reg_hit = HitTestRegion(p);

	selected_ann_    = ann_hit;
	selected_region_ = reg_hit;
	Refresh();

	if(ann_hit >= 0 && ann_layer_) {
		WhenRegionSelected(ann_layer_->annotations[ann_hit].id);
	} else if(reg_hit >= 0) {
		WhenRegionSelected(Format("region-%d", reg_hit));
	} else {
		WhenRegionSelected(String());
	}
}

int FrameCanvas::HitTestRegion(Point p) const
{
	for(int i = regions_.GetCount() - 1; i >= 0; i--) {
		const VsmChangedRect& r = regions_[i];
		Rect rr(r.x, r.y + kTopOffset, r.x + r.w, r.y + r.h + kTopOffset);
		if(rr.Contains(p)) return i;
	}
	return -1;
}

int FrameCanvas::HitTestAnnotation(Point p) const
{
	if(!ann_layer_) return -1;
	for(int i = ann_layer_->annotations.GetCount() - 1; i >= 0; i--) {
		const VsmRegionAnnotation& a = ann_layer_->annotations[i];
		Rect rr(a.x, a.y + kTopOffset, a.x + a.w, a.y + a.h + kTopOffset);
		if(rr.Contains(p)) return i;
	}
	return -1;
}
