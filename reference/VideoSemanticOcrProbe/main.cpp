#include "VideoSemanticOcrProbe.h"

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

// Task 0277: bridges this tool's CLI-facing OcrProbeOptions onto the shared
// uppsrc/VisualStateModel/TesseractOcr.h options struct. The two explicit-
// override booleans are carried straight through so VsmAutoResolveTesseract()
// reproduces ResolveTessdataDir()/ResolveTsvConfig()'s exact old behavior
// (an explicit --tessdata-dir/--tsv-config is verified as-is; the default,
// non-explicit value is only the FIRST of several candidates a full scan
// tries -- confirmed necessary for this project's own real environment,
// whose tessdata actually lives under tmp/tessdata, not the hardcoded
// default path).
static VsmTesseractOptions ToTesseractOptions(const OcrProbeOptions& opt)
{
	VsmTesseractOptions t;
	t.tesseract             = opt.tesseract;
	t.tessdata_dir          = opt.tessdata_dir;
	t.tessdata_dir_explicit = opt.tessdata_dir_explicit;
	t.lang                  = opt.lang;
	t.psm                   = opt.psm;
	t.preprocess            = opt.preprocess;
	t.otsu                  = opt.otsu;
	t.tsv_config            = opt.tsv_config;
	t.tsv_config_explicit   = opt.tsv_config_explicit;
	return t;
}

// Task 0277: RunCrop() now just bridges to the shared, already-validated
// VsmRunTesseractOcr() pipeline (uppsrc/VisualStateModel/TesseractOcr.h --
// ported 1:1 from this file's own former PreprocessCrop/PreprocessCropOtsu/
// RunTesseract/RunTesseractTsv/TextScore/BestVariant/BestText/BestExitCode)
// and copies its result back into the local OcrResult shape this file's own
// JSON writer already expects. `tocr` is resolved once in RunProbe() and
// passed in so every crop reuses the same resolved tessdata_dir/tsv_config.
static OcrResult RunCrop(const OcrProbeOptions& opt, const VsmTesseractOptions& tocr,
                          const OcrCrop& crop)
{
	OcrResult result;
	result.crop = crop;

	VsmTesseractOcrResult r = VsmRunTesseractOcr(tocr, crop.path, crop.semantic, opt.tracker_dir);

	result.original_text          = r.original_text;
	result.original_exit_code     = r.original_exit_code;
	result.original_avg_conf      = r.original_avg_conf;
	result.preprocessed_text      = r.preprocessed_text;
	result.preprocessed_path      = r.preprocessed_path;
	result.preprocessed_exit_code = r.preprocessed_exit_code;
	result.preprocessed_avg_conf  = r.preprocessed_avg_conf;
	result.otsu_text              = r.otsu_text;
	result.otsu_path              = r.otsu_path;
	result.otsu_exit_code         = r.otsu_exit_code;
	result.otsu_avg_conf          = r.otsu_avg_conf;
	result.otsu_invert            = r.otsu_invert;
	result.otsu_confident         = r.otsu_confident;
	result.crop_stddev            = r.crop_stddev;
	result.blank_detected         = r.blank_detected;
	result.best_text              = r.text;
	result.best_exit_code         = r.exit_code;
	result.best_variant           = r.best_variant;

	return result;
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
	// Task 0277: language/tsv resolution now goes through the shared
	// VsmAutoResolveTesseract() (ports VerifyTesseractLanguage()'s +
	// ResolveTsvConfig()'s exact candidate search + fatal/non-fatal split:
	// missing language data is fatal, a missing tsv config only disables
	// confidence-based scoring). The resolved tessdata_dir/tsv_config are
	// copied back into `opt` so this function's own JSON output (and
	// SaveErrorJson()) keep reporting the real resolved values.
	VsmTesseractOptions tocr = ToTesseractOptions(opt);
	String resolve_error;
	if(!VsmAutoResolveTesseract(tocr, resolve_error)) {
		Cerr() << "ERROR: " << resolve_error << "\n";
		Cerr() << "Install the language data or pass --tessdata-dir and --lang.\n";
		SaveErrorJson(opt, resolve_error);
		return false;
	}
	opt.tessdata_dir = tocr.tessdata_dir;
	opt.tsv_config    = tocr.tsv_config;
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
		OcrResult result = RunCrop(opt, tocr, crops[i]);
		String best_text = result.best_text;
		int best_exit_code = result.best_exit_code;
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
		     << ", \"best_variant\": " << result.best_variant
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
