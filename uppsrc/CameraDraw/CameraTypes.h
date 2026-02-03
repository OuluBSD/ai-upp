#ifndef _CameraDraw_CameraTypes_h_
#define _CameraDraw_CameraTypes_h_

struct CameraFrame : Moveable<CameraFrame> {
	Image img;
	int64 timestamp_us = 0;
	dword flags = 0;
};

struct CameraStats {
	int frame_count = 0;
	int dropped_frames = 0;
};

#endif
