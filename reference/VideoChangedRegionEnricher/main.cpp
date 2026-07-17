#include "VideoChangedRegionEnricher.h"

NAMESPACE_UPP

static Value V(ValueMap m, const char *key) { return m.Get(key, Value()); }
static int Int(ValueMap m, const char *key) { return IsNumber(V(m, key)) ? (int)V(m, key) : StrInt(AsString(V(m, key))); }
static String Str(ValueMap m, const char *key) { return AsString(V(m, key)); }
static Rect R(ValueMap m) { return Rect(Int(m,"x"), Int(m,"y"), Int(m,"x")+Int(m,"w"), Int(m,"y")+Int(m,"h")); }
static ValueArray A(ValueMap m, const char *key) { return IsValueArray(V(m, key)) ? ValueArray(V(m, key)) : ValueArray(); }
static void Help() {
	Cout() << "VideoChangedRegionEnricher --manifest <file> --out-dir <dir> "
	          "[--ocr-probe-json <path>]\n"
	          "  --ocr-probe-json <path>  Read OCR results from this exact "
	          "VideoSemanticOcrProbe JSON\n"
	          "                           output instead of guessing "
	          "<tracker_dir>/ocr_probe.json\n"
	          "                           (Task 0277). Applies to every "
	          "occurrence in the manifest,\n"
	          "                           regardless of that occurrence's own "
	          "tracker_dir. Omit to keep\n"
	          "                           the previous per-occurrence-tracker_dir "
	          "guess.\n";
}

static double Overlap(const Rect& a, const Rect& b)
{
	Rect i = a & b;
	return a.Width() > 0 && a.Height() > 0 ? (double)(i.Width() * i.Height()) / (a.Width() * a.Height()) : 0;
}

static ValueMap FindTable(ValueMap summary, int frame_index, int table_id)
{
	for(Value fv : A(summary, "frames")) {
		ValueMap frame = fv;
		if(Int(frame, "index") != frame_index && Int(frame, "frame_index") != frame_index) continue;
		for(Value tv : A(frame, "tables")) {
			ValueMap table = tv;
			if(Int(table, "id") == table_id || Int(table, "table_id") == table_id) return table;
		}
	}
	return ValueMap();
}

static ValueArray Hits(ValueMap table, const Rect& changed)
{
	ValueArray out;
	ValueMap semantic = V(table, "semantic");
	for(int i = 0; i < semantic.GetCount(); i++) {
		String name = semantic.GetKey(i);
		ValueMap item = semantic.Get(name, Value());
		Rect rect = R(V(item, "rect"));
		double overlap = Overlap(changed, rect);
		if(overlap <= 0) continue;
		ValueMap hit;
		hit("name", name)("rect", V(item, "rect"))("overlap", overlap);
		out.Add(hit);
	}
	return out;
}

// Task 0277: `ocr_probe_json_override`, when non-empty, is read verbatim
// instead of guessing <tracker_dir>/ocr_probe.json -- real
// VideoSemanticOcrProbe runs write wherever --out says, essentially never
// the guessed default, so every real enrichment run used to silently report
// every occurrence "unavailable" unless someone manually copied a probe
// output into the exact guessed path first (see AGENTS.md).
static ValueMap OcrEvidence(const String& tracker_dir, int frame_index, int table_id,
	                          ValueArray hits, const String& ocr_probe_json_override = String())
{
	String probe_path = ocr_probe_json_override.IsEmpty()
	                         ? AppendFileName(tracker_dir, "ocr_probe.json")
	                         : ocr_probe_json_override;
	Value root = ParseJSON(LoadFile(probe_path));
	if(IsError(root) || !IsValueMap(root)) {
		ValueMap evidence;
		evidence("status", "unavailable")("reason", "ocr_probe_json_missing");
		return evidence;
	}
	ValueArray results = A((ValueMap)root, "results");
	ValueArray matched;
	for(Value hit_value : hits) {
		ValueMap hit = hit_value;
		String semantic = Str(hit, "name");
		for(Value result_value : results) {
			ValueMap result = result_value;
			if(Int(result, "frame_index") != frame_index ||
			   Int(result, "table_id") != table_id ||
			   Str(result, "semantic") != semantic)
				continue;
			matched.Add(result);
		}
	}
	if(matched.IsEmpty()) {
		ValueMap evidence;
		evidence("status", "unavailable");
		evidence("reason", "no_matching_semantic_ocr_result");
		return evidence;
	}
	ValueMap evidence;
	evidence("status", "available")("results", matched);
	return evidence;
}

