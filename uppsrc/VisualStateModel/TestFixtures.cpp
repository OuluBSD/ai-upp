#include "VisualStateModel.h"

namespace Upp {

bool VsmBuildSyntheticSession(const VsmSyntheticSessionOptions& opts,
                               VsmSessionStore& out_store)
{
	AppLog log;
	log.SetForwardToUppLog(false);
	out_store.SetLog(&log);

	// Create the session store
	if(!out_store.Create(opts.output_dir, opts.session_id, opts.width, opts.height)) {
		return false;
	}

	// Generate frames
	for(int i = 0; i < opts.frame_count; ++i) {
		VsmImageBuffer frame;

		if(opts.solid_value >= 0) {
			// If solid_value is specified, use it for all frames
			byte value = (byte)opts.solid_value;
			frame = VsmImageBuffer::MakeSolid(opts.width, opts.height, value, 1);
		} else {
			// Use rotating pattern: Solid, Gradient, Checkerboard
			int pattern = i % 3;

			if(pattern == 0) {
				// Frame 0, 3, 6, ... : MakeSolid with varying grayscale values
				// Use 100 + i*25 to get distinct values (100, 125, 150, 175, 200, ...)
				byte value = (byte)(100 + (i * 25) % 156); // Keep within 100-255 range
				frame = VsmImageBuffer::MakeSolid(opts.width, opts.height, value, 1);
			} else if(pattern == 1) {
				// Frame 1, 4, 7, ... : MakeGradient
				frame = VsmImageBuffer::MakeGradient(opts.width, opts.height);
			} else {
				// Frame 2, 5, 8, ... : MakeCheckerboard
				int cell_size = opts.checkerboard_cell_size;
				if(cell_size < 0) {
					// Auto-compute based on frame dimensions (at least 2 cells per dimension)
					cell_size = (opts.width / 2) > 0 ? (opts.width / 2) : 1;
					if(opts.height < opts.width) {
						cell_size = (opts.height / 2) > 0 ? (opts.height / 2) : 1;
					}
				}
				frame = VsmImageBuffer::MakeCheckerboard(opts.width, opts.height, cell_size);
			}
		}

		// Save the frame image
		if(out_store.SaveFrameImage(i, frame).IsEmpty()) {
			return false;
		}
	}

	// Save the manifest
	if(!out_store.SaveManifest()) {
		return false;
	}

	return true;
}

} // namespace Upp
