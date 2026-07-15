#include "VideoChangedRegionCollector.h"

NAMESPACE_UPP

struct RegionOccurrence : Moveable<RegionOccurrence> {
	String tracker_dir;
	int frame_index = 0;
	int table_id = 0;
	int region_index = 0;
	Rect rect;
	int block_count = 0;
	int changed_pixels = 0;
	String crop_path;
	String frame_path;
	String table_crop_path;
	String change_overlay_path;
};

struct UniqueRegion : Moveable<UniqueRegion> {
	String fingerprint;
	Image image;
	String sample_path;
	Vector<RegionOccurrence> occurrences;
};

static void PrintHelp()
{
	Cout() << "VideoChangedRegionCollector\n\n"
	       << "Usage: VideoChangedRegionCollector --tracker-dir <dir> [options]\n\n"
	       << "Options:\n"
	       << "  --tracker-dir <dir>  Tracker output directory; may be repeated\n"
	       << "  --out-dir <dir>      Output directory\n"
	       << "  --help, -h           Show help\n";
}

static ChangedRegionCollectorOptions ParseOptions(const Vector<String>& args)
{
	ChangedRegionCollectorOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--tracker-dir" && i + 1 < args.GetCount())
			opt.tracker_dirs << args[++i];
		else if(args[i] == "--out-dir" && i + 1 < args.GetCount())
			opt.out_dir = args[++i];
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(opt.out_dir.IsEmpty() && !opt.tracker_dirs.IsEmpty())
		opt.out_dir = AppendFileName(opt.tracker_dirs[0], "changed_region_manifest");
	return opt;
}

static String JsonString(const String& value)
{
	String out;
	for(int i = 0; i < value.GetCount(); i++) {
		byte c = value[i];
		if(c == '\\') out << "\\\\";
		else if(c == '"') out << "\\\"";
		else if(c == '\n') out << "\\n";
		else if(c == '\r') out << "\\r";
		else if(c == '\t') out << "\\t";
		else out.Cat(c);
	}
	return out;
}

static String TextValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, Value());
	return IsString(value) ? String(value) : AsString(value);
}

static int IntValue(ValueMap map, const char *key, int fallback = 0)
{
	Value value = map.Get(key, Value());
	if(IsNumber(value))
		return (int)value;
	int parsed = StrInt(AsString(value));
	return IsNull(parsed) ? fallback : parsed;
}

static ValueArray ArrayValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, ValueArray());
	return IsValueArray(value) ? ValueArray(value) : ValueArray();
}

static Rect RectValue(ValueMap map, const char *key)
{
	ValueMap value = map.Get(key, ValueMap());
	return Rect(IntValue(value, "x"), IntValue(value, "y"),
	            IntValue(value, "x") + IntValue(value, "w"),
	            IntValue(value, "y") + IntValue(value, "h"));
}

static String ResolvePath(const String& path)
{
	if(path.IsEmpty())
		return String();
	if(FileExists(path))
		return path;
	String absolute = AppendFileName(GetCurrentDirectory(), path);
	return FileExists(absolute) ? absolute : path;
}

static String Fingerprint(const Image& image)
{
	return Format("%d`x%d:%08x", image.GetWidth(), image.GetHeight(),
	              (int)image.GetHashValue());
}

static int FindUnique(const Vector<UniqueRegion>& regions, const String& fingerprint)
{
	for(int i = 0; i < regions.GetCount(); i++)
		if(regions[i].fingerprint == fingerprint)
			return i;
	return -1;
}

static void AppendRect(String& json, const Rect& rect)
{
	json << "{\"x\": " << rect.left << ", \"y\": " << rect.top
	     << ", \"w\": " << rect.Width() << ", \"h\": " << rect.Height() << "}";
}

