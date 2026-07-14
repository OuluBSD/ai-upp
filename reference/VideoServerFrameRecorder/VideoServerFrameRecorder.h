#ifndef _VideoServerFrameRecorder_VideoServerFrameRecorder_h_
#define _VideoServerFrameRecorder_VideoServerFrameRecorder_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

NAMESPACE_UPP

struct RecorderOptions {
	String host = "127.0.0.1";
	int    port = 8082;
	int    frames = 5;
	int    poll_ms = 33;
	int    timeout_ms = 5000;
	String out_dir;
	bool   help = false;
};

struct RecorderFrameInfo : Moveable<RecorderFrameInfo> {
	int    index = 0;
	uint32 id = 0;
	String path;
	Size   size;
	String format;
};

END_UPP_NAMESPACE

#endif

