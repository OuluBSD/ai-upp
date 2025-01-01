#include "AI.h"

NAMESPACE_UPP


String GetStringRange(String content, Point begin, Point end) {
	String ln = "\n";
	if (content.Find("\r\n") >= 0)
		ln = "\r\n";
	Vector<String> lines = Split(content, ln, false);
	{
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

END_UPP_NAMESPACE
