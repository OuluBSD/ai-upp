#include "VisualStateModel.h"

namespace Upp {

void VsmDrawRegionOverlay(Draw& w, const Vector<VsmChangedRect>& regions,
                          Point origin, int selected)
{
	for(int i = 0; i < regions.GetCount(); i++) {
		const VsmChangedRect& r = regions[i];
		bool sel = (i == selected);
		Color fill   = sel ? Color(255, 220, 50)  : Color(255, 80,  0);
		Color border = sel ? Color(255, 255, 120) : Color(255, 140, 0);
		Rect rr(origin.x + r.x, origin.y + r.y,
		        origin.x + r.x + r.w, origin.y + r.y + r.h);
		w.DrawRect(rr, Color(fill.GetR() / 4, fill.GetG() / 4, fill.GetB() / 4));
		DrawFrame(w, rr, border);
		String label = Format("%.0f%%", r.score * 100);
		w.DrawText(rr.left + 2, rr.top + 2, label, StdFont(9), White());
	}
}

} // namespace Upp