static void AppendOccurrence(String& json, const RegionOccurrence& occurrence)
{
	json << "{\"tracker_dir\": \"" << JsonString(occurrence.tracker_dir)
	     << "\", \"frame_index\": " << occurrence.frame_index
	     << ", \"table_id\": " << occurrence.table_id
	     << ", \"region_index\": " << occurrence.region_index
	     << ", \"rect\": ";
	AppendRect(json, occurrence.rect);
	json << ", \"block_count\": " << occurrence.block_count
	     << ", \"changed_pixels\": " << occurrence.changed_pixels
	     << ", \"crop_path\": \"" << JsonString(occurrence.crop_path)
	     << "\", \"frame_path\": \"" << JsonString(occurrence.frame_path)
	     << "\", \"table_crop_path\": \"" << JsonString(occurrence.table_crop_path)
	     << "\", \"change_overlay_path\": \""
	     << JsonString(occurrence.change_overlay_path) << "\"}";
}

static void AppendManifest(String& json, const Vector<UniqueRegion>& regions,
                           const Vector<String>& tracker_dirs)
{
	json << "{\n  \"tracker_dirs\": [";
	for(int i = 0; i < tracker_dirs.GetCount(); i++) {
		if(i) json << ", ";
		json << "\"" << JsonString(tracker_dirs[i]) << "\"";
	}
	json << "],\n  \"unique_count\": " << regions.GetCount() << ",\n  \"groups\": [\n";
	for(int i = 0; i < regions.GetCount(); i++) {
		if(i) json << ",\n";
		const UniqueRegion& group = regions[i];
		json << "    {\"id\": " << i << ", \"fingerprint\": \""
		     << JsonString(group.fingerprint) << "\", \"sample_path\": \""
		     << JsonString(group.sample_path) << "\", \"occurrence_count\": "
		     << group.occurrences.GetCount() << ", \"occurrences\": [";
		for(int j = 0; j < group.occurrences.GetCount(); j++) {
			if(j) json << ", ";
			AppendOccurrence(json, group.occurrences[j]);
		}
		json << "]}";
	}
	json << "\n  ]\n}\n";
}

static String HtmlEscape(const String& value)
{
	String out;
	for(int i = 0; i < value.GetCount(); i++) {
		byte c = value[i];
		if(c == '&') out << "&amp;";
		else if(c == '<') out << "&lt;";
		else if(c == '>') out << "&gt;";
		else if(c == '"') out << "&quot;";
		else out.Cat(c);
	}
	return out;
}

static String GalleryPath(const String& html_path, const String& target)
{
	String from = NormalizePath(GetFileDirectory(html_path));
	String normalized = NormalizePath(target);
	if(normalized.StartsWith(from))
		return normalized.Mid(from.GetCount());
	return normalized;
}

static bool SaveGallery(const String& path, const Vector<UniqueRegion>& regions)
{
	String html;
	html << "<!doctype html><meta charset=\"utf-8\"><title>Changed regions</title>"
	      << "<style>body{font:14px sans-serif;background:#222;color:#eee}"
	      << "article{display:inline-block;vertical-align:top;margin:12px;padding:10px;"
	      << "background:#333;max-width:480px}img{display:block;max-width:460px;max-height:260px;"
	      << "image-rendering:auto}code{white-space:pre-wrap}</style>"
	      << "<h1>Unique changed regions</h1><p>Groups are exact image fingerprints;"
	      << " every occurrence is retained for manual classification.</p>";
	for(int i = 0; i < regions.GetCount(); i++) {
		const UniqueRegion& group = regions[i];
		html << "<article><h2>Group " << i << " (" << group.occurrences.GetCount()
		     << " occurrences)</h2><img src=\"" << HtmlEscape(GalleryPath(path, group.sample_path))
		     << "\"><code>" << HtmlEscape(group.fingerprint) << "\n";
		for(const RegionOccurrence& occurrence : group.occurrences)
			html << "frame=" << occurrence.frame_index << " table=" << occurrence.table_id
		     << " rect=" << occurrence.rect.left << "," << occurrence.rect.top << " "
		     << occurrence.rect.Width() << "`x" << occurrence.rect.Height()
		     << " pixels=" << occurrence.changed_pixels << "\n";
		html << "</code></article>";
	}
	return SaveFile(path, html);
}

