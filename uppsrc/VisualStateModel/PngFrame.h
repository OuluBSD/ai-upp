#ifndef _VisualStateModel_PngFrame_h_
#define _VisualStateModel_PngFrame_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Decode a PNG (or any other Draw-supported raster format) straight into a
// VsmFrameImage RGBA buffer. Implemented over StreamRaster::LoadFileAny —
// Draw + plugin/png gated only, no CtrlCore/CtrlLib dependency (see task 0102
// build evidence: Draw links headlessly from a CONSOLE_APP; plugin/png is
// needed in the link so the PNG raster format actually registers itself).

bool VsmLoadPngFrame(const String& path, VsmFrameImage& out);

// ---------------------------------------------------------------------------
// Minimal M01/M02 TexasHoldem session frame reader.
//
// Reads just enough of metadata.json (table_width, table_height, frame_count,
// provider, session_id) to iterate frames/%08d.png without downstream tools
// (0104/0105) each re-parsing the full session contract. This intentionally
// does not depend on game/TexasHoldem/TexasHoldemSessionContract — VSM is a
// lower-layer, headless package and must not depend upward on game code; the
// frame filename pattern ("%08d.png") is a stable, documented part of the
// contract (Manager/2-plan/ai-upp/root/VisualStateModel/docs/TEXAS_HOLDEM_SOURCE_CONTRACT.md) and is
// duplicated here as a small literal, not a re-implementation of the format.

struct VsmM01M02SessionInfo : Moveable<VsmM01M02SessionInfo> {
	int    table_width  = 0;
	int    table_height = 0;
	int    frame_count  = 0;
	String provider;
	String session_id;

	bool IsEmpty() const { return frame_count == 0; }
};

// Read metadata.json from a M01/M02 session root directory.
bool VsmReadM01M02SessionInfo(const String& session_root, VsmM01M02SessionInfo& out);

// Decode frames/%08d.png for the given frame_id under session_root.
bool VsmLoadM01M02SessionFrame(const String& session_root, int frame_id, VsmFrameImage& out);

} // namespace Upp

#endif
