#ifndef _VisualStateModel_RegionOverlay_h_
#define _VisualStateModel_RegionOverlay_h_

// Shared, non-Ctrl changed-region overlay drawing, extracted from
// reference/VisualStateWorkbench/FrameCanvas::DrawRegionOverlay (task 0105) so
// it can be reused both by the GUI Workbench and by a headless overlay-PNG
// renderer (reference/VisualStateRegionDump). Placed in uppsrc/VisualStateModel
// rather than a Workbench-only location because the package depends on Draw
// (added in 0103) but not on CtrlCore/CtrlLib — a headless CONSOLE_APP can link
// it, a Workbench-only location could not have been linked headlessly.

#include <Draw/Draw.h>

namespace Upp {

// Draws each region in `regions` as a translucent fill + frame border + a
// "%.0f%%" score label, offset by `origin` (e.g. FrameCanvas's kTopOffset
// reserved-header-space offset; pass Point(0, 0) for a 1:1 frame-pixel
// overlay, as the headless PNG renderer does). `selected` (index into
// `regions`, or -1 for none) controls the brighter GUI selection-highlight
// colors; headless callers that have no notion of selection pass -1.
void VsmDrawRegionOverlay(Draw& w, const Vector<VsmChangedRect>& regions,
                          Point origin, int selected = -1);

} // namespace Upp

#endif
