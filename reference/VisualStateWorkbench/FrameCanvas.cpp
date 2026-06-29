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
	selected_ = -1;
	Refresh();
}

void FrameCanvas::Paint(Draw& w)
{
	Size sz = GetSize();

	// Dark background (represents an uncaptured/dimmed frame)
	w.DrawRect(sz, Color(30, 30, 30));

	if(session_) {
		String info = Format("Session: %s  %dx%d  source: %s",
		                     session_->session_id,
		                     session_->frame_width,
		                     session_->frame_height,
		                     session_->source_type);
		w.DrawText(8, 8, info, StdFont(), SColorText());
	} else {
		w.DrawText(8, 8, "No session loaded", StdFont(), SColorShadow());
	}

	// Draw changed region overlays
	for(int i = 0; i < regions_.GetCount(); i++) {
		const VsmChangedRect& r = regions_[i];
		Color fill = (i == selected_) ? Color(255, 200, 0) : Color(255, 80, 0);
		Color border = (i == selected_) ? Color(255, 255, 0) : Color(255, 140, 0);
		// Semi-transparent fill via 20% dimming + overlay rectangle
		Rect rr(r.x, r.y + 24, r.x + r.w, r.y + r.h + 24); // +24 for info text
		w.DrawRect(rr, Color(fill.GetR(), fill.GetG(), fill.GetB()));
		DrawFrame(w, rr, border);
		// Score label
		String label = Format("%.0f%%", r.score * 100);
		w.DrawText(rr.left + 2, rr.top + 2, label, StdFont(10), White());
	}

	if(regions_.IsEmpty() && session_) {
		w.DrawText(8, 32, "No changed regions — load a session and step through events",
		           StdFont(), SColorShadow());
	}
}

void FrameCanvas::LeftDown(Point p, dword)
{
	int hit = HitTest(p);
	selected_ = hit;
	Refresh();
	if(hit >= 0)
		WhenRegionSelected(Format("region-%d", hit));
	else
		WhenRegionSelected(String());
}

int FrameCanvas::HitTest(Point p) const
{
	for(int i = regions_.GetCount() - 1; i >= 0; i--) {
		const VsmChangedRect& r = regions_[i];
		Rect rr(r.x, r.y + 24, r.x + r.w, r.y + r.h + 24);
		if(rr.Contains(p)) return i;
	}
	return -1;
}
