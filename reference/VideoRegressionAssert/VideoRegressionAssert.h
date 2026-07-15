#ifndef _VideoRegressionAssert_VideoRegressionAssert_h_
#define _VideoRegressionAssert_VideoRegressionAssert_h_

#include <Core/Core.h>
#include <VisualStateModel/VisualStateModel.h>

NAMESPACE_UPP

struct VideoAssertOptions {
	String tracker_dir;
	String pipeline_summary;
	String events_json;
	String ocr_json;
	String table_quality_json;
	String table_mode;
	int    expect_frames = -1;
	int    min_frames = -1;
	int    expect_tables = -1;
	int    min_tables = -1;
	int    min_events = -1;
	int    min_ocr_crops = -1;
	int    min_usable_tables = -1;
	bool   require_ocr_ok = false;
	bool   help = false;
	bool   parse_error = false;
	Vector<String> required_events;
	Vector<String> required_ocr_texts;
};

struct VideoAssertContext {
	String pipeline_text;
	String events_text;
	String ocr_text;
	String table_quality_text;
	ValueMap pipeline;
	ValueMap events_root;
	ValueMap ocr_root;
	bool pipeline_ok = false;
	bool events_ok = false;
	bool ocr_ok = false;
	bool table_quality_ok = false;
};

END_UPP_NAMESPACE

#endif
