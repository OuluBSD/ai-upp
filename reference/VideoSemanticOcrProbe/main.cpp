#include "VideoSemanticOcrProbe.h"
#include "OtsuPreprocess.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoSemanticOcrProbe\n\n"
	       << "Usage: VideoSemanticOcrProbe --tracker-dir <dir> [options]\n\n"
	       << "Options:\n"
	       << "  --tracker-dir <dir>   VideoWindowTracker output directory\n"
	       << "  --crop-list <file>    Tab-separated manifest \"semantic<TAB>path\" per line;\n"
	       << "                        bypasses the semantic/ directory scan and OCRs exactly\n"
	       << "                        the listed crop files (e.g. changed_regions/ crops\n"
	       << "                        selected by an external classifier). Requires --out.\n"
	       << "  --out <file>          JSON output path (default <dir>/ocr_probe.json)\n"
	       << "  --tesseract <exe>     Tesseract executable path\n"
	       << "  --tessdata-dir <dir>  Tesseract tessdata directory\n"
	       << "  --lang <name>         OCR language (default eng)\n"
	       << "  --psm <mode>          Tesseract page segmentation mode (default 6)\n"
	       << "  --max-crops <count>   Maximum crop files to OCR (default 40)\n"
	       << "  --preprocess          OCR original plus preprocessed crop (default)\n"
	       << "  --no-preprocess       OCR original crop only\n"
	       << "  --otsu                Also OCR an Otsu-binarized crop with automatic\n"
	       << "                        light/dark polarity detection (default)\n"
	       << "  --no-otsu             Skip the Otsu variant\n"
	       << "  --tsv-config <path>   Path to tesseract's `tsv` config file (Task 0273).\n"
	       << "                        Default: resolved as <tesseract-exe-dir>/tessdata/\n"
	       << "                        configs/tsv (the system Tesseract-OCR install's own\n"
	       << "                        config, independent of --tessdata-dir which only\n"
	       << "                        needs to hold the language *.traineddata file).\n"
	       << "  --help, -h            Show help\n";
}

static OcrProbeOptions ParseOptions(const Vector<String>& args)
{
	OcrProbeOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--tracker-dir" && i + 1 < args.GetCount())
			opt.tracker_dir = args[++i];
		else if(args[i] == "--crop-list" && i + 1 < args.GetCount())
			opt.crop_list = args[++i];
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_path = args[++i];
		else if(args[i] == "--tesseract" && i + 1 < args.GetCount())
			opt.tesseract = args[++i];
		else if(args[i] == "--tessdata-dir" && i + 1 < args.GetCount()) {
			opt.tessdata_dir = args[++i];
			opt.tessdata_dir_explicit = true;
		}
		else if(args[i] == "--lang" && i + 1 < args.GetCount())
			opt.lang = args[++i];
		else if(args[i] == "--psm" && i + 1 < args.GetCount())
			opt.psm = max(3, StrInt(args[++i]));
		else if(args[i] == "--max-crops" && i + 1 < args.GetCount())
			opt.max_crops = max(1, StrInt(args[++i]));
		else if(args[i] == "--preprocess")
			opt.preprocess = true;
		else if(args[i] == "--no-preprocess")
			opt.preprocess = false;
		else if(args[i] == "--otsu")
			opt.otsu = true;
		else if(args[i] == "--no-otsu")
			opt.otsu = false;
		else if(args[i] == "--tsv-config" && i + 1 < args.GetCount()) {
			opt.tsv_config = args[++i];
			opt.tsv_config_explicit = true;
		}
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(opt.out_path.IsEmpty() && !opt.tracker_dir.IsEmpty())
		opt.out_path = AppendFileName(opt.tracker_dir, "ocr_probe.json");
	return opt;
}

static String JsonString(const String& s)
{
	String out;
	for(int i = 0; i < s.GetCount(); i++) {
		byte c = s[i];
		if(c == '\\')
			out << "\\\\";
		else if(c == '"')
			out << "\\\"";
		else if(c == '\n')
			out << "\\n";
		else if(c == '\r')
			out << "\\r";
		else if(c == '\t')
			out << "\\t";
		else
			out.Cat(c);
	}
	return out;
}

