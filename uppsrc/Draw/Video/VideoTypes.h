#ifndef _Draw_Video_VideoTypes_h_
#define _Draw_Video_VideoTypes_h_

struct VideoFrame : Moveable<VideoFrame> {
	Image img;
	int64 timestamp_us = 0;
	dword flags = 0;
};

struct VideoStats {
	int frame_count = 0;
	int dropped_frames = 0;
};

#endif
