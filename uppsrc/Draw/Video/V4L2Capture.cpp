#include "Video.h"

NAMESPACE_UPP

bool VideoV4L2Backend::Open() {
#ifdef flagLINUX
	return false;
#else
	return false;
#endif
}

void VideoV4L2Backend::Close() {
}

bool VideoV4L2Backend::IsOpen() const {
	return false;
}

void VideoV4L2Backend::PopFrames(Vector<VideoFrame>& out) {
	out.Clear();
}

bool VideoV4L2Backend::SetFormat(VideoPixelFormat f, Size s, int frames_per_sec) {
	fmt = f;
	size = s;
	fps = frames_per_sec;
	return false;
}

bool VideoV4L2Backend::GetFormat(VideoPixelFormat& f, Size& s, int& frames_per_sec) const {
	f = fmt;
	s = size;
	frames_per_sec = fps;
	return false;
}

END_UPP_NAMESPACE
