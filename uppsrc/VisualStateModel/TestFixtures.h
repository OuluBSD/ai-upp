#ifndef _VisualStateModel_TestFixtures_h_
#define _VisualStateModel_TestFixtures_h_

namespace Upp {

// Options for building a synthetic test session with a few frames.
// Used to consolidate boilerplate across reference packages and test fixtures.
struct VsmSyntheticSessionOptions : Moveable<VsmSyntheticSessionOptions> {
	String output_dir;           // Directory where session will be created
	String session_id;           // Session identifier
	int    frame_count = 3;      // Number of frames to generate
	int    width = 32;           // Frame width in pixels
	int    height = 32;          // Frame height in pixels
	int    solid_value = -1;     // If >= 0, use this grayscale value for ALL frames;
	                              // else use rotating pattern (Solid/Gradient/Checkerboard)
	int    checkerboard_cell_size = -1; // For checkerboard frames, cell size;
	                              // if -1, auto-compute based on dimensions
};

// Build a synthetic test session with generated frames.
// Frames are generated via a rotating pattern of:
//   Frame 0 mod 3: MakeSolid (grayscale value varies by frame index)
//   Frame 1 mod 3: MakeGradient
//   Frame 2 mod 3: MakeCheckerboard
//
// Returns true on success, false on error.
// The session is created in output_dir/session_id/
bool VsmBuildSyntheticSession(const VsmSyntheticSessionOptions& opts,
                               VsmSessionStore& out_store);

} // namespace Upp

#endif
