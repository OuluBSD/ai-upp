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
};

struct OcrCrop : Moveable<OcrCrop> {
	int    frame_index = 0;
	int    table_id = 0;
	String semantic;
	String path;
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
};

END_UPP_NAMESPACE

#endif
