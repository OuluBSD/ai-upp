#include "AICore.h"

NAMESPACE_UPP

FarStageCompiler::FarStageCompiler() {
	
}

bool FarStageCompiler::Compile(Nod& stage) {
	String txt = stage.value;
	txt.Replace("\r", "");
	
	// Split ranges with separator '#define'
	Vector<String> lines = Split(txt, "\n");
	int line_i = -1;
	VectorMap<String, Point> ranges;
	Point cur;
	String cur_header;
	cur.x = 0;
	for (String& line : lines) {
		line_i++;
		CParser p(line);
		if (p.Char('#')) {
			if (p.Id("define")) {
				cur.y = line_i;
				ranges.Add(cur_header, cur);
				cur_header = TrimBoth(p.GetPtr());
				cur.x = line_i+1;
			}
		}
	}
	cur.y = lines.GetCount()-1;
	ranges.Add(cur_header, cur);
	
	DUMPM(ranges);
	
	return true;
}

END_UPP_NAMESPACE