static void Diagnostics(ValueMap& occurrence, const String& out_dir, int index)
{
	String source = Str(occurrence, "crop_path");
	Image image = StreamRaster::LoadFileAny(source);
	ValueMap meta;
	meta("source", source)("available", !image.IsEmpty())("method", "crop_path")
	    ("scale", 1)("grayscale", "luma")("inverse", "255-minus-luma");
	if(image.IsEmpty()) { occurrence("diagnostic_crops", ValueMap())("preprocessing", meta); return; }
	String dir = AppendFileName(out_dir, "diagnostic_crops"); RealizeDirectory(dir);
	String base = Format("occurrence_%06d", index);
	String original = AppendFileName(dir, base + "_original.jpg");
	String gray = AppendFileName(dir, base + "_grayscale.jpg");
	String inverse = AppendFileName(dir, base + "_inverse.jpg");
	JPGEncoder().Quality(95).SaveFile(original, image);
	ImageBuffer g(image.GetSize()), inv(image.GetSize());
	for(int y = 0; y < image.GetHeight(); y++) for(int x = 0; x < image.GetWidth(); x++) {
		RGBA p = image[y][x]; byte v = (byte)Grayscale(p); g[y][x].r = g[y][x].g = g[y][x].b = v; g[y][x].a = 255; inv[y][x].r = inv[y][x].g = inv[y][x].b = 255-v; inv[y][x].a = 255;
	}
	JPGEncoder().Quality(95).SaveFile(gray, g); JPGEncoder().Quality(95).SaveFile(inverse, inv);
	ValueMap paths; paths("original", original)("grayscale", gray)("inverse", inverse);
	occurrence("diagnostic_crops", paths)("preprocessing", meta);
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	EnricherOptions opt; const Vector<String>& args = CommandLine();
	for(int i=0;i<args.GetCount();i++) { if(args[i]=="--manifest"&&i+1<args.GetCount()) opt.manifest=args[++i]; else if(args[i]=="--out-dir"&&i+1<args.GetCount()) opt.out_dir=args[++i]; else if(args[i]=="--ocr-probe-json"&&i+1<args.GetCount()) opt.ocr_probe_json=args[++i]; else if(args[i]=="--help"||args[i]=="-h") opt.help=true; }
	if(opt.help || opt.manifest.IsEmpty() || opt.out_dir.IsEmpty()) { Help(); if(!opt.help) SetExitCode(1); return; }
	Value root = ParseJSON(LoadFile(opt.manifest)); if(IsError(root) || !IsValueMap(root)) { Cerr()<<"ERROR: invalid manifest\n"; SetExitCode(1); return; }
	ValueArray enriched; int occurrences=0, matched=0, missing=0;
	for(Value gv : A((ValueMap)root,"groups")) for(Value ov : A((ValueMap)gv,"occurrences")) {
		ValueMap o=ov; occurrences++; String dir=Str(o,"tracker_dir"); Value summary=ParseJSON(LoadFile(AppendFileName(dir,"tracking_summary.json")));
		ValueMap table; if(!IsError(summary)&&IsValueMap(summary)) table=FindTable(summary,Int(o,"frame_index"),Int(o,"table_id"));
		if(table.IsEmpty()) missing++; else matched++;
		ValueArray hits = Hits(table,R(V(o, "rect")));
		o("semantic_hits", hits);
		ValueMap ocr = OcrEvidence(dir, Int(o, "frame_index"), Int(o, "table_id"), hits, opt.ocr_probe_json);
		String text_hypothesis = Str(ocr, "status") == "available" ? "likely" : "unknown";
		ValueMap text; text("hypothesis", text_hypothesis)
			("setting", "semantic-overlap-plus-ocr")
			("basis", Str(ocr, "status") == "available" ? "ocr_probe" : "semantic-overlap-only");
		o("contains_text", text);
		ValueMap ai; ai("status","unknown"); o("ai_classification",ai);
		o("ocr", ocr);
		Diagnostics(o,opt.out_dir,occurrences-1); enriched.Add(o);
	}
	RealizeDirectory(opt.out_dir); ValueMap output; output("manifest",opt.manifest)("occurrence_count",occurrences)("matched_count",matched)("missing_tracker_count",missing)("occurrences",enriched);
	String path=AppendFileName(opt.out_dir,"changed_region_enrichment.json"); if(!SaveFile(path,AsJSON(output,true))) { SetExitCode(1); return; }
	Cout()<<"occurrences="<<occurrences<<" matched="<<matched<<" missing_trackers="<<missing<<"\n"<<"enrichment_json="<<path<<"\n";
}
