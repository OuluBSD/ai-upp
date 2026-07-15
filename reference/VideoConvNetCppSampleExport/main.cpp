#include "VideoConvNetCppSampleExport.h"

NAMESPACE_UPP

static String J(const String& s)
{
	String r;
	for(int i = 0; i < s.GetCount(); i++) {
		byte c = s[i];
		if(c == '\\') r << "\\\\";
		else if(c == '"') r << "\\\"";
		else if(c == '\n') r << "\\n";
		else if(c == '\r') r << "\\r";
		else r.Cat(c);
	}
	return r;
}

static String Text(ValueMap m, const char *key)
{
	Value v = m.Get(key, Value());
	return IsVoid(v) || IsNull(v) ? String() : AsString(v);
}

static String Resolve(String p)
{
	if(p.IsEmpty()) return String();
	if(FileExists(p)) return NormalizePath(p);
	String q = AppendFileName(GetCurrentDirectory(), p);
	return FileExists(q) ? NormalizePath(q) : NormalizePath(p);
}

static String Field(ValueMap map, const char *key)
{
	String value = Text(map, key);
	if(!value.IsEmpty())
		return value;
	Value source = map.Get("source", ValueMap());
	return IsValueMap(source) ? Text(ValueMap(source), key) : String();
}

static void AddPath(Vector<String>& paths, const String& p)
{
	if(p.IsEmpty()) return;
	for(const String& path : paths) if(path == p) return;
	paths.Add(p);
}

static void LoadReview(const String& path, Index<String>& label_keys, Vector<String>& label_values)
{
	if(path.IsEmpty()) return;
	Value root = ParseJSON(LoadFile(path));
	if(IsError(root) || !IsValueMap(root)) return;
	for(Value v : ValueMap(root).Get("groups", ValueArray())) {
		ValueMap g = v;
		String key = Text(g, "exact_fingerprint");
		if(key.IsEmpty()) key = Text(g, "fingerprint");
		if(!key.IsEmpty()) { label_keys.Add(key); label_values.Add(Text(g, "label")); }
	}
}

static ExportOptions Parse(const Vector<String>& args)
{
	ExportOptions o;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--candidates" && i + 1 < args.GetCount()) o.candidates = args[++i];
		else if(args[i] == "--review" && i + 1 < args.GetCount()) o.review = args[++i];
		else if(args[i] == "--out-dir" && i + 1 < args.GetCount()) o.out_dir = args[++i];
		else if(args[i] == "--help" || args[i] == "-h") o.help = true;
	}
	return o;
}

static bool Run(const ExportOptions& o)
{
	Value root = ParseJSON(LoadFile(o.candidates));
	if(IsError(root) || !IsValueMap(root)) { Cerr() << "ERROR: invalid candidates file\n"; return false; }
	ValueArray groups = ValueMap(root).Get("candidates", ValueArray());
	if(groups.IsEmpty())
		groups = ValueMap(root).Get("groups", ValueArray());
	Index<String> label_keys;
	Vector<String> label_values;
	LoadReview(o.review, label_keys, label_values);
	String out;
	out << "{\n  \"schema\": \"video-convnetcpp-dataset-v1\",\n"
	     << "  \"model\": {\"status\": \"unavailable\", \"runtime_required\": false},\n"
	     << "  \"samples\": [\n";
	for(int i = 0; i < groups.GetCount(); i++) {
		ValueMap g = groups[i];
		String fingerprint = Field(g, "exact_fingerprint");
		if(fingerprint.IsEmpty()) fingerprint = Text(g, "fingerprint");
		String label = Text(g, "label");
		if(label.IsEmpty()) {
			int label_index = label_keys.Find(fingerprint);
			if(label_index >= 0) label = label_values[label_index];
		}
		String status = Text(g, "review_status");
		if(status.IsEmpty()) status = "unknown";
		Vector<String> variants;
		String sample_path = Resolve(Field(g, "sample_path"));
		if(sample_path.IsEmpty())
			sample_path = Resolve(Field(g, "crop_path"));
		AddPath(variants, sample_path);
		Value diagnostic_value = g.Get("diagnostic_crops", ValueMap());
		if(IsValueMap(diagnostic_value)) {
			ValueMap diagnostics = diagnostic_value;
			for(const char *key : {"original", "grayscale", "inverse"})
				AddPath(variants, Resolve(Text(diagnostics, key)));
		}
		for(Value v : ValueMap(g).Get("occurrences", ValueArray())) {
			ValueMap occurrence = v;
			AddPath(variants, Resolve(Text(occurrence, "crop_path")));
		}
		if(fingerprint.IsEmpty() && !sample_path.IsEmpty()) {
			Image image = StreamRaster::LoadFileAny(sample_path);
			if(!image.IsEmpty())
				fingerprint = Format("image:%d`x%d:%08x", image.GetWidth(), image.GetHeight(), (int)image.GetHashValue());
		}
		if(i) out << ",\n";
		out << "    {\"id\": " << i << ", \"input_signature\": \"" << J(fingerprint)
		    << "\", \"image_variants\": [";
		for(int j = 0; j < variants.GetCount(); j++) { if(j) out << ", "; out << "\"" << J(variants[j]) << "\""; }
		out << "], \"label\": " << (label.IsEmpty() ? "null" : "\"" + J(label) + "\"")
		    << ", \"needs_review\": " << (label.IsEmpty() || status != "approved" ? "true" : "false")
		    << ", \"review_status\": \"" << J(status) << "\"}";
	}
	out << "\n  ]\n}\n";
	RealizeDirectory(o.out_dir);
	String manifest = AppendFileName(o.out_dir, "dataset_manifest.json");
	String contract = "{\n  \"format\": \"ConvNetCpp\",\n  \"dataset\": \"dataset_manifest.json\",\n  \"input\": {\"image_variants\": true, \"label_nullable\": true},\n  \"model_status\": \"unknown\"\n}\n";
	bool ok = SaveFile(manifest, out) && SaveFile(AppendFileName(o.out_dir, "convnetcpp_contract.json"), contract);
	Cout() << "candidate_groups=" << groups.GetCount() << " exported_samples=" << groups.GetCount()
	       << " model_status=unavailable\n"
	       << "dataset_manifest=" << manifest << "\n";
	return ok;
}

END_UPP_NAMESPACE
using namespace Upp;
CONSOLE_APP_MAIN
{
	ExportOptions o = Parse(CommandLine());
	if(o.help || o.candidates.IsEmpty() || o.out_dir.IsEmpty()) {
		Cout() << "Usage: VideoConvNetCppSampleExport --candidates <file> --out-dir <dir> [--review <file>]\n";
		if(!o.help) SetExitCode(1);
		return;
	}
	if(!Run(o)) SetExitCode(1);
}
