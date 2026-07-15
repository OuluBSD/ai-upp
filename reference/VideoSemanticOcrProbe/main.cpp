#include "VideoSemanticOcrProbe.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoSemanticOcrProbe\n\n"
	       << "Usage: VideoSemanticOcrProbe --tracker-dir <dir> [options]\n\n"
	       << "Options:\n"
	       << "  --tracker-dir <dir>   VideoWindowTracker output directory\n"
	       << "  --out <file>          JSON output path (default <dir>/ocr_probe.json)\n"
	       << "  --tesseract <exe>     Tesseract executable path\n"
	       << "  --tessdata-dir <dir>  Tesseract tessdata directory\n"
	       << "  --lang <name>         OCR language (default eng)\n"
	       << "  --psm <mode>          Tesseract page segmentation mode (default 6)\n"
	       << "  --max-crops <count>   Maximum crop files to OCR (default 40)\n"
	       << "  --help, -h            Show help\n";
}

static OcrProbeOptions ParseOptions(const Vector<String>& args)
{
	OcrProbeOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--tracker-dir" && i + 1 < args.GetCount())
			opt.tracker_dir = args[++i];
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_path = args[++i];
		else if(args[i] == "--tesseract" && i + 1 < args.GetCount())
			opt.tesseract = args[++i];
		else if(args[i] == "--tessdata-dir" && i + 1 < args.GetCount())
			opt.tessdata_dir = args[++i];
		else if(args[i] == "--lang" && i + 1 < args.GetCount())
			opt.lang = args[++i];
		else if(args[i] == "--psm" && i + 1 < args.GetCount())
			opt.psm = max(3, StrInt(args[++i]));
		else if(args[i] == "--max-crops" && i + 1 < args.GetCount())
			opt.max_crops = max(1, StrInt(args[++i]));
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
	       name == "left_seats" || name == "right_seats" || name == "bottom_seat";
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

static bool VerifyTesseractLanguage(const OcrProbeOptions& opt, String& error)
{
	String trained = AppendFileName(opt.tessdata_dir, opt.lang + ".traineddata");
	if(!FileExists(trained)) {
		error = "missing tesseract language data: " + trained;
		Cerr() << "ERROR: " << error << "\n";
		Cerr() << "Install the language data or pass --tessdata-dir and --lang.\n";
		return false;
	}
	return true;
}

static bool RunProbe(const OcrProbeOptions& opt)
{
	if(!FileExists(opt.tesseract)) {
		Cerr() << "ERROR: missing tesseract executable: " << opt.tesseract << "\n";
		SaveErrorJson(opt, "missing tesseract executable: " + opt.tesseract);
		return false;
	}
	String language_error;
	if(!VerifyTesseractLanguage(opt, language_error)) {
		SaveErrorJson(opt, language_error);
		return false;
	}
	Vector<OcrCrop> crops = FindCrops(opt);
	String json;
	json << "{\n";
	json << "  \"ok\": true,\n";
	json << "  \"tracker_dir\": \"" << JsonString(opt.tracker_dir) << "\",\n";
	json << "  \"tesseract\": \"" << JsonString(opt.tesseract) << "\",\n";
	json << "  \"tessdata_dir\": \"" << JsonString(opt.tessdata_dir) << "\",\n";
	json << "  \"lang\": \"" << JsonString(opt.lang) << "\",\n";
	json << "  \"psm\": " << opt.psm << ",\n";
	json << "  \"crop_count\": " << crops.GetCount() << ",\n";
	json << "  \"results\": [\n";
	for(int i = 0; i < crops.GetCount(); i++) {
		int exit_code = -1;
		String text = RunTesseract(opt, crops[i].path, exit_code);
		Cout() << "ocr frame=" << crops[i].frame_index
		       << " table=" << crops[i].table_id
		       << " semantic=" << crops[i].semantic
		       << " exit=" << exit_code
		       << " text=\"" << text << "\"\n";
		if(i)
			json << ",\n";
		json << "    {\"frame_index\": " << crops[i].frame_index
		     << ", \"table_id\": " << crops[i].table_id
		     << ", \"semantic\": \"" << JsonString(crops[i].semantic)
		     << "\", \"path\": \"" << JsonString(crops[i].path)
		     << "\", \"exit_code\": " << exit_code
		     << ", \"text\": \"" << JsonString(text) << "\"}";
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
