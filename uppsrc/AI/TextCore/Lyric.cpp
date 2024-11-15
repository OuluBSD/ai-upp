#include "TextCore.h"


NAMESPACE_UPP



void RemoveSinger(String& s) {
	int a = s.Find(" by the singer");
	if (a >= 0)
		s = s.Left(a);
}


// TODO rename to GetLyricTypeString
String GetTextTypeString(int i) {
	switch (i) {
		case TXT_NULL:			return "Null";
		case TXT_NORMAL:		return "Verse";
		case TXT_PRE_REPEAT:	return "Pre-Chorus";
		case TXT_REPEAT:		return "Chorus";
		case TXT_TWIST:			return "Bridge";
		default:				return "<error>";
	}
}

void ParseTextPartType(String part_name, TextPartType& text_type, int& text_num) {
	part_name = TrimBoth(ToLower(part_name));
	for(int i = 0; i < TXT_COUNT; i++) {
		String cmp = ToLower(GetTextTypeString(i));
		if (part_name.Left(cmp.GetCount()) == cmp) {
			text_type = (TextPartType)i;
			if (part_name.GetCount() == cmp.GetCount()) {
				text_num = 0;
			}
			else {
				String tail = TrimLeft(part_name.Mid(cmp.GetCount()));
				if (tail.GetCount() && IsDigit(tail[0]))
					text_num = ScanInt(tail)-1;
			}
			return;
		}
	}
	text_type = TXT_NULL;
}


void LineElement::Overlay(const LineElement& le) {
	if (!le.element.IsEmpty()) element = le.element;
	if (!le.attr.group.IsEmpty()) attr.group = le.attr.group;
	if (!le.attr.value.IsEmpty()) attr.value = le.attr.value;
	if (!le.act.action.IsEmpty()) act.action = le.act.action;
	if (!le.act.arg.IsEmpty()) act.arg = le.act.arg;
	if (le.clr_i >= 0) clr_i = le.clr_i;
	if (le.typeclass_i >= 0) typeclass_i = le.typeclass_i;
	if (le.con_i >= 0) con_i = le.con_i;
}


String DynPart::GetName() const {
	String s = GetTextTypeString(text_type);
	if (text_num >= 0)
		s += " " + IntStr(text_num+1);
	return s;
}

int DynPart::GetExpectedLineCount() const {
	int line_count = 0;
	for (const auto& p : sub) {
		for (const auto& l : p.lines)
			if (l.text.GetCount())
				line_count++;
	}
	return line_count;
}

int DynPart::GetContrastIndex() const {
	int idx = PART_COUNT-1;
	
	if (text_type == TXT_NORMAL)
		idx = PART_BEGIN;
	
	if (text_type == TXT_PRE_REPEAT)
		idx = PART_MID;
	
	if (text_type == TXT_REPEAT)
		idx = PART_MID;
	
	if (text_type == TXT_TWIST)
		idx = PART_END;
	
	return idx;
}

String Script::GetUserText() const {
	String s;
	int line_i = 0;
	for (const DynPart& p: parts) {
		s << "[" << p.GetName() << "]\n";
		for(int i = 0; i < p.sub.GetCount(); i++) {
			const DynSub& ds = p.sub[i];
			for(int j = 0; j < ds.lines.GetCount(); j++) {
				const DynLine& dl = ds.lines[j];
				s << dl.user_text << "\n";
			}
		}
		s << "\n";
	}
	return s;
}

String DynPart::GetLineElementString(int line) const {
	String s;
	int line_i = 0;
	for(int i = 0; i < sub.GetCount(); i++) {
		const DynSub& ds = sub[i];
		for(int j = 0; j < ds.lines.GetCount(); j++) {
			const DynLine& dl = ds.lines[j];
			if (line == line_i) {
				return ds.el.element;
			}
			line_i++;
		}
	}
	return s;
}





String LineScore::Get(int i, int j) const {
	ASSERT(line_n > 0);
	ASSERT(j >= 0 && j < line_n);
	int pos = i * line_n + j;
	return lines[pos];
}

int LineScore::GetScore(int i, int j) const {
	ASSERT(i >= 0 && i < GetCount());
	ASSERT(j >= 0 && j < SCORE_COUNT);
	int pos = i * SCORE_COUNT + j;
	return scores[pos];
}

void LineScore::SetCount(int i, int line_n) {
	this->line_n = line_n;
	int total = i * line_n;
	int score_total = i * SCORE_COUNT;
	lines.SetCount(total);
	scores.SetCount(score_total, 0);
}