static bool IsSelectedSemantic(const String& name)
{
	return name == "title" || name == "pot_label" || name == "top_seat" ||
	       name == "bottom_seat" || name == "left_seats" || name == "right_seats" ||
	       name == "left_top_seat" || name == "left_bottom_seat" ||
	       name == "right_top_seat" || name == "right_bottom_seat";
}

static bool ParseSemanticDirName(const String& name, int& frame_index, int& table_id)
{
	Vector<String> part = Split(name, '_');
	if(part.GetCount() != 4 || part[0] != "frame" || part[2] != "table")
		return false;
	frame_index = StrInt(part[1]);
	table_id = StrInt(part[3]);
	return true;
}

// Parses changed_regions crop filenames of the form
// "frame_000014_table_1_region_02.jpg" to recover frame_index/table_id.
// Falls back to 0/0 if the name doesn't match (still OCRs the file; only
// affects the frame/table fields and the preprocessed-output subfolder name).
static void ParseChangedRegionFileName(const String& name, int& frame_index, int& table_id)
{
	frame_index = 0;
	table_id = 0;
	Vector<String> part = Split(GetFileTitle(name), '_');
	// part: ["frame", "000014", "table", "1", "region", "02"]
	for(int i = 0; i + 1 < part.GetCount(); i++) {
		if(part[i] == "frame")
			frame_index = StrInt(part[i + 1]);
		else if(part[i] == "table")
			table_id = StrInt(part[i + 1]);
	}
}

static Vector<OcrCrop> LoadCropList(const String& list_path)
{
	Vector<OcrCrop> crops;
	String content = LoadFile(list_path);
	if(IsNull(content)) {
		Cerr() << "ERROR: failed to read --crop-list file: " << list_path << "\n";
		return crops;
	}
	Vector<String> lines = Split(content, '\n');
	for(String line : lines) {
		line = TrimBoth(line);
		if(line.IsEmpty())
			continue;
		int tab = line.Find('\t');
		if(tab < 0) {
			Cerr() << "WARNING: skipping malformed --crop-list line (expected "
			          "\"semantic<TAB>path\"): " << line << "\n";
			continue;
		}
		String semantic = TrimBoth(line.Left(tab));
		String path = TrimBoth(line.Mid(tab + 1));
		if(!FileExists(path)) {
			Cerr() << "WARNING: --crop-list path does not exist, skipping: " << path << "\n";
			continue;
		}
		OcrCrop& crop = crops.Add();
		crop.semantic = semantic;
		crop.path = path;
		ParseChangedRegionFileName(GetFileName(path), crop.frame_index, crop.table_id);
	}
	return crops;
}

static Vector<OcrCrop> FindCrops(const OcrProbeOptions& opt)
{
	Vector<OcrCrop> crops;
	String semantic_root = AppendFileName(opt.tracker_dir, "semantic");
	FindFile dir_ff(AppendFileName(semantic_root, "*"));
	while(dir_ff && crops.GetCount() < opt.max_crops) {
		if(dir_ff.IsFolder()) {
			int frame_index = 0;
			int table_id = 0;
			if(ParseSemanticDirName(dir_ff.GetName(), frame_index, table_id)) {
				String table_dir = AppendFileName(semantic_root, dir_ff.GetName());
				FindFile crop_ff(AppendFileName(table_dir, "*.jpg"));
				while(crop_ff && crops.GetCount() < opt.max_crops) {
					if(crop_ff.IsFile()) {
						String semantic = GetFileTitle(crop_ff.GetName());
						if(IsSelectedSemantic(semantic)) {
							OcrCrop& crop = crops.Add();
							crop.frame_index = frame_index;
							crop.table_id = table_id;
							crop.semantic = semantic;
							crop.path = AppendFileName(table_dir, crop_ff.GetName());
						}
					}
					crop_ff.Next();
				}
			}
		}
		dir_ff.Next();
	}
	return crops;
}

