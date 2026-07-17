#ifndef _VisualStateModel_TesseractOcr_h_
#define _VisualStateModel_TesseractOcr_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Task 0277: the validated tesseract + Otsu + confidence-based-variant-
// selection per-crop OCR pipeline, extracted from reference/
// VideoSemanticOcrProbe/main.cpp's RunCrop() (and the PreprocessCrop/
// PreprocessCropOtsu/RunTesseract/RunTesseractTsv/TextScore/BestVariant/
// BestText helpers it called, plus the tesseract/tessdata path-resolution
// helpers GetTessdataCandidates/ResolveTessdataDir/ResolveTsvConfig/
// VerifyTesseractLanguage) so it can be shared between that reference CLI
// tool and a real production VsmOcrEngine (VsmTesseractOcrEngine, see
// TesseractOcrEngine.h). This is a disciplined 1:1 port -- the scoring
// formula, tie-break rules, and blank-crop threshold are all copied
// verbatim from the already-validated (100% on a real 31-crop dataset,
// Tasks 0271-0276) reference implementation, not reinvented.
//
// Operates on a plain file path rather than the reference tool's OcrCrop
// (which also carries frame_index/table_id metadata the core OCR logic
// itself never used for anything beyond building a diagnostic output
// filename -- see VsmRunTesseractOcr()'s `out_dir` parameter below, which
// keys that filename off the crop's own file title instead, exactly as
// Task 0275's bugfix already established is the only correct uniqueness
// key).

struct VsmTesseractOptions {
	String tesseract = "C:\\Program Files\\Tesseract-OCR\\tesseract.exe";
	String tessdata_dir; // empty = auto-detect, ported from GetTessdataCandidates()
	String lang = "eng";
	int    psm = 6;
	bool   preprocess = true;
	bool   otsu = true;
	String tsv_config; // empty = auto-resolve, ported from ResolveTsvConfig()

	// Mirrors the reference tool's OcrProbeOptions::tessdata_dir_explicit /
	// tsv_config_explicit: when a caller (e.g. a CLI --tessdata-dir/
	// --tsv-config flag) supplies a value explicitly, VsmAutoResolveTesseract()
	// verifies exactly that value and fails loudly if it's wrong rather than
	// silently falling back to a scan. Left false (the default) for a plain
	// "empty = auto-detect" caller -- confirmed necessary, not cosmetic: this
	// project's real tessdata directory is NOT at the hardcoded default
	// tesseract_dir path above, only found via the full candidate scan (see
	// VsmGetTessdataCandidates() in the .cpp), so treating a merely-defaulted
	// (non-explicit) value as if it were explicit would break resolution.
	bool   tessdata_dir_explicit = false;
	bool   tsv_config_explicit = false;
};

struct VsmTesseractOcrResult : Moveable<VsmTesseractOcrResult> {
	// Winning (BestVariant()-selected) text/exit-code -- what most callers want.
	String text;
	double confidence = 0.0; // BestVariant()'s chosen variant's avg_conf (-1 if unavailable)
	int    exit_code = -1;
	int    best_variant = 0; // 0=original, 1=preprocessed, 2=otsu -- BestVariant()'s exact logic
	bool   blank_detected = false; // Task 0274 Phase 4 grayscale-stddev blank-crop gate

	// Per-variant detail, kept for parity with the reference tool's OcrResult
	// (used by VideoSemanticOcrProbe's JSON output; also useful for anyone
	// diagnosing a real engine's OCR decision).
	String original_text;
	int    original_exit_code = -1;
	double original_avg_conf = -1;

	String preprocessed_text;
	String preprocessed_path; // set only when out_dir is supplied to VsmRunTesseractOcr()
	int    preprocessed_exit_code = -1;
	double preprocessed_avg_conf = -1;

	String otsu_text;
	String otsu_path; // set only when out_dir is supplied to VsmRunTesseractOcr()
	int    otsu_exit_code = -1;
	double otsu_avg_conf = -1;
	bool   otsu_invert = false;
	bool   otsu_confident = false;

	double crop_stddev = -1;
};

// Resolves opt.tesseract/tessdata_dir/tsv_config in place. Ports
// ResolveTessdataDir()/VerifyTesseractLanguage()/ResolveTsvConfig()'s exact
// candidate search order. Returns false (with `error` set) only for a
// missing tesseract executable or missing language data -- both fatal,
// matching the reference tool's RunProbe() gate. A missing tsv config is
// NOT fatal here either (matches the reference tool: scoring degrades
// gracefully to keyword/garbage-byte-only when tsv_config stays empty); if
// that happens VsmAutoResolveTesseract() still returns true and leaves
// opt.tsv_config empty.
bool VsmAutoResolveTesseract(VsmTesseractOptions& opt, String& error);

// Runs the full validated per-crop OCR pipeline (original + preprocessed-
// grayscale + Otsu-auto-polarity variants, tesseract `tsv` word-confidence
// scoring, blank-crop detection, confidence-based BestVariant() selection)
// against one file already on disk.
//
// `semantic` drives TextScore()'s keyword-based scoring bonus exactly as in
// the reference tool ("pot_label" / "title" get dedicated keyword bonuses;
// anything else -- including an empty string -- falls back to the generic
// "bb" keyword bonus). Pass whatever semantic-type label is available, or
// an empty string if none is.
//
// `out_dir`, if non-empty, is where the preprocessed/otsu intermediate
// images get saved (as out_dir/ocr_preprocessed/<title>_<semantic>.jpg and
// out_dir/ocr_otsu/<title>_<semantic>.jpg, <title> = GetFileTitle(file_path))
// so a caller that wants to keep them around (e.g. VideoSemanticOcrProbe,
// for its own diagnostic JSON output / gallery tooling) can. When empty
// (the common case for a production engine that only wants OCR text back),
// the intermediate images are written to and read back from temp files that
// are deleted before this function returns -- tesseract only ever operates
// on a real file path, never in-memory, so *some* file always gets written
// for each variant, but it need not outlive the call.
VsmTesseractOcrResult VsmRunTesseractOcr(const VsmTesseractOptions& opt,
                                          const String& file_path, const String& semantic,
                                          const String& out_dir = String());

} // namespace Upp

#endif
