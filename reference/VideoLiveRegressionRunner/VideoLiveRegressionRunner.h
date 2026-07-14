#ifndef _VideoLiveRegressionRunner_VideoLiveRegressionRunner_h_
#define _VideoLiveRegressionRunner_VideoLiveRegressionRunner_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct LiveRegressionOptions {
	String host = "127.0.0.1";
	int    port = 8082;
	int    frames = 10;
	String name = "live_smoke";
	String out_root = "tmp";
	bool   help = false;
};

END_UPP_NAMESPACE

#endif

