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

String AppendUnixFileName(String a, String b) {
	if (a.GetCount() && a[a.GetCount()-1] == '/')
		a = a.Left(a.GetCount()-1);
	if (b.GetCount() && b[0] == '/')
		b = b.Mid(1);
	return a + "/" + b;
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

END_UPP_NAMESPACE
