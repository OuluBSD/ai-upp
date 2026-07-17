#include "VisualStateModel.h"
#include "TesseractOcr.h"
#include "OtsuPreprocess.h"
#include <plugin/jpg/jpg.h>

namespace Upp {

// ---------------------------------------------------------------------------
// Semantic-driven preprocessing scale, ported verbatim from
// VideoSemanticOcrProbe/main.cpp's ScaleForSemantic().
static int VsmScaleForSemantic(const String& semantic)
{
	if(semantic == "title")
		return 3;
	if(semantic == "pot_label")
		return 4;
	return 2;
}

// Where to write one preprocessing variant's output image. When `out_dir` is
// non-empty, mirrors the reference tool's persistent ocr_preprocessed/
// ocr_otsu subfolders (but keyed only by the crop's own file title +
// semantic, not frame/table metadata -- Task 0275 already established file
// title alone is the correct uniqueness key). When `out_dir` is empty
// (typical for a production engine caller that only wants OCR text back),
// uses a throwaway temp file that the caller deletes once tesseract is done
// reading it.
static String VsmPreprocessOutputPath(const String& out_dir, const String& subfolder,
                                       const String& file_path, const String& semantic,
                                       bool& is_temp)
{
	is_temp = out_dir.IsEmpty();
	if(is_temp)
		return GetTempFileName() + ".jpg";
	String dir = AppendFileName(out_dir, subfolder);
	RealizeDirectory(dir);
	String title = GetFileTitle(file_path);
	String name = semantic.IsEmpty() ? title : (title + "_" + semantic);
	return AppendFileName(dir, name + ".jpg");
}

// Ported verbatim (grayscale-rescale variant) from VideoSemanticOcrProbe/
// main.cpp's PreprocessCrop(). The reference tool's separate GrayscaleImage()
// helper performed the identical per-pixel Upp::Grayscale() conversion as
// OtsuPreprocess.h's OcrGrayscale(), so this uses that shared copy directly
// instead of duplicating it.
static String VsmPreprocessCrop(const VsmTesseractOptions& opt, const String& file_path,
                                 const String& semantic, const String& out_dir, bool& is_temp)
{
	is_temp = false;
	if(!opt.preprocess)
		return String();
	Image image = StreamRaster::LoadFileAny(file_path);
	if(IsNull(image)) {
		Cerr() << "WARNING: failed to load crop for preprocessing: " << file_path << "\n";
		return String();
	}
	int scale = VsmScaleForSemantic(semantic);
	Size target(image.GetWidth() * scale, image.GetHeight() * scale);
	Image resized = RescaleFilter(image, target, FILTER_BILINEAR);
	Image gray = OcrGrayscale(resized);
	String out_path = VsmPreprocessOutputPath(out_dir, "ocr_preprocessed", file_path, semantic, is_temp);
	if(!JPGEncoder().Quality(95).SaveFile(out_path, gray)) {
		Cerr() << "WARNING: failed to save preprocessed crop: " << out_path << "\n";
		return String();
	}
	return out_path;
}

// Ported verbatim from VideoSemanticOcrProbe/main.cpp's PreprocessCropOtsu()
// (Task 0272): Otsu-binarize with automatic light/dark polarity detection.
static String VsmPreprocessCropOtsu(const VsmTesseractOptions& opt, const String& file_path,
                                     const String& semantic, const String& out_dir,
                                     bool& out_invert, bool& out_confident, bool& is_temp)
{
	out_invert = false;
	out_confident = false;
	is_temp = false;
	if(!opt.otsu)
		return String();
	Image image = StreamRaster::LoadFileAny(file_path);
	if(IsNull(image)) {
		Cerr() << "WARNING: failed to load crop for Otsu preprocessing: " << file_path << "\n";
		return String();
	}
	int scale = VsmScaleForSemantic(semantic);
	Size target(image.GetWidth() * scale, image.GetHeight() * scale);
	Image resized = RescaleFilter(image, target, FILTER_BILINEAR);
	Image gray = OcrGrayscale(resized);
	OcrPolarityResult polarity = OcrDetectPolarity(gray);
	out_invert = polarity.invert;
	out_confident = polarity.confident;
	Image bw = OcrBinarize(gray, OcrOtsuThreshold(gray));
	Image result = polarity.invert ? OcrInvert(bw) : bw;
	String out_path = VsmPreprocessOutputPath(out_dir, "ocr_otsu", file_path, semantic, is_temp);
	if(!JPGEncoder().Quality(95).SaveFile(out_path, result)) {
		Cerr() << "WARNING: failed to save Otsu-preprocessed crop: " << out_path << "\n";
		return String();
	}
	return out_path;
}

// Ported verbatim from VideoSemanticOcrProbe/main.cpp's RunTesseract().
static String VsmRunTesseractPlain(const VsmTesseractOptions& opt, const String& path, int& exit_code)
{
	Vector<String> args;
	args << path << "stdout"
	     << "--tessdata-dir" << opt.tessdata_dir
	     << "--psm" << AsString(opt.psm)
	     << "-l" << opt.lang;
	String out;
	exit_code = Sys(opt.tesseract, args, out);
	return TrimBoth(out);
}

// Ported from VideoSemanticOcrProbe/main.cpp's RunTesseractTsv() (Task 0273
// Phase 1) -- only the avg-confidence output is kept here. The per-word
// bbox table it also parses isn't consumed by anything outside the
// reference tool's own crop-boundary-refinement workflow, so it isn't part
// of this shared API (see TesseractOcr.h's header comment).
static void VsmRunTesseractTsv(const VsmTesseractOptions& opt, const String& path,
                                int& exit_code, double& out_avg_conf)
{
	out_avg_conf = -1;
	if(opt.tsv_config.IsEmpty()) {
		exit_code = -1;
		return;
	}
	Vector<String> args;
	args << path << "stdout"
	     << "--tessdata-dir" << opt.tessdata_dir
	     << "--psm" << AsString(opt.psm)
	     << "-l" << opt.lang
	     << opt.tsv_config;
	String out;
	exit_code = Sys(opt.tesseract, args, out);
	if(exit_code != 0)
		return;
	Vector<String> lines = Split(out, '\n');
	double conf_sum = 0;
	int conf_count = 0;
	for(int i = 0; i < lines.GetCount(); i++) {
		String line = TrimBoth(lines[i]);
		if(line.IsEmpty() || line.StartsWith("level\t"))
			continue;
		Vector<String> col = Split(line, '\t');
		// level page_num block_num par_num line_num word_num left top width height conf text
		if(col.GetCount() < 11)
			continue;
		if(StrInt(col[0]) != 5)
			continue; // only word-level rows carry a real bbox + conf
		double conf = StrDbl(col[10]);
		if(conf >= 0) {
			conf_sum += conf;
			conf_count++;
		}
	}
	if(conf_count > 0)
		out_avg_conf = conf_sum / conf_count;
}

// Task 0273 Phase 5: ported verbatim from VideoSemanticOcrProbe/main.cpp's
// TextScore(). See that file's history for the empirical validation notes
// this formula is based on -- not re-derived here, only ported.
static double VsmTextScore(const String& semantic, const String& text, double avg_conf)
{
	if(text.IsEmpty())
		return -1000;
	double score = avg_conf >= 0 ? avg_conf : 0;
	int junk = 0;
	for(int i = 0; i < text.GetCount(); i++)
		if((byte)text[i] >= 128)
			junk++;
	score -= junk * 15;
	String lower = ToLower(text);
	if(semantic == "pot_label") {
		if(lower.Find("pot") >= 0)
			score += 30;
		if(lower.Find("bb") >= 0)
			score += 10;
	}
	else if(semantic == "title") {
		if(lower.Find("hold") >= 0)
			score += 30;
		if(lower.Find("limit") >= 0)
			score += 20;
		if(text.Find("$") >= 0)
			score += 10;
	}
	else {
		if(lower.Find("bb") >= 0)
			score += 10;
	}
	return score;
}

// Task 0274 Phase 4: blank/uniform-color crop text override -- see
// OtsuPreprocess.h's OcrGrayscaleStdDev() doc comment for the validation
// numbers this threshold is based on.
static const double kBlankStdDevThreshold = 10.0;
static const char*  kNoTextDetected = "(no text detected -- blank crop)";

// ---------------------------------------------------------------------------
// Tesseract/tessdata resolution -- ported from VideoSemanticOcrProbe/
// main.cpp's GetTessdataCandidates()/ResolveTessdataDir()/
// VerifyTesseractLanguage()/ResolveTsvConfig().

static String VsmLanguageDataPath(const String& dir, const String& lang)
{
	return AppendFileName(dir, lang + ".traineddata");
}

static bool VsmHasLanguageData(const String& dir, const String& lang)
{
	return !dir.IsEmpty() && FileExists(VsmLanguageDataPath(dir, lang));
}

static Vector<String> VsmGetTessdataCandidates(const VsmTesseractOptions& opt)
{
	Vector<String> candidates;
	candidates << opt.tessdata_dir;
	String env_tessdata = GetEnv("TESSDATA_PREFIX");
	if(!env_tessdata.IsEmpty()) {
		candidates << env_tessdata;
		candidates << AppendFileName(env_tessdata, "tessdata");
	}
	String exe_dir = GetFileDirectory(GetExeFilePath());
	candidates << AppendFileName(exe_dir, "tessdata");
	candidates << AppendFileName(GetCurrentDirectory(), "tessdata");
	candidates << AppendFileName(AppendFileName(GetCurrentDirectory(), "tmp"), "tessdata");
	candidates << AppendFileName(AppendFileName(GetCurrentDirectory(), "share"), "tessdata");
	String vcpkg_root = GetEnv("VCPKG_ROOT");
	if(vcpkg_root.IsEmpty() && !GetEnv("USERPROFILE").IsEmpty())
		vcpkg_root = AppendFileName(GetEnv("USERPROFILE"), "vcpkg");
	if(!vcpkg_root.IsEmpty()) {
		String vcpkg_share = AppendFileName(AppendFileName(AppendFileName(vcpkg_root, "installed"),
		                                                   "x64-windows"),
		                                    "share");
		candidates << AppendFileName(vcpkg_share, "tessdata");
		candidates << AppendFileName(AppendFileName(vcpkg_share, "tesseract"), "tessdata");
	}
	Vector<String> unique;
	for(const String& candidate : candidates) {
		String normalized = NormalizePath(candidate);
		bool duplicate = false;
		for(const String& existing : unique) {
			if(existing == normalized) {
				duplicate = true;
				break;
			}
		}
		if(!normalized.IsEmpty() && !duplicate)
			unique << normalized;
	}
	return unique;
}

// `explicit_dir` mirrors the reference tool's tessdata_dir_explicit flag: a
// caller-supplied non-empty opt.tessdata_dir on entry is verified directly
// (no candidate scan, hard error if the language data isn't there); an
// empty opt.tessdata_dir scans VsmGetTessdataCandidates() for the first hit.
static bool VsmResolveTessdataDir(VsmTesseractOptions& opt, bool explicit_dir, String& error)
{
	if(explicit_dir) {
		if(VsmHasLanguageData(opt.tessdata_dir, opt.lang))
			return true;
		error = "missing tesseract language data: " + VsmLanguageDataPath(opt.tessdata_dir, opt.lang);
		return false;
	}
	Vector<String> candidates = VsmGetTessdataCandidates(opt);
	for(const String& candidate : candidates) {
		if(VsmHasLanguageData(candidate, opt.lang)) {
			opt.tessdata_dir = candidate;
			return true;
		}
	}
	error = "missing tesseract language data for '" + opt.lang + "'; checked:";
	for(const String& candidate : candidates)
		error << " " << VsmLanguageDataPath(candidate, opt.lang) << ";";
	return false;
}

// Ported from VideoSemanticOcrProbe/main.cpp's ResolveTsvConfig() (Task 0273
// Phase 1). Not fatal if it fails -- see VsmAutoResolveTesseract()'s comment.
static bool VsmResolveTsvConfig(VsmTesseractOptions& opt, bool explicit_tsv, String& error)
{
	if(explicit_tsv) {
		if(FileExists(opt.tsv_config))
			return true;
		error = "missing tesseract tsv config file: " + opt.tsv_config;
		return false;
	}
	Vector<String> candidates;
	String exe_dir = GetFileDirectory(opt.tesseract);
	candidates << AppendFileName(AppendFileName(exe_dir, "tessdata"), AppendFileName("configs", "tsv"));
	candidates << AppendFileName(AppendFileName(opt.tessdata_dir, "configs"), "tsv");
	String env_tessdata = GetEnv("TESSDATA_PREFIX");
	if(!env_tessdata.IsEmpty())
		candidates << AppendFileName(AppendFileName(env_tessdata, "configs"), "tsv");
	for(const String& candidate : candidates) {
		if(FileExists(candidate)) {
			opt.tsv_config = candidate;
			return true;
		}
	}
	error = "missing tesseract tsv config file; checked:";
	for(const String& candidate : candidates)
		error << " " << candidate << ";";
	return false;
}

bool VsmAutoResolveTesseract(VsmTesseractOptions& opt, String& error)
{
	if(!FileExists(opt.tesseract)) {
		error = "missing tesseract executable: " + opt.tesseract;
		return false;
	}
	String tessdata_error;
	if(!VsmResolveTessdataDir(opt, opt.tessdata_dir_explicit, tessdata_error)) {
		error = tessdata_error;
		return false;
	}
	String tsv_error;
	if(!VsmResolveTsvConfig(opt, opt.tsv_config_explicit, tsv_error))
		Cerr() << "WARNING: " << tsv_error << " -- word-confidence scoring disabled\n";
	return true;
}

// ---------------------------------------------------------------------------
// VsmRunTesseractOcr -- ported from VideoSemanticOcrProbe/main.cpp's
// RunCrop() + BestVariant() + BestText() + BestExitCode(), combined into one
// call since none of those helpers is independently useful without the
// others.

VsmTesseractOcrResult VsmRunTesseractOcr(const VsmTesseractOptions& opt,
                                          const String& file_path, const String& semantic,
                                          const String& out_dir)
{
	VsmTesseractOcrResult result;

	{
		Image image = StreamRaster::LoadFileAny(file_path);
		if(!IsNull(image)) {
			result.crop_stddev = OcrGrayscaleStdDev(image);
			result.blank_detected = result.crop_stddev >= 0 &&
			                         result.crop_stddev < kBlankStdDevThreshold;
		}
	}

	result.original_text = VsmRunTesseractPlain(opt, file_path, result.original_exit_code);

	bool prep_is_temp = false;
	String preprocessed_path = VsmPreprocessCrop(opt, file_path, semantic, out_dir, prep_is_temp);
	if(!preprocessed_path.IsEmpty())
		result.preprocessed_text = VsmRunTesseractPlain(opt, preprocessed_path,
		                                                result.preprocessed_exit_code);

	bool otsu_is_temp = false;
	String otsu_path = VsmPreprocessCropOtsu(opt, file_path, semantic, out_dir,
	                                          result.otsu_invert, result.otsu_confident, otsu_is_temp);
	if(!otsu_path.IsEmpty())
		result.otsu_text = VsmRunTesseractPlain(opt, otsu_path, result.otsu_exit_code);

	// Task 0273 Phase 1/5: gather per-variant avg conf (word-level tsv
	// output) for every variant that actually ran, so BestVariant() below
	// can score by real OCR confidence instead of raw text length.
	if(!opt.tsv_config.IsEmpty()) {
		int tsv_exit;
		VsmRunTesseractTsv(opt, file_path, tsv_exit, result.original_avg_conf);
		if(!preprocessed_path.IsEmpty())
			VsmRunTesseractTsv(opt, preprocessed_path, tsv_exit, result.preprocessed_avg_conf);
		if(!otsu_path.IsEmpty())
			VsmRunTesseractTsv(opt, otsu_path, tsv_exit, result.otsu_avg_conf);
	}

	// Expose the intermediate-file paths only when the caller asked for them
	// to be kept (out_dir non-empty); otherwise clean up the temp files now
	// that every tesseract invocation against them has finished.
	if(prep_is_temp) {
		if(!preprocessed_path.IsEmpty())
			DeleteFile(preprocessed_path);
	}
	else
		result.preprocessed_path = preprocessed_path;
	if(otsu_is_temp) {
		if(!otsu_path.IsEmpty())
			DeleteFile(otsu_path);
	}
	else
		result.otsu_path = otsu_path;

	// Task 0273 Phase 5's confidence-based BestVariant() replacement, exact
	// tie-break precedent preserved: a later variant only wins on a STRICT
	// score improvement, so original beats preprocessed beats otsu on a tie.
	double s_orig = VsmTextScore(semantic, result.original_text, result.original_avg_conf);
	double s_prep = VsmTextScore(semantic, result.preprocessed_text, result.preprocessed_avg_conf);
	double s_otsu = VsmTextScore(semantic, result.otsu_text, result.otsu_avg_conf);
	int best = 0;
	double best_score = s_orig;
	if(s_prep > best_score) { best = 1; best_score = s_prep; }
	if(s_otsu > best_score) { best = 2; best_score = s_otsu; }
	result.best_variant = best;

	// BestExitCode()/BestText() ported: the exit code and confidence always
	// follow the winning variant regardless of blank_detected, but the text
	// itself is overridden to the blank-crop marker when blank_detected --
	// exactly the reference tool's split behavior.
	int    variant_exit_code;
	double variant_conf;
	String variant_text;
	switch(best) {
	case 2:
		variant_exit_code = result.otsu_exit_code;
		variant_conf      = result.otsu_avg_conf;
		variant_text       = result.otsu_text;
		break;
	case 1:
		variant_exit_code = result.preprocessed_exit_code;
		variant_conf      = result.preprocessed_avg_conf;
		variant_text       = result.preprocessed_text;
		break;
	default:
		variant_exit_code = result.original_exit_code;
		variant_conf      = result.original_avg_conf;
		variant_text       = result.original_text;
		break;
	}
	result.exit_code  = variant_exit_code;
	result.confidence = variant_conf;
	result.text        = result.blank_detected ? kNoTextDetected : variant_text;

	return result;
}

} // namespace Upp