static int ScaleForSemantic(const String& semantic)
{
	if(semantic == "title")
		return 3;
	if(semantic == "pot_label")
		return 4;
	return 2;
}

static Image GrayscaleImage(const Image& image)
{
	ImageBuffer out(image.GetSize());
	for(int y = 0; y < image.GetHeight(); y++) {
		const RGBA *src = image[y];
		RGBA *dst = out[y];
		for(int x = 0; x < image.GetWidth(); x++) {
			byte g = (byte)Grayscale(src[x]);
			dst[x].r = g;
			dst[x].g = g;
			dst[x].b = g;
			dst[x].a = 255;
		}
	}
	return out;
}

static String PreprocessCrop(const OcrProbeOptions& opt, const OcrCrop& crop)
{
	if(!opt.preprocess)
		return String();
	Image image = StreamRaster::LoadFileAny(crop.path);
	if(IsNull(image)) {
		Cerr() << "WARNING: failed to load crop for preprocessing: " << crop.path << "\n";
		return String();
	}
	int scale = ScaleForSemantic(crop.semantic);
	Size target(image.GetWidth() * scale, image.GetHeight() * scale);
	Image resized = RescaleFilter(image, target, FILTER_BILINEAR);
	Image gray = GrayscaleImage(resized);
	String out_dir = AppendFileName(opt.tracker_dir, "ocr_preprocessed");
	// Bug found 2026-07-18 (Task 0275): using only frame_index/table_id here
	// collided every --crop-list entry whose filename doesn't match the
	// "frame_NNNNNN_table_N_..." pattern (e.g. externally-refined crops like
	// "h1_f29_balance.jpg") onto the SAME frame_000000_table_0 subfolder, so
	// unrelated crops of the same semantic silently overwrote each other's
	// preprocessed/otsu output file on disk -- the JSON text for each crop
	// was still correct (captured immediately after that crop's own
	// tesseract run), but a diagnostic gallery built afterward from these
	// paths could display a LATER crop's image under an EARLIER crop's
	// row. Always fold in the source crop's own file title so the output
	// path is unique per crop file, never just per frame/table guess.
	String table_dir = AppendFileName(out_dir, Format("frame_%06d_table_%d_%s",
	                                                  crop.frame_index, crop.table_id,
	                                                  GetFileTitle(crop.path)));
	RealizeDirectory(table_dir);
	String out_path = AppendFileName(table_dir, crop.semantic + ".jpg");
	if(!JPGEncoder().Quality(95).SaveFile(out_path, gray)) {
		Cerr() << "WARNING: failed to save preprocessed crop: " << out_path << "\n";
		return String();
	}
	return out_path;
}

// Task 0272: Otsu-binarize the crop, choosing normal-vs-inverse polarity
// via OcrDetectPolarity()'s Sobel-gradient predictive method, then save it
// alongside the existing grayscale/rescale preprocessed variant so both can
// be OCR'd and compared.
static String PreprocessCropOtsu(const OcrProbeOptions& opt, const OcrCrop& crop,
                                  bool& out_invert, bool& out_confident)
{
	out_invert = false;
	out_confident = false;
	if(!opt.otsu)
		return String();
	Image image = StreamRaster::LoadFileAny(crop.path);
	if(IsNull(image)) {
		Cerr() << "WARNING: failed to load crop for Otsu preprocessing: " << crop.path << "\n";
		return String();
	}
	int scale = ScaleForSemantic(crop.semantic);
	Size target(image.GetWidth() * scale, image.GetHeight() * scale);
	Image resized = RescaleFilter(image, target, FILTER_BILINEAR);
	Image gray = OcrGrayscale(resized);
	OcrPolarityResult polarity = OcrDetectPolarity(gray);
	out_invert = polarity.invert;
	out_confident = polarity.confident;
	Image bw = OcrBinarize(gray, OcrOtsuThreshold(gray));
	Image result = polarity.invert ? OcrInvert(bw) : bw;
	String out_dir = AppendFileName(opt.tracker_dir, "ocr_otsu");
	// Same collision fix as PreprocessCrop() above -- see comment there.
	String table_dir = AppendFileName(out_dir, Format("frame_%06d_table_%d_%s",
	                                                  crop.frame_index, crop.table_id,
	                                                  GetFileTitle(crop.path)));
	RealizeDirectory(table_dir);
	String out_path = AppendFileName(table_dir, crop.semantic + ".jpg");
	if(!JPGEncoder().Quality(95).SaveFile(out_path, result)) {
		Cerr() << "WARNING: failed to save Otsu-preprocessed crop: " << out_path << "\n";
		return String();
	}
	return out_path;
}

