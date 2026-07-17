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
	String table_dir = AppendFileName(out_dir, Format("frame_%06d_table_%d",
	                                                  crop.frame_index, crop.table_id));
	RealizeDirectory(table_dir);
	String out_path = AppendFileName(table_dir, crop.semantic + ".jpg");
	if(!JPGEncoder().Quality(95).SaveFile(out_path, gray)) {
		Cerr() << "WARNING: failed to save preprocessed crop: " << out_path << "\n";
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

static OcrResult RunCrop(const OcrProbeOptions& opt, const OcrCrop& crop)
{
	OcrResult result;
	result.crop = crop;
	result.original_text = RunTesseract(opt, crop.path, result.original_exit_code);
	result.preprocessed_path = PreprocessCrop(opt, crop);
	if(!result.preprocessed_path.IsEmpty())
		result.preprocessed_text = RunTesseract(opt, result.preprocessed_path,
		                                        result.preprocessed_exit_code);
	return result;
}

static int TextScore(const String& semantic, const String& text)
{
	if(text.IsEmpty())
		return -1000;
	int score = text.GetCount() / 8;
	String lower = ToLower(text);
	if(semantic == "pot_label") {
		if(lower.Find("pot") >= 0)
			score += 100;
		if(lower.Find("bb") >= 0)
			score += 20;
	}
	else if(semantic == "title") {
		if(lower.Find("hold") >= 0)
			score += 60;
		if(lower.Find("limit") >= 0)
			score += 40;
		if(text.Find("$") >= 0)
			score += 20;
	}
	else {
		if(lower.Find("bb") >= 0)
			score += 20;
	}
	return score;
}

static String BestText(const OcrResult& result)
{
	return TextScore(result.crop.semantic, result.preprocessed_text) >
	       TextScore(result.crop.semantic, result.original_text) ?
	       result.preprocessed_text : result.original_text;
}

static int BestExitCode(const OcrResult& result)
{
	return TextScore(result.crop.semantic, result.preprocessed_text) >
	       TextScore(result.crop.semantic, result.original_text) ?
	       result.preprocessed_exit_code : result.original_exit_code;
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
		     << ", \"preprocessed_text\": \"" << JsonString(result.preprocessed_text) << "\"}";
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

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	OcrProbeOptions opt = ParseOptions(CommandLine());
	if(opt.help || opt.tracker_dir.IsEmpty()) {
		PrintHelp();
		if(opt.tracker_dir.IsEmpty() && !opt.help)
			SetExitCode(1);
		return;
	}
	if(!RunProbe(opt))
		SetExitCode(1);
}
