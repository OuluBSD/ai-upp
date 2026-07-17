#ifndef _VideoSemanticOcrProbe_VideoSemanticOcrProbe_h_
#define _VideoSemanticOcrProbe_VideoSemanticOcrProbe_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

NAMESPACE_UPP

struct OcrProbeOptions {
	String tracker_dir;
	String out_path;
	String crop_list;
	String tesseract = "C:\\Program Files\\Tesseract-OCR\\tesseract.exe";
	String tessdata_dir = "C:\\Program Files\\Tesseract-OCR\\tessdata";
	String lang = "eng";
	int    psm = 6;
	int    max_crops = 40;
	bool   help = false;
	bool   tessdata_dir_explicit = false;
	bool   preprocess = true;
	// Task 0272: Otsu binarization + convolution-based predictive polarity
	// detection, run as a third preprocessing variant alongside original/
	// grayscale (--preprocess). Independently toggleable so callers can
	// compare with/without it.
	bool   otsu = true;
	// Task 0273 Phase 1: word-level bounding boxes via tesseract's built-in
	// `tsv` config, used both as ground truth for crop-boundary refinement
	// (Phases 2-3) and as a confidence signal for variant selection
	// (Phase 5). The `tsv` CONFIG FILE lives in the system Tesseract-OCR
	// install's tessdata/configs/tsv -- NOT necessarily in whatever
	// language-data tessdata_dir got resolved above (the local/fallback
	// tessdata dirs this tool searches only ever contain *.traineddata,
	// not the configs/ subtree) -- so it is resolved separately, normally
	// as a sibling of the `tesseract` executable itself
	// (<tesseract-dir>/tessdata/configs/tsv), overridable via --tsv-config.
	String tsv_config;
	bool   tsv_config_explicit = false;
};

struct OcrCrop : Moveable<OcrCrop> {
	int    frame_index = 0;
	int    table_id = 0;
	String semantic;
	String path;
};

// Task 0273 Phase 1: one word-level detection from tesseract's `tsv` config
// output (level==5 rows only; level 1-4 rows are page/block/par/line
// aggregates and carry conf==-1, so RunTesseractTsv() below filters to
// level 5 words already).
struct OcrWordBox : Moveable<OcrWordBox> {
	int    left = 0;
	int    top = 0;
	int    width = 0;
	int    height = 0;
	double conf = -1;
	String text;
};

struct OcrResult : Moveable<OcrResult> {
	OcrCrop crop;
	String original_text;
	String preprocessed_text;
	String preprocessed_path;
	int    original_exit_code = -1;
	int    preprocessed_exit_code = -1;
	// Task 0272: Otsu(+auto-polarity) variant.
	String otsu_text;
	String otsu_path;
	int    otsu_exit_code = -1;
	bool   otsu_invert = false;
	bool   otsu_confident = false;
	// Task 0273 Phase 1: word bboxes + avg conf per variant, from the `tsv`
	// config. avg_conf is the mean of the level-5 word confidences (-1, i.e.
	// "no words recognized", if the word list is empty) -- used by Phase 5's
	// confidence-based BestVariant() replacement.
	Vector<OcrWordBox> original_words;
	Vector<OcrWordBox> preprocessed_words;
	Vector<OcrWordBox> otsu_words;
	double original_avg_conf = -1;
	double preprocessed_avg_conf = -1;
	double otsu_avg_conf = -1;
};

END_UPP_NAMESPACE

#endif