static bool Collect(const ChangedRegionCollectorOptions& opt)
{
	Vector<UniqueRegion> regions;
	int input_regions = 0;
	int invalid_regions = 0;
	for(const String& tracker_dir : opt.tracker_dirs) {
		String summary_path = AppendFileName(tracker_dir, "tracking_summary.json");
		Value root_value = ParseJSON(LoadFile(summary_path));
		if(IsError(root_value) || !IsValueMap(root_value)) {
			Cerr() << "ERROR: invalid tracking summary " << summary_path << "\n";
			return false;
		}
		ValueMap root = root_value;
		for(Value frame_value : ArrayValue(root, "frames")) {
			ValueMap frame = frame_value;
			int frame_index = IntValue(frame, "index");
			String frame_path = ResolvePath(TextValue(frame, "path"));
			for(Value table_value : ArrayValue(frame, "tables")) {
				ValueMap table = table_value;
				int table_id = IntValue(table, "id");
				String table_crop_path = ResolvePath(TextValue(table, "crop"));
				String change_overlay_path = ResolvePath(TextValue(table, "change_overlay"));
				for(Value region_value : ArrayValue(table, "changed_regions")) {
					input_regions++;
					ValueMap source = region_value;
					String crop_path = ResolvePath(TextValue(source, "crop_path"));
					Image image = StreamRaster::LoadFileAny(crop_path);
					if(image.IsEmpty()) {
						invalid_regions++;
						Cerr() << "WARNING: cannot load changed region crop " << crop_path << "\n";
						continue;
					}
					String fingerprint = Fingerprint(image);
					int group_id = FindUnique(regions, fingerprint);
					if(group_id < 0) {
						UniqueRegion& group = regions.Add();
						group.fingerprint = fingerprint;
						group.image = image;
						group.sample_path = crop_path;
						group_id = regions.GetCount() - 1;
					}
					RegionOccurrence& occurrence = regions[group_id].occurrences.Add();
					occurrence.tracker_dir = tracker_dir;
					occurrence.frame_index = frame_index;
					occurrence.table_id = table_id;
					occurrence.region_index = IntValue(source, "index");
					occurrence.rect = RectValue(source, "rect");
					occurrence.block_count = IntValue(source, "block_count");
					occurrence.changed_pixels = IntValue(source, "changed_pixels");
					occurrence.crop_path = crop_path;
					occurrence.frame_path = frame_path;
					occurrence.table_crop_path = table_crop_path;
					occurrence.change_overlay_path = change_overlay_path;
				}
			}
		}
	}
	RealizeDirectory(opt.out_dir);
	String manifest_path = AppendFileName(opt.out_dir, "changed_region_manifest.json");
	String gallery_path = AppendFileName(opt.out_dir, "changed_region_gallery.html");
	String manifest;
	AppendManifest(manifest, regions, opt.tracker_dirs);
	if(!SaveFile(manifest_path, manifest) || !SaveGallery(gallery_path, regions))
		return false;
	Cout() << "input_regions=" << input_regions << " unique_groups=" << regions.GetCount()
	       << " invalid_regions=" << invalid_regions << "\n"
	       << "manifest_json=" << manifest_path << "\n"
	       << "gallery_html=" << gallery_path << "\n";
	return invalid_regions == 0;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	ChangedRegionCollectorOptions opt = ParseOptions(CommandLine());
	if(opt.help || opt.tracker_dirs.IsEmpty()) {
		PrintHelp();
		if(opt.tracker_dirs.IsEmpty() && !opt.help)
			SetExitCode(1);
		return;
	}
	if(!Collect(opt))
		SetExitCode(1);
}
