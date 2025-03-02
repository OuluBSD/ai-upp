#include "AI.h"

NAMESPACE_UPP


String GetStringRange(String content, Point begin, Point end) {
	String ln = "\n";
	if (content.Find("\r\n") >= 0)
		ln = "\r\n";
	Vector<String> lines = Split(content, ln, false);
	if (end.y >= 0) {
		int c = lines.GetCount();
		int last_i = c-1;
		if (end.y < last_i) {
			int first_rm = end.y+1;
			int rm_count = c - first_rm;
			lines.Remove(first_rm, rm_count);
		}
		if (lines.GetCount()) {
			String& last_line = lines.Top();
			if (end.x < last_line.GetCount()) {
				last_line = last_line.Left(end.x);
			}
		}
	}
	if (begin.y > 0) {
		int first_rm = 0;
		int rm_count = min(lines.GetCount(), begin.y);
		if (rm_count > 0)
			lines.Remove(first_rm, rm_count);
		if (lines.GetCount()) {
			String& first_line = lines[0];
			if (begin.x > 0) {
				first_line = first_line.Mid(begin.x);
			}
		}
	}
	return Join(lines, "\n");
}

#if 0
bool UpdateMetaSrcFile(MetaSrcFile& f, const String& path)
{
	int i = CodeIndex().Find(path);
	if (i < 0)
		return false;
	FileAnnotation& fa = CodeIndex()[i];
	f.UpdateLinks(fa);
	return true;
}
#endif

bool RangeContains(const Point& pos, const Point& begin, const Point& end)
{
	if ( pos.y < begin.y ||
		(pos.y == begin.y && pos.x < begin.x) ||
		 pos.y > end.y ||
		(pos.y == end.y && pos.x > end.x)) {
		return false;
	}
	return true;
}

bool IsAllDigit(const String& s) {
	if (s.IsEmpty()) return false;
	const char* c = s.Begin();
	const char* e = s.End();
	while (c != e)
		if (!IsDigit(*c++))
			return false;
	return true;
}


// TODO: this function might be too dirty hack
ValueMap& ValueToMap(Value& val) {
	dword type = val.GetType();
	if (type != VALUEMAP_V)
		val = ValueMap();
	return *(ValueMap*)&val; // Illegal, but works -> better than crash in release mode
}

// TODO: this function might be too dirty hack
ValueArray& ValueToArray(Value& val) {
	dword type = val.GetType();
	if (type != VALUEARRAY_V)
		val = ValueArray();
	return *(ValueArray*)&val; // Illegal, but works -> better than crash in release mode
}

void RemoveColonTrail(String& s) {
	int a = s.Find(":");
	if (a >= 0)
		s = s.Left(a);
}

void RemoveCommentTrail(String& s) {
	int a = s.Find("//");
	if (a >= 0)
		s = TrimRight(s.Left(a));
}

double FractionDbl(const String& s) {
	int i = s.Find("/");
	if (i >= 0) {
		int num = i > 0 ? ScanInt(TrimLeft(s.Left(i))) : 1;
		int denom = ScanInt(TrimLeft(s.Mid(i+1)));
		double fract = (double)num / (double)denom;
		return fract;
	}
	else {
		int num = ScanInt(TrimLeft(s));
		return num;
	}
}

String GetDurationString(double p_seconds) {
	if (!p_seconds)
		return "0 seconds";
	int64 milliseconds = (int64)(p_seconds * 1000.0);
	int64 seconds = milliseconds / 1000LL;
	milliseconds = milliseconds % 1000LL;
	int64 minutes = seconds / 60LL;
	seconds = seconds % 60LL;
	int64 hours = minutes / 60LL;
	minutes = minutes % 60LL;
	int64 days = hours / 24LL;
	hours = hours % 24LL;
	int64 weeks = days / 7LL;
	days = days % 7LL;
	
	#define ITEM(x) \
	{if (x) { \
		if (!s.IsEmpty()) s << ", "; \
		s << x << " " #x; \
	}}
	String s;
	ITEM(weeks);
	ITEM(hours);
	ITEM(minutes);
	ITEM(seconds);
	if (s.IsEmpty() || p_seconds < 60.0)
		ITEM(milliseconds);
	return s;
}

String GetSizeString(uint64 bytes) {
	if (!bytes)
		return "0 bytes";
	bool show_bytes = bytes < 1024;
	#define PART(from, to) \
		uint64 to = from / 1024ULL; \
		from = from % 1024ULL;
	PART(bytes, Kb);
	PART(Kb, Mb);
	PART(Mb, Gb);
	PART(Gb, Tb);
	#define ITEM(x) \
	{if (x) { \
		if (!s.IsEmpty()) s << ", "; \
		s << x << " " #x; \
	}}
	String s;
	ITEM(Tb);
	ITEM(Gb);
	ITEM(Mb);
	ITEM(Kb);
	if (s.IsEmpty() || show_bytes)
		ITEM(bytes);
	return s;
}

Size GetAspectRatio(Size sz) {
	for(int i = sz.cx; i > 0; i--) {
		if ((sz.cx % i) == 0 && (sz.cy % i) == 0) {
			return Size(sz.cx / i, sz.cy / i);
		}
	}
	return sz;
}

END_UPP_NAMESPACE