static String RunTesseract(const OcrProbeOptions& opt, const String& path, int& exit_code)
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

// Task 0273 Phase 1: resolve the `tsv` config file's path. This is
// independent of tessdata_dir (which only needs to hold <lang>.traineddata,
// and per this tool's own local/fallback candidates in
// GetTessdataCandidates() below, generally does NOT also contain the
// configs/ subtree). Confirmed during scoping: the system Tesseract-OCR
// install's own tessdata/configs/tsv file works fine even when a *different*
// --tessdata-dir supplies the language data, so the natural place to find it
// is next to whichever tesseract executable is actually being run.
static bool ResolveTsvConfig(OcrProbeOptions& opt, String& error)
{
	if(opt.tsv_config_explicit) {
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

// Task 0273 Phase 1: run tesseract with the `tsv` config and parse its
// tab-separated word table into level-5 (word-level) OcrWordBox rects.
// Verified end to end during implementation on a real crop
// (frame_000027_table_1_region_00.jpg, 120x48): tesseract's own bbox for
// the word "130.555'" came back as left=15 top=0 width=105 height=19,
// confirming the crop's text sits flush against the top edge (top=0) --
// exactly the boundary bug this task fixes.
static Vector<OcrWordBox> RunTesseractTsv(const OcrProbeOptions& opt, const String& path,
                                           int& exit_code, double& out_avg_conf)
{
	Vector<OcrWordBox> words;
	out_avg_conf = -1;
	if(opt.tsv_config.IsEmpty()) {
		exit_code = -1;
		return words;
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
		return words;
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
		OcrWordBox& w = words.Add();
		w.left = StrInt(col[6]);
		w.top = StrInt(col[7]);
		w.width = StrInt(col[8]);
		w.height = StrInt(col[9]);
		w.conf = StrDbl(col[10]);
		w.text = col.GetCount() > 11 ? col[11] : String();
		if(w.conf >= 0) {
			conf_sum += w.conf;
			conf_count++;
		}
	}
	if(conf_count > 0)
		out_avg_conf = conf_sum / conf_count;
	return words;
}

static bool SaveErrorJson(const OcrProbeOptions& opt, const String& error)
{
	String json;
	json << "{\n";
	json << "  \"ok\": false,\n";
	json << "  \"tracker_dir\": \"" << JsonString(opt.tracker_dir) << "\",\n";
	json << "  \"tesseract\": \"" << JsonString(opt.tesseract) << "\",\n";
	json << "  \"tessdata_dir\": \"" << JsonString(opt.tessdata_dir) << "\",\n";
	json << "  \"lang\": \"" << JsonString(opt.lang) << "\",\n";
	json << "  \"error\": \"" << JsonString(error) << "\",\n";
	json << "  \"crop_count\": 0,\n";
	json << "  \"results\": []\n";
	json << "}\n";
	if(!SaveFile(opt.out_path, json))
		return false;
	Cout() << "ocr_probe_json=" << opt.out_path << "\n";
	return true;
}

static String LanguageDataPath(const String& dir, const String& lang)
{
	return AppendFileName(dir, lang + ".traineddata");
}

static bool HasLanguageData(const String& dir, const String& lang)
{
	return !dir.IsEmpty() && FileExists(LanguageDataPath(dir, lang));
}

static Vector<String> GetTessdataCandidates(const OcrProbeOptions& opt)
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

static bool ResolveTessdataDir(OcrProbeOptions& opt, String& error)
{
	if(opt.tessdata_dir_explicit) {
		if(HasLanguageData(opt.tessdata_dir, opt.lang))
			return true;
		error = "missing tesseract language data: " + LanguageDataPath(opt.tessdata_dir, opt.lang);
		return false;
	}
	Vector<String> candidates = GetTessdataCandidates(opt);
	for(const String& candidate : candidates) {
		if(HasLanguageData(candidate, opt.lang)) {
			opt.tessdata_dir = candidate;
			return true;
		}
	}
	error = "missing tesseract language data for '" + opt.lang + "'; checked:";
	for(const String& candidate : candidates)
		error << " " << LanguageDataPath(candidate, opt.lang) << ";";
	return false;
}

static bool VerifyTesseractLanguage(OcrProbeOptions& opt, String& error)
{
	if(ResolveTessdataDir(opt, error))
		return true;
	String trained = LanguageDataPath(opt.tessdata_dir, opt.lang);
	if(!FileExists(trained)) {
		Cerr() << "ERROR: " << error << "\n";
		Cerr() << "Install the language data or pass --tessdata-dir and --lang.\n";
		return false;
	}
	return true;
}

// Task 0274 Phase 4: below this grayscale-stddev-of-the-original-crop
// value, treat the crop as blank (no text at all) rather than trusting
// whatever OCR guessed on Otsu-amplified noise. Validated against the real
// 31-crop dataset: the one genuine blank crop measured ~4.4; every crop
// with real text measured >= 16.08. See OtsuPreprocess.h for detail.
static const double kBlankStdDevThreshold = 10.0;

static OcrResult RunCrop(const OcrProbeOptions& opt, const OcrCrop& crop)
{
	OcrResult result;
	result.crop = crop;
	{
		Image image = StreamRaster::LoadFileAny(crop.path);
		if(!IsNull(image)) {
			result.crop_stddev = OcrGrayscaleStdDev(image);
			result.blank_detected = result.crop_stddev >= 0 &&
			                         result.crop_stddev < kBlankStdDevThreshold;
		}
	}
	result.original_text = RunTesseract(opt, crop.path, result.original_exit_code);
	result.preprocessed_path = PreprocessCrop(opt, crop);
	if(!result.preprocessed_path.IsEmpty())
		result.preprocessed_text = RunTesseract(opt, result.preprocessed_path,
		                                        result.preprocessed_exit_code);
	result.otsu_path = PreprocessCropOtsu(opt, crop, result.otsu_invert, result.otsu_confident);
	if(!result.otsu_path.IsEmpty())
		result.otsu_text = RunTesseract(opt, result.otsu_path, result.otsu_exit_code);
	// Task 0273 Phase 1/5: gather word bboxes + avg conf for every variant
	// that actually ran, so BestVariant() (Phase 5) can score by real OCR
	// confidence instead of raw text length.
	if(!opt.tsv_config.IsEmpty()) {
		int tsv_exit;
		result.original_words = RunTesseractTsv(opt, crop.path, tsv_exit, result.original_avg_conf);
		if(!result.preprocessed_path.IsEmpty())
			result.preprocessed_words = RunTesseractTsv(opt, result.preprocessed_path, tsv_exit,
			                                            result.preprocessed_avg_conf);
		if(!result.otsu_path.IsEmpty())
			result.otsu_words = RunTesseractTsv(opt, result.otsu_path, tsv_exit, result.otsu_avg_conf);
	}
	return result;
}

// Task 0273 Phase 5: replaces the old "longer text wins" heuristic
// (text.GetCount() / 8, documented as buggy in Task 0271/0272 -- it
// systematically preferred a longer-but-wrong string, e.g. a hallucinated
// extra digit in "99.55 BB" beating the correct "99.5 BB" by exactly the
// margin contributed by that one bogus character) with tesseract's own
// average per-word recognition confidence (Task 0273 Phase 1's `tsv`
// output) as the primary score.
//
// Empirically validated (not just theorized) against real per-word conf
// captured for all 31 crops in the dataset before landing on this formula:
//   - avg_conf as the base score does NOT regress any of the 13 frames
//     Task 0271/0272 already scored EXACT/correct.
//   - it DOES surface hand2 frame33's previously-masked correct Otsu/
//     original read ("99.5 BB", avg_conf ~88/~86) over the wrong
//     "99.55 BB" (avg_conf ~86 preprocessed) that length-based scoring
//     picked before, because the correct short reading is *also* the more
//     confident one there.
//   - it does NOT fix hand2 frame53 ("186.4 BB" vs "166.4 BB", an 8-vs-6
//     digit misread): tesseract is genuinely, if wrongly, more confident
//     in the misread "166.4" (avg_conf ~89) than the correct Otsu "186.4"
//     (avg_conf ~85) on that crop. No amount of score-formula tuning fixes
//     this from word-level confidence alone (tsv only reports word-level,
//     not per-glyph, confidence) -- it's an honest limitation, not
//     something worth gaming the formula to hide.
// A garbage-byte penalty is added on top: real balance/pot/bet text is
// plain ASCII (digits, letters, "$", ":", " "). Mojibake/garbled OCR output
// on contaminated or too-loose crops routinely contains high-byte/non-ASCII
// characters (e.g. the literal UTF-8 bytes tesseract emits for
// "130.5565 <mojibake>" on the too-loose frame27 crop); these never appear
// in a genuinely correct read, so they're penalized directly instead of
// being indirectly favored by whichever variant happens to have fewer of
// them via raw length.
static double TextScore(const String& semantic, const String& text, double avg_conf)
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

// Picks the best of the up-to-3 OCR variants (original / grayscale-
// rescaled / Otsu-auto-polarity) by TextScore(). Preserves the pre-existing
// (original vs. preprocessed) tie-break precedent exactly: a later variant
// only wins if its score is STRICTLY greater than the best score seen so
// far, so on any tie the earlier/simpler variant (original first, then
// preprocessed) keeps winning.
static int BestVariant(const OcrResult& result)
{
	double s_orig = TextScore(result.crop.semantic, result.original_text, result.original_avg_conf);
	double s_prep = TextScore(result.crop.semantic, result.preprocessed_text, result.preprocessed_avg_conf);
	double s_otsu = TextScore(result.crop.semantic, result.otsu_text, result.otsu_avg_conf);
	int best = 0;
	double best_score = s_orig;
	if(s_prep > best_score) { best = 1; best_score = s_prep; }
	if(s_otsu > best_score) { best = 2; best_score = s_otsu; }
	return best;
}

// Task 0274 Phase 4: a blank crop should be reported as such, not as
// whatever OCR hallucinated on Otsu-amplified noise.
static const char* kNoTextDetected = "(no text detected -- blank crop)";

static String BestText(const OcrResult& result)
{
	if(result.blank_detected)
		return kNoTextDetected;
	switch(BestVariant(result)) {
	case 2: return result.otsu_text;
	case 1: return result.preprocessed_text;
	default: return result.original_text;
	}
}

static int BestExitCode(const OcrResult& result)
{
	switch(BestVariant(result)) {
	case 2: return result.otsu_exit_code;
	case 1: return result.preprocessed_exit_code;
	default: return result.original_exit_code;
	}
}

static bool RunProbe(OcrProbeOptions opt)
{
	if(!FileExists(opt.tesseract)) {
		Cerr() << "ERROR: missing tesseract executable: " << opt.tesseract << "\n";
		SaveErrorJson(opt, "missing tesseract executable: " + opt.tesseract);
		return false;
	}
	if(!opt.crop_list.IsEmpty() && opt.out_path.IsEmpty()) {
		Cerr() << "ERROR: --crop-list requires an explicit --out path\n";
		return false;
	}
	String language_error;
	if(!VerifyTesseractLanguage(opt, language_error)) {
		SaveErrorJson(opt, language_error);
		return false;
	}
	// Task 0273 Phase 1: resolve the `tsv` config file so word bboxes +
	// per-word confidence are available for BestVariant() (Phase 5). Not
	// fatal if missing -- TextScore() degrades gracefully (avg_conf stays
	// -1, so scoring falls back to just the keyword/garbage-byte terms)
	// rather than refusing to run the whole probe.
	String tsv_error;
	if(!ResolveTsvConfig(opt, tsv_error))
		Cerr() << "WARNING: " << tsv_error << " -- word-confidence scoring disabled\n";
	Vector<OcrCrop> crops = opt.crop_list.IsEmpty() ? FindCrops(opt) : LoadCropList(opt.crop_list);
	String json;
	json << "{\n";
	json << "  \"ok\": true,\n";
	json << "  \"tracker_dir\": \"" << JsonString(opt.tracker_dir) << "\",\n";
	json << "  \"tesseract\": \"" << JsonString(opt.tesseract) << "\",\n";
	json << "  \"tessdata_dir\": \"" << JsonString(opt.tessdata_dir) << "\",\n";
	json << "  \"lang\": \"" << JsonString(opt.lang) << "\",\n";
	json << "  \"psm\": " << opt.psm << ",\n";
	json << "  \"preprocess\": " << (opt.preprocess ? "true" : "false") << ",\n";
	json << "  \"otsu\": " << (opt.otsu ? "true" : "false") << ",\n";
	json << "  \"crop_count\": " << crops.GetCount() << ",\n";
	json << "  \"results\": [\n";
	for(int i = 0; i < crops.GetCount(); i++) {
		OcrResult result = RunCrop(opt, crops[i]);
		String best_text = BestText(result);
		int best_exit_code = BestExitCode(result);
		Cout() << "ocr frame=" << crops[i].frame_index
		       << " table=" << crops[i].table_id
		       << " semantic=" << crops[i].semantic
		       << " original_exit=" << result.original_exit_code
		       << " preprocessed_exit=" << result.preprocessed_exit_code
		       << " text=\"" << best_text << "\"\n";
		if(i)
			json << ",\n";
		json << "    {\"frame_index\": " << crops[i].frame_index
		     << ", \"table_id\": " << crops[i].table_id
		     << ", \"semantic\": \"" << JsonString(crops[i].semantic)
		     << "\", \"path\": \"" << JsonString(crops[i].path)
		     << "\", \"preprocessed_path\": \"" << JsonString(result.preprocessed_path)
		     << "\", \"exit_code\": " << best_exit_code
		     << ", \"text\": \"" << JsonString(best_text)
		     << "\", \"original_exit_code\": " << result.original_exit_code
		     << ", \"original_text\": \"" << JsonString(result.original_text)
		     << "\", \"preprocessed_exit_code\": " << result.preprocessed_exit_code
		     << ", \"preprocessed_text\": \"" << JsonString(result.preprocessed_text)
		     << "\", \"otsu_path\": \"" << JsonString(result.otsu_path)
		     << "\", \"otsu_exit_code\": " << result.otsu_exit_code
		     << ", \"otsu_text\": \"" << JsonString(result.otsu_text)
		     << "\", \"otsu_invert\": " << (result.otsu_invert ? "true" : "false")
		     << ", \"otsu_confident\": " << (result.otsu_confident ? "true" : "false")
		     << ", \"best_variant\": " << BestVariant(result)
		     << ", \"original_avg_conf\": " << result.original_avg_conf
		     << ", \"preprocessed_avg_conf\": " << result.preprocessed_avg_conf
		     << ", \"otsu_avg_conf\": " << result.otsu_avg_conf
		     << ", \"crop_stddev\": " << result.crop_stddev
		     << ", \"blank_detected\": " << (result.blank_detected ? "true" : "false") << "}";
	}
	json << "\n  ]\n";
	json << "}\n";
	if(!SaveFile(opt.out_path, json)) {
		Cerr() << "ERROR: failed to write " << opt.out_path << "\n";
		return false;
	}
	Cout() << "ocr_probe_json=" << opt.out_path << "\n";
	return true;
}

// Task 0272 Phase 2 validation: every real crop in the 31-crop dataset this
// tool was validated against happens to share the same true polarity
// (light text on a dark/colored background) -- so on real data alone,
// "always predict invert=true" would score 100% too, and would be
// indistinguishable from a genuinely working detector. This synthetic
// self-test constructs two minimal images with a KNOWN, opposite, and
// unambiguous true polarity (a solid dark block on a light background,
// and a solid light block on a dark background -- standing in for a thick
// glyph stroke) and checks OcrDetectPolarity() picks the correct answer on
// both, so the algorithm is verified to actually discriminate, not just
// agree with a dataset that happens to be monotone.
static bool SelfTestPolarity()
{
	bool ok = true;

	// Case A: dark text (block) on a light background -> must NOT invert.
	{
		int w = 40, h = 20;
		ImageBuffer ib(w, h);
		for(int y = 0; y < h; y++)
			for(int x = 0; x < w; x++) {
				RGBA& p = ib[y][x];
				p.r = p.g = p.b = 230; p.a = 255;
			}
		for(int y = 5; y < 15; y++)
			for(int x = 10; x < 30; x++) {
				RGBA& p = ib[y][x];
				p.r = p.g = p.b = 20; p.a = 255;
			}
		Image gray = OcrGrayscale((Image)ib);
		OcrPolarityResult r = OcrDetectPolarity(gray);
		Cout() << "selftest dark_text_on_light_bg: invert=" << (r.invert ? "true" : "false")
		       << " confident=" << (r.confident ? "true" : "false")
		       << " edge_mean=" << r.edge_mean << " background=" << r.background_level
		       << " edge_pixels=" << r.edge_pixel_count << "\n";
		if(r.invert || !r.confident) {
			Cerr() << "SELF-TEST FAILED: expected invert=false, confident=true\n";
			ok = false;
		}
	}

	// Case B: light text (block) on a dark background -> must invert.
	{
		int w = 40, h = 20;
		ImageBuffer ib(w, h);
		for(int y = 0; y < h; y++)
			for(int x = 0; x < w; x++) {
				RGBA& p = ib[y][x];
				p.r = p.g = p.b = 20; p.a = 255;
			}
		for(int y = 5; y < 15; y++)
			for(int x = 10; x < 30; x++) {
				RGBA& p = ib[y][x];
				p.r = p.g = p.b = 230; p.a = 255;
			}
		Image gray = OcrGrayscale((Image)ib);
		OcrPolarityResult r = OcrDetectPolarity(gray);
		Cout() << "selftest light_text_on_dark_bg: invert=" << (r.invert ? "true" : "false")
		       << " confident=" << (r.confident ? "true" : "false")
		       << " edge_mean=" << r.edge_mean << " background=" << r.background_level
		       << " edge_pixels=" << r.edge_pixel_count << "\n";
		if(!r.invert || !r.confident) {
			Cerr() << "SELF-TEST FAILED: expected invert=true, confident=true\n";
			ok = false;
		}
	}

	Cout() << (ok ? "selftest: PASS\n" : "selftest: FAIL\n");
	return ok;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	for(const String& a : args)
		if(a == "--self-test-polarity") {
			if(!SelfTestPolarity())
				SetExitCode(1);
			return;
		}
	OcrProbeOptions opt = ParseOptions(args);
	if(opt.help || opt.tracker_dir.IsEmpty()) {
		PrintHelp();
		if(opt.tracker_dir.IsEmpty() && !opt.help)
			SetExitCode(1);
		return;
	}
	if(!RunProbe(opt))
		SetExitCode(1);
}
