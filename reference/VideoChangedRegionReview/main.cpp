#include "VideoChangedRegionReview.h"

NAMESPACE_UPP

static String JsonString(const String& value)
{
	String out;
	for(int i = 0; i < value.GetCount(); i++) {
		byte c = value[i];
		if(c == '\\') out << "\\\\";
		else if(c == '"') out << "\\\"";
		else if(c == '\n') out << "\\n";
		else out.Cat(c);
	}
	return out;
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

static String Text(ValueMap map, const char *key)
{
	Value value = map.Get(key, Value());
	return IsVoid(value) || IsNull(value) ? String() : AsString(value);
}

static String Resolve(const String& path)
{
	if(path.IsEmpty()) return String();
	if(FileExists(path)) return NormalizePath(path);
	String candidate = AppendFileName(GetCurrentDirectory(), path);
	return FileExists(candidate) ? NormalizePath(candidate) : path;
}

static String RelativeTo(const String& file, const String& target)
{
	String base = NormalizePath(GetFileDirectory(file));
	String normalized = NormalizePath(target);
	if(normalized.StartsWith(base))
		return normalized.Mid(base.GetCount());
	normalized.Replace("\\", "/");
	if(normalized.GetCount() > 1 && normalized[1] == ':')
		return "file:///" + normalized;
	return normalized;
}

static String Signature(const Image& image)
{
	String result = "rgb8x8:";
	for(int y = 0; y < 8; y++)
		for(int x = 0; x < 8; x++) {
			int sx = min(image.GetWidth() - 1, x * image.GetWidth() / 8);
			int sy = min(image.GetHeight() - 1, y * image.GetHeight() / 8);
			Color c = image[sy][sx];
			result << Format("%02x%02x%02x", c.GetR() >> 4, c.GetG() >> 4, c.GetB() >> 4);
		}
	return result;
}

static void Help()
{
	Cout() << "Usage: VideoChangedRegionReview --manifest <file> --out-dir <dir>\n";
}

static ReviewOptions Parse(const Vector<String>& args)
{
	ReviewOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--manifest" && i + 1 < args.GetCount()) opt.manifest = args[++i];
		else if(args[i] == "--out-dir" && i + 1 < args.GetCount()) opt.out_dir = args[++i];
		else if(args[i] == "--help" || args[i] == "-h") opt.help = true;
	}
	return opt;
}

struct ReviewGroup : Moveable<ReviewGroup> {
	String signature, sample, fingerprint;
	int count = 0;
	String review_status, label, rank, suit;
};

static void LoadPreviousLabels(const String& path, Vector<ReviewGroup>& groups)
{
	Value root_value = ParseJSON(LoadFile(path));
	if(IsError(root_value) || !IsValueMap(root_value))
		return;
	ValueMap root = root_value;
	ValueArray previous = root.Get("groups", ValueArray());
	for(Value item_value : previous) {
		ValueMap item = item_value;
		String fingerprint = Text(item, "exact_fingerprint");
		for(ReviewGroup& group : groups)
			if(group.fingerprint == fingerprint) {
				group.review_status = Text(item, "review_status");
				group.label = Text(item, "label");
				group.rank = Text(item, "rank");
				group.suit = Text(item, "suit");
			}
	}
}

static void AppendNullable(String& json, const char *key, const String& value)
{
	json << "\"" << key << "\": ";
	if(value.IsEmpty())
		json << "null";
	else
		json << "\"" << JsonString(value) << "\"";
}