void LineScore::Set(int i, int j, const String& s) {
	ASSERT(line_n > 0);
	ASSERT(j >= 0 && j < line_n);
	int pos = i * line_n + j;
	lines[pos] = s;
}

void LineScore::SetScore(int i, int j, int value) {
	ASSERT(i >= 0 && i < GetCount());
	ASSERT(j >= 0 && j < SCORE_COUNT);
	int pos = i * SCORE_COUNT + j;
	scores[pos] = value;
}

int LineScore::GetCount() const {
	if (line_n == 0 || lines.GetCount() == 0)
		return 0;
	return lines.GetCount() / line_n;
}










Script::~Script() {
	
}

void Script::Store(Entity& a) {
	TODO
	/*String dir = a.GetScriptDir();
	RealizeDirectory(dir);
	String json_path = dir + file_title + ".json";
	StoreAsJsonFileStandard(*this, json_path, true);*/
}

void Script::LoadTitle(Entity& a, String title) {
	TODO
	/*String dir = a.GetScriptDir();
	file_title = title;
	String json_path = dir + file_title + ".json";
	LoadFromJsonFileStandard(*this, json_path);*/
}

String Script::GetAnyTitle() const {
	TODO
	/*if (native_title.GetCount())
		return native_title;
	
	return file_title;*/return "";
}

String Script::GetText() const {
	if (__text.GetCount())
		return __text;
	return GetUserText();
}

String Script::GetTextStructure(bool coarse) const {
	String out;
	for(const DynPart& dp : parts) {
		if (dp.text_type == TextPartType::TXT_NULL)
			continue;
		out << "[" << dp.GetName();
		if (!dp.person.IsEmpty())
			out << ": " << dp.person;
		out << "]\n";
		for (const DynSub& ds : dp.sub) {
			out.Cat('\t',1);
			out << "[" << ds.el.element << "]\n";
			for (const DynLine& dl : ds.lines) {
				out.Cat('\t',2);
				out << dl.user_text << "\n";
			}
		}
		out << "\n\n";
	}
	return out;
}

int Script::GetFirstPartPosition() const {
	#if 0
	for(int i = 0; i < active_struct.parts.GetCount(); i++) {
		String type = active_struct.parts[i];
		for(int j = 0; j < parts.GetCount(); j++) {
			if (parts[j].part_type != StaticPart::SKIP &&
				parts[j].type == type)
				return j;
		}
	}
	#endif
	return -1;
}

DynPart* Script::FindPartByName(const String& name) {
	String lname = ToLower(name);
	RemoveSinger(lname);
	for (DynPart& sp : parts)
		if (ToLower(sp.GetName()) == lname)
			return &sp;
	return 0;
}

void Script::LoadStructuredText(const String& s) {
	parts.Clear();
	Vector<String> lines = Split(s, "\n");
	int indent = 0;
	DynPart* part = 0;
	DynSub* sub = 0;
	String el1;
	for (String& l : lines) {
		l = TrimBoth(l);
		if (l.IsEmpty()) continue;
		int diff = 0;;
		if (l[0] == '[') {
			int a = 1;
			int b0 = l.Find("]");
			int b1 = l.Find(":");
			int b = -1;
			String part_name;
			if (b0 >= 0 && b1 >= 0) {
				b = min(b0,b1);
				b1++;
				part_name = l.Mid(b1,b0-b1);
			}
			else if (b0 >= 0) b = b0;
			else b = b1;
			
			if (b >= 0) {
				indent = Split(l.Mid(a,b-a),".").GetCount();
				
				String el;
				b = l.Find("]",b);
				a = l.Find("(",b);
				if (a >= 0) {
					a++;
					b = l.Find(")",a);
					el = l.Mid(a,b-a);
				}
				
				diff = -1;
				int level = indent + diff;
				if (level == 0) {
					part = &parts.Add();
					ParseTextPartType(part_name, part->text_type, part->text_num);
					part->el.element = el;
				}
				else if (level == 1) {
					el1 = el;
					sub = &part->sub.Add();
					sub->el.element = el;
				}
				else if (level == 2) {
					if (sub->lines.GetCount()) {
						sub = &part->sub.Add();
						sub->el.element = el1;
					}
					sub->el.element = el;
				}
			}
		}
		else {
			DynLine& dl = sub->lines.Add();
			dl.text = l;
		}
	}
}

