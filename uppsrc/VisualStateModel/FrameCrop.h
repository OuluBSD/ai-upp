#ifndef _VisualStateModel_FrameCrop_h_
#define _VisualStateModel_FrameCrop_h_

// Shared headless region-crop rendering, extracted from
// reference/VisualStateRegionDump/main.cpp's task-0110 `--crop-report-out`
// code (task 0116) so both VisualStateRegionDump and VisualStateLayoutAssign
// can render a debug crop PNG for one changed region without duplicating the
// RGBA -> Image conversion or the crop/pad geometry. Placed in
// uppsrc/VisualStateModel (not a tool-local file) mirroring the exact
// precedent task 0105 already set for VsmDrawRegionOverlay
// (uppsrc/VisualStateModel/RegionOverlay.h) — avoid copy-paste drift between
// two CLIs that both need to render a region crop. Draw-only (no
// CtrlCore/CtrlLib), so it stays headless-linkable from a CONSOLE_APP.

#include <Draw/Draw.h>

namespace Upp {

// Raw RGBA VsmFrameImage -> U++ Image, one row at a time via ImageBuffer.
// Named .r/.g/.b/.a member access (not raw byte offsets) matches
// PngFrame.cpp's decode direction, so this stays correct regardless of
// RGBA's in-memory byte order.
Image VsmFrameImageToImage(const VsmFrameImage& frame);

// Debug crop image padding (task 0110): a small fixed context margin
// surrounding a changed-region rectangle — NOT the whole frame (that's what
// VsmSaveOverlayPng/--overlay-out already produces). Clamped to the frame
// bounds so regions near an edge still crop cleanly.
static const int kCropPadding = 12;

// Crops `frame` to `region`'s rect + kCropPadding on each side (clamped to
// frame bounds) and saves it as a PNG at `path`. Returns false if `frame` is
// empty or the PNG write fails.
bool VsmSaveRegionCropPng(const VsmFrameImage& frame, const VsmChangedRect& region,
                          const String& path);

} // namespace Upp

#endif
