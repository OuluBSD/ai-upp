#include "VideoCardCandidateExtractor.h"

using namespace Upp;

static Value V(ValueMap m, const char *k) { return m.Get(k, Value()); }
static String S(ValueMap m, const char *k) { return AsString(V(m, k)); }
static int I(ValueMap m, const char *k) { Value v=V(m,k); return IsNumber(v) ? (int)v : StrInt(AsString(v)); }
static ValueArray A(ValueMap m, const char *k) { Value v=V(m,k); return IsValueArray(v) ? ValueArray(v) : ValueArray(); }
static bool Has(ValueMap m, const char *k) { return !IsNull(V(m,k)) && !AsString(V(m,k)).IsEmpty(); }

static void AddRecord(ValueArray& out, ValueMap record)
{
	String semantic;
	for(Value hv : A(record, "semantic_hits")) {
		ValueMap h=hv; String n=ToLower(S(h,"name"));
		if(!semantic.IsEmpty()) semantic << " "; semantic << n;
	}
	ValueMap rect=V(record,"rect");
	int w=I(rect,"w"), h=I(rect,"h");
	String type="unknown";
	ValueMap text_state=V(record,"contains_text");
	bool text = S(text_state,"hypothesis") == "likely" || semantic.Find("text") >= 0 || semantic.Find("label") >= 0 || semantic.Find("number") >= 0;
	bool board = semantic.Find("board") >= 0 || semantic.Find("community") >= 0 || semantic.Find("card") >= 0;
	bool seat = semantic.Find("seat") >= 0 || semantic.Find("player") >= 0 || semantic.Find("hole") >= 0;
	if(text) type="text_candidate";
	else if(board && w > 0 && h > 0) type="board_candidate";
	else if(seat && w > 0 && h > 0) type="seat_candidate";
	ValueMap c;
	if(Has(record,"id")) c("id",V(record,"id"));
	if(Has(record,"source_id")) c("source_id",V(record,"source_id"));
	c("candidate_type",type)("rect",rect)("semantic_hits",V(record,"semantic_hits"));
	for(const char *k : {"review_label", "review", "label"}) if(Has(record,k)) { c("review_label",V(record,k)); break; }
	for(const char *k : {"crop_path", "frame_path", "table_crop_path", "change_overlay_path", "diagnostic_crops", "preprocessing"}) if(Has(record,k)) c(k,V(record,k));
	for(const char *k : {"frame_index", "table_id", "region_index", "group_id", "event_id"}) if(Has(record,k)) c(k,V(record,k));
	c("source",record);
	out.Add(c);
}

static ValueArray Records(Value root)
{
	if(!IsValueMap(root)) return ValueArray(); ValueMap m=root;
	ValueArray out=A(m,"occurrences"); if(!out.IsEmpty()) return out;
	for(Value v : A(m,"events")) { ValueMap e=v; for(Value ov : A(e,"occurrences")) { ValueMap o=ov; o("event_id",V(e,"event_id")); out.Add(o); } }
	for(Value v : A(m,"groups")) { ValueMap g=v; for(Value o : A((ValueMap)g,"occurrences")) out.Add(o); }
	return out;
}

static String HtmlEscape(const String& value)
{
	String out;
	for(int i=0; i<value.GetCount(); i++) {
		if(value[i]=='&') out << "&amp;";
		else if(value[i]=='<') out << "&lt;";
		else if(value[i]=='>') out << "&gt;";
		else if(value[i]=='\"') out << "&quot;";
		else out.Cat(value[i]);
	}
	return out;
}

static bool Gallery(String path, ValueArray candidates)
{
	String html="<!doctype html><meta charset=utf-8><title>Video card candidates</title><style>body{font:14px sans-serif;background:#222;color:#eee}article{display:inline-block;vertical-align:top;margin:10px;padding:10px;background:#333;max-width:440px}img{max-width:420px;max-height:280px;display:block}pre{white-space:pre-wrap}</style><h1>Video card candidates</h1>";
	for(int i=0;i<candidates.GetCount();i++) { ValueMap c=candidates[i]; String img=S(c,"crop_path"); html << "<article><h2>"<<i<<" "<<S(c,"candidate_type")<<"</h2>"; if(!img.IsEmpty()) html << "<img src=\""<<HtmlEscape(img)<<"\">"; html << "<pre>"<<HtmlEscape(AsJSON(c,true))<<"</pre></article>"; }
	return SaveFile(path,html);
}

CONSOLE_APP_MAIN
{
	String events,enrichment,outdir; const Vector<String>& args=CommandLine();
	for(int i=0;i<args.GetCount();i++) { if(args[i]=="--events"&&i+1<args.GetCount()) events=args[++i]; else if(args[i]=="--enrichment"&&i+1<args.GetCount()) enrichment=args[++i]; else if(args[i]=="--out-dir"&&i+1<args.GetCount()) outdir=args[++i]; }
	if(events.IsEmpty()||enrichment.IsEmpty()||outdir.IsEmpty()) { Cout()<<"VideoCardCandidateExtractor --events <file> --enrichment <file> --out-dir <dir>\n"; SetExitCode(1); return; }
	Value er=ParseJSON(LoadFile(events)), xr=ParseJSON(LoadFile(enrichment)); if(IsError(er)||IsError(xr)) { Cerr()<<"ERROR: invalid input JSON\n"; SetExitCode(1); return; }
	ValueArray base=Records(er), extra=Records(xr), candidates; for(Value v:base) { ValueMap r=v; for(Value x:extra) { ValueMap e=x; if((Has(r,"source_id")&&S(r,"source_id")==S(e,"source_id")) || (I(r,"frame_index")==I(e,"frame_index")&&I(r,"table_id")==I(e,"table_id")&&I(r,"region_index")==I(e,"region_index"))) { for(int j=0;j<e.GetCount();j++) r(e.GetKey(j),e[j]); break; } } AddRecord(candidates,r); }
	RealizeDirectory(outdir); ValueMap result; result("events",events)("enrichment",enrichment)("candidate_count",candidates.GetCount())("candidates",candidates); String json=AppendFileName(outdir,"video_card_candidates.json"); SaveFile(json,AsJSON(result,true)); String gallery=AppendFileName(outdir,"video_card_candidates.html"); Gallery(gallery,candidates); Cout()<<"candidate_count="<<candidates.GetCount()<<"\njson="<<json<<"\ngallery="<<gallery<<"\n";
}
