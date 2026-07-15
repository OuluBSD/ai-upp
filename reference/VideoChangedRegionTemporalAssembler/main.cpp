#include "VideoChangedRegionTemporalAssembler.h"

NAMESPACE_UPP

static Value V(ValueMap m, const char *k) { return m.Get(k, Value()); }
static String S(ValueMap m, const char *k) { return AsString(V(m, k)); }
static int I(ValueMap m, const char *k) { Value v=V(m,k); if(IsNumber(v)) return (int)v; int n=StrInt(AsString(v)); return IsNull(n) ? 0 : n; }
static Rect RectOf(ValueMap m)
{
	ValueMap r = V(m,"rect");
	if(r.IsEmpty()) r=m;
	int x=I(r,"x"), y=I(r,"y"), w=I(r,"w"), h=I(r,"h");
	if(!w) w=I(r,"width"); if(!h) h=I(r,"height");
	return Rect(x,y,x+w,y+h);
}
static ValueArray Arr(ValueMap m, const char *k) { Value v=V(m,k); return IsValueArray(v) ? ValueArray(v) : ValueArray(); }
static int Frame(ValueMap m) { return IsNumber(V(m,"frame_index")) ? (int)V(m,"frame_index") : I(m,"frame"); }
static int Table(ValueMap m) { return IsNumber(V(m,"table_id")) ? (int)V(m,"table_id") : I(m,"table"); }
static bool Close(Rect a, Rect b)
{
	if(!(a & b).IsEmpty()) return true;
	const int d=16;
	return a.left <= b.right+d && b.left <= a.right+d && a.top <= b.bottom+d && b.top <= a.bottom+d;
}
static String Html(String s)
{
	s.Replace("&","&amp;"); s.Replace("<","&lt;"); s.Replace(">","&gt;"); s.Replace("\"","&quot;"); return s;
}
static void Help() { Cout()<<"VideoChangedRegionTemporalAssembler --enrichment <file> [--frame-gap <n>] --out-dir <dir>\n"; }

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	String input, out; int gap=1; const Vector<String>& a=CommandLine();
	for(int i=0;i<a.GetCount();i++) { if(a[i]=="--enrichment"&&i+1<a.GetCount()) input=a[++i]; else if(a[i]=="--frame-gap"&&i+1<a.GetCount()) gap=max(0,StrInt(a[++i])); else if(a[i]=="--out-dir"&&i+1<a.GetCount()) out=a[++i]; else if(a[i]=="--help"||a[i]=="-h") { Help(); return; } }
	if(input.IsEmpty()||out.IsEmpty()) { Help(); SetExitCode(1); return; }
	Value root=ParseJSON(LoadFile(input)); if(IsError(root)||!IsValueMap(root)) { Cerr()<<"ERROR: invalid enrichment JSON\n"; SetExitCode(1); return; }
	ValueArray src=Arr((ValueMap)root,"occurrences");
	Vector<ValueMap> occ; for(Value v:src) occ.Add((ValueMap)v);
	Index<int> used; ValueArray events;
	for(int i=0;i<occ.GetCount();i++) if(used.Find(i) < 0) {
		Vector<int> members; members<<i; used.Add(i); int last=Frame(occ[i]); Rect lastrect=RectOf(occ[i]);
		for(int j=i+1;j<occ.GetCount();j++) if(used.Find(j)<0 && Table(occ[j])==Table(occ[i]) && Frame(occ[j])-last<=gap && Close(lastrect,RectOf(occ[j]))) { members<<j; used.Add(j); last=Frame(occ[j]); lastrect=RectOf(occ[j]); }
		ValueArray ids, evidence, member_records; int first=Frame(occ[members[0]]), end=first;
		for(int k:members) { ValueMap o=occ[k]; String id=S(o,"occurrence_id"); if(id.IsEmpty()) id=S(o,"id"); ids.Add(id.IsEmpty()?Format("occurrence_%d",k):id); member_records.Add(o); evidence.Add(o); end=max(end,Frame(o)); }
		ValueMap e; e("event_id",Format("table_%d_%d",Table(occ[i]),first))("table_id",Table(occ[i]))("start_frame",first)("end_frame",end)("event_type",members.GetCount()==1?"appeared":"changed")("occurrence_ids",ids)("occurrences",member_records)("evidence",evidence); events.Add(e);
	}
	// A track's final state is explicit, making disappeared events consumable by downstream tools.
	ValueArray finalized;
	for(int i=0;i<events.GetCount();i++) { ValueMap e=events[i]; if(i==events.GetCount()-1 || Table(e)!=Table((ValueMap)events[i+1])) e("event_type","disappeared"); finalized.Add(e); }
	events=finalized;
	ValueMap result; result("input",input)("frame_gap",gap)("event_count",events.GetCount())("events",events); RealizeDirectory(out);
	String json=AppendFileName(out,"temporal_events.json"); SaveFile(json,AsJSON(result,true));
	String html="<!doctype html><meta charset=utf-8><title>Changed region gallery</title><style>body{font:14px sans-serif;background:#18202a;color:#eee}main{display:grid;grid-template-columns:repeat(auto-fill,minmax(260px,1fr));gap:12px}.card{background:#263342;padding:12px;border-radius:8px}img{max-width:100%;max-height:180px;display:block;margin:auto}</style><h1>Changed region temporal gallery</h1><main>";
	for(Value v:events) { ValueMap e=v; html<<"<section class=card><h2>"<<Html(S(e,"event_type"))<<" · table "<<I(e,"table_id")<<"</h2><p>frames "<<I(e,"start_frame")<<"–"<<I(e,"end_frame")<<"</p><pre>"<<Html(AsJSON(e,false))<<"</pre></section>"; }
	html<<"</main>"; SaveFile(AppendFileName(out,"gallery.html"),html); Cout()<<"events_json="<<json<<"\ngallery="<<AppendFileName(out,"gallery.html")<<"\n";
}
