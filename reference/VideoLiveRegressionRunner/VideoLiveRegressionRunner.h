#ifndef _VideoLiveRegressionRunner_VideoLiveRegressionRunner_h_
#define _VideoLiveRegressionRunner_VideoLiveRegressionRunner_h_

#include <Core/Core.h>
#include <VisualStateModel/VisualStateModel.h>

NAMESPACE_UPP

struct LiveRegressionOptions {
	String host = "127.0.0.1";
	int    port = 8082;
	int    frames = 10;
	String name = "live_smoke";
	String out_root = "tmp";
	String table_mode = "unknown";
	int    expect_frames = -1;
	int    min_frames = -1;
	int    expect_tables = -1;
	int    min_tables = -1;
	int    min_events = -1;
	int    min_ocr_crops = -1;
	int    min_usable_tables = -1;
	bool   require_ocr_ok = false;
	bool   help = false;
	Vector<String> required_events;
	Vector<String> required_ocr_texts;
};

END_UPP_NAMESPACE

#endif
