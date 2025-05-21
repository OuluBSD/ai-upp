#include "AICore.h"

NAMESPACE_UPP

FarStageCompiler::FarStageCompiler() {
	
}

bool FarStageCompiler::Compile(Nod& stage_node) {
	bool succ = true;
	msgs.Clear();
	this->stage.Clear();
	this->stage.Create();
	
	String txt = stage_node.value;
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
	//DUMPM(ranges);
	
	{
		Point& p = ranges[0];
		String s;
		for(int i = p.x, j = 0; i < p.y; i++, j++) {
			if (j)
				s += "\n";
			s += lines[i];
		}
		this->stage->body = s;
		this->stage->value = ParseJSON(s, false);
		if (stage->value.IsError()) {
			ProcMsg& m = msgs.Add();
			m.severity = PROCMSG_ERROR;
			m.msg = GetErrorText(stage->value);
			succ = false;
		}
	}
	
	for(int i = 1; i < ranges.GetCount(); i++) {
		String header = ranges.GetKey(i);
		auto& fn = this->stage->funcs.Add();
		CParser p(header);
		try {
			fn.name = p.ReadId();
			p.PassChar('(');
			while (!p.Char(')')) {
				if (!fn.params.IsEmpty())
					p.PassChar(',');
				fn.params.Add(p.ReadString());
			}
			p.PassChar2('-','>');
			fn.ret = p.ReadString();
		}
		catch (Exc e) {
			ProcMsg& m = msgs.Add();
			m.severity = PROCMSG_ERROR;
			m.msg = e;
			succ = false;
		}
		Point& pt = ranges[i];
		String s;
		for(int i = pt.x, j = 0; i < pt.y; i++, j++) {
			if (j)
				s += "\n";
			s += lines[i];
		}
		fn.body = s;
		fn.value = ParseJSON(s, false);
		if (fn.value.IsError()) {
			ProcMsg& m = msgs.Add();
			m.severity = PROCMSG_ERROR;
			m.msg = GetErrorText(fn.value);
			succ = false;
		}
		
		// Create declaration for Esc script
		fn.esc_declaration = fn.name + "(";
		int param_i = 0;
		for (const auto& param : fn.params) {
			VfsPath p;
			p.Set(param);
			if (p.IsEmpty()) {
				ProcMsg& m = msgs.Add();
				m.severity = PROCMSG_ERROR;
				m.msg = "invalid param in function " + fn.name;
				return false;
			}
			if (param_i)
				fn.esc_declaration.Cat(',');
			fn.esc_declaration += p.TopPart().ToString();
			param_i++;
		}
		fn.esc_declaration += ")";
		
		// Create hash by Visitor (TODO cleanup ugliness)
		fn.hash = 0;
		Visitor v((hash_t)0);
		v VISN(fn);
		fn.hash = v.hash;
	}
	
	// Create hash by Visitor (TODO cleanup ugliness)
	stage->hash = 0;
	Visitor v((hash_t)0);
	v VISN(*stage);
	stage->hash = v.hash;
	
	return succ;
}

END_UPP_NAMESPACE