void Script::SetEditText(const String& s) {
	Vector<String> sparts = Split(s, "[");
	int part_i = 0;
	for(int i = 0; i < sparts.GetCount(); i++) {
		String& p = sparts[i];
		Vector<String> lines = Split(p, "\n");
		lines.Remove(0);
		int line_i = 0;
		if (part_i >= parts.GetCount())
			break;
		auto& part = parts[part_i++];
		if (part.sub.IsEmpty() || (part.sub.GetCount() == 1 && part.sub[0].lines.IsEmpty())) {
			i--;
			continue;
		}
		for (auto& sub : part.sub) {
			for (auto& line : sub.lines) {
				if (line_i < lines.GetCount())
					line.edit_text = lines[line_i];
				else
					line.edit_text = "";
				line_i++;
			}
		}
	}
	while (part_i < parts.GetCount()) {
		auto& part = parts[part_i];
		for (auto& sub : part.sub) {
			for (auto& line : sub.lines) {
				line.edit_text = "";
			}
		}
		part_i++;
	}
}

void Script::LoadStructuredTextExt(const String& s) {
	Vector<String> lines = Split(s, "\n");
	int indent = 0;
	DynPart* part = 0;
	DynSub* sub = 0;
	int part_i = -1, sub_i = -1, line_i = -1;
	int sub_lines = 0;
	String el1;
	for (String& l : lines) {
		l = TrimBoth(l);
		if (l.IsEmpty()) continue;
		int diff = 0;;
		if (l[0] == '[') {
			int a = 1;
			int b0 = l.Find("]");
			int b1 = l.Find(":");
			int b = -1;
			//String part_name;
			if (b0 >= 0 && b1 >= 0) {
				b = min(b0,b1);
				b1++;
				//part_name = l.Mid(b1,b0-b1);
			}
			else if (b0 >= 0) b = b0;
			else b = b1;
			
			if (b >= 0) {
				indent = Split(l.Mid(a,b-a),".").GetCount();
				
				String el;
				b = l.Find("]",b);
				a = l.Find("(",b);
				if (a >= 0) {
					a++;
					b = l.Find(")",a);
					el = l.Mid(a,b-a);
				}
				
				diff = -1;
				int level = indent + diff;
				if (level == 0) {
					part_i++;
					sub_i = -1;
					line_i = -1;
					sub_lines = 0;
					if (part_i < parts.GetCount())
						part = &parts[part_i];
					else
						part = &parts.Add();
					//ParseTextPartType(part_name, part->text_type, part->text_num);
					//part->element = el;
				}
				else if (level == 1) {
					sub_i++;
					line_i = -1;
					sub_lines = 0;
					el1 = el;
					if (sub_i < part->sub.GetCount())
						sub = &part->sub[sub_i];
					else
						sub = &part->sub.Add();
					//sub->element0 = el;
				}
				else if (level == 2) {
					if (sub_lines > 0) {
						sub_i++;
						line_i = -1;
						sub_lines = 0;
						if (sub_i < part->sub.GetCount())
							sub = &part->sub[sub_i];
						else
							sub = &part->sub.Add();
						//sub->element0 = el1;
					}
					//sub->element1 = el;
				}
			}
		}
		else {
			line_i++;
			sub_lines++;
			DynLine* dl;
			if (line_i < sub->lines.GetCount())
				dl = &sub->lines[line_i];
			else
				dl = &sub->lines.Add();
			dl->alt_text = l;
		}
	}
}

double ScriptStruct::GetNormalScore() const {
	bool verse1 = false;
	bool verse2 = false;
	bool prerepeat1 = false;
	bool repeat1 = false;
	bool repeat2 = false;
	bool twist1 = false;
	
	for (const auto& part : parts) {
		if (!verse1 && part.type == TXT_NORMAL && part.num == 0)
			verse1 = true;
		if (!verse2 && part.type == TXT_NORMAL && part.num == 1)
			verse2 = true;
		if (!prerepeat1 && part.type == TXT_PRE_REPEAT && part.num == 0)
			prerepeat1 = true;
		if (!repeat1 && part.type == TXT_REPEAT && part.num == 0)
			repeat1 = true;
		if (!repeat2 && part.type == TXT_REPEAT && part.num == 1)
			repeat2 = true;
		if (!twist1 && part.type == TXT_TWIST && part.num == 0)
			twist1 = true;
	}
	double score =
		verse1 * 1.0 +
		verse2 * 0.9 +
		prerepeat1 * 0.5 +
		repeat1 * 1.0 +
		repeat2 * 0.6 +
		twist1 * 0.8
		;
	
	return score;
}


END_UPP_NAMESPACE