static bool Run(const ReviewOptions& opt)
{
	Value root_value = ParseJSON(LoadFile(opt.manifest));
	if(IsError(root_value) || !IsValueMap(root_value)) {
		Cerr() << "ERROR: invalid manifest " << opt.manifest << "\n";
		return false;
	}
	ValueMap root = root_value;
	ValueArray input = root.Get("groups", ValueArray());
	Vector<ReviewGroup> groups;
	for(int i = 0; i < input.GetCount(); i++) {
		ValueMap item = input[i];
		String sample = Resolve(Text(item, "sample_path"));
		Image image = StreamRaster::LoadFileAny(sample);
		if(image.IsEmpty()) { Cerr() << "WARNING: cannot load " << sample << "\n"; continue; }
		String signature = Signature(image);
		int found = -1;
		for(int j = 0; j < groups.GetCount(); j++) if(groups[j].signature == signature) found = j;
		if(found < 0) { ReviewGroup& group = groups.Add(); group.signature = signature; group.sample = sample; group.fingerprint = Text(item, "fingerprint"); found = groups.GetCount() - 1; }
		groups[found].count++;
	}
	RealizeDirectory(opt.out_dir);
	String json_path = AppendFileName(opt.out_dir, "changed_region_review.json");
	LoadPreviousLabels(json_path, groups);
	String json = "{\n  \"groups\": [\n";
	for(int i = 0; i < groups.GetCount(); i++) {
		if(i) json << ",\n";
		const ReviewGroup& g = groups[i];
		json << "    {\"id\": " << i << ", \"exact_fingerprint\": \"" << JsonString(g.fingerprint)
		     << "\", \"normalized_group_id\": \"group-" << i << "\", \"normalized_signature\": \""
		     << g.signature << "\", \"sample_path\": \"" << JsonString(g.sample)
		     << "\", \"occurrence_count\": " << g.count << ", \"review_status\": \""
		     << JsonString(g.review_status.IsEmpty() ? String("unknown") : g.review_status) << "\", ";
		AppendNullable(json, "label", g.label);
		json << ", ";
		AppendNullable(json, "rank", g.rank);
		json << ", ";
		AppendNullable(json, "suit", g.suit);
		json << "}";
	}
	json << "\n  ]\n}\n";
	String html = "<!doctype html><meta charset=\"utf-8\"><title>Changed region review</title><style>body{font:14px sans-serif;background:#222;color:#eee}article{display:inline-block;vertical-align:top;margin:12px;padding:12px;background:#333}img{display:block;max-width:320px;max-height:240px}</style><h1>Changed region review</h1>";
	for(int i = 0; i < groups.GetCount(); i++) {
		const ReviewGroup& g = groups[i];
		html << "<article><h2>Group " << i << "</h2><img src=\"" << HtmlEscape(RelativeTo(AppendFileName(opt.out_dir, "changed_region_review.html"), g.sample)) << "\"><p>status: " << HtmlEscape(g.review_status.IsEmpty() ? String("unknown") : g.review_status) << "<br>label: " << HtmlEscape(g.label.IsEmpty() ? String("—") : g.label) << "<br>rank: " << HtmlEscape(g.rank.IsEmpty() ? String("—") : g.rank) << "<br>suit: " << HtmlEscape(g.suit.IsEmpty() ? String("—") : g.suit) << "<br>occurrences: " << g.count << "</p><code>" << HtmlEscape(g.signature) << "</code><br><code>" << HtmlEscape(g.fingerprint) << "</code></article>";
	}
	if(!SaveFile(json_path, json) || !SaveFile(AppendFileName(opt.out_dir, "changed_region_review.html"), html)) return false;
	Cout() << "samples=" << input.GetCount() << " groups=" << groups.GetCount() << "\nreview_json=" << json_path << "\nreview_html=" << AppendFileName(opt.out_dir, "changed_region_review.html") << "\n";
	return true;
}

END_UPP_NAMESPACE
using namespace Upp;
CONSOLE_APP_MAIN
{
	ReviewOptions opt = Parse(CommandLine());
	if(opt.help || opt.manifest.IsEmpty() || opt.out_dir.IsEmpty()) { Help(); if(!opt.help) SetExitCode(1); return; }
	if(!Run(opt)) SetExitCode(1);
}
