#ifndef _VideoSemanticOcrProbe_VideoSemanticOcrProbe_h_
#define _VideoSemanticOcrProbe_VideoSemanticOcrProbe_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

NAMESPACE_UPP

struct OcrProbeOptions {
	String tracker_dir;
	String out_path;
	String tesseract = "C:\\Program Files\\Tesseract-OCR\\tesseract.exe";
	String tessdata_dir = "C:\\Program Files\\Tesseract-OCR\\tessdata";
	String lang = "eng";
	int    psm = 6;
	int    max_crops = 40;
	bool   help = false;
	bool   tessdata_dir_explicit = false;
	bool   preprocess = true;
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
};

END_UPP_NAMESPACE

#endif
