#ifndef _VideoTableStateExtractor_VideoTableStateExtractor_h_
#define _VideoTableStateExtractor_VideoTableStateExtractor_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>
#include <VisualStateModel/VisualStateModel.h>

NAMESPACE_UPP

struct TableStateOptions {
	String tracker_dir;
	String tracking_summary_json;
	String ocr_json;
	String table_quality_json;
	String out_path;
	String table_mode = "unknown";
	bool   help = false;
};

struct OcrTextKey : Moveable<OcrTextKey> {
	int    frame_index = 0;
	int    table_id = 0;
	String semantic;
	String text;
	String path;
};

struct ExtractedTableState : Moveable<ExtractedTableState> {
	int    frame_index = 0;
	int    table_id = 0;
	String table_mode;
	bool   usable = false;
	String quality_reason;
	String title_text;
	String pot_text;
	String board_crop_path;
	int    board_card_count = 0;
	double board_confidence = 0;
	String board_reason;
	Vector<OcrTextKey> seat_texts;
	double confidence = 0;
	Vector<String> reasons;
};

END_UPP_NAMESPACE

#endif
