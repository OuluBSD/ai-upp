#include "Core.h"

namespace Upp {

// YAML parser with advanced features (multi-line, anchors, flow style)
class YamlParser {
	Vector<String> lines;
	int            line_idx;
	
	// Use parallel vectors for anchors to avoid ValueMap corruption issue
	Vector<String> anchor_names;
	Vector<Value>  anchor_values;
	
	int    GetLineIndent(int line) const;
	String GetLine(int line) const;
	bool   IsEnd() const;
	void   SkipCommentsAndBlanks();
	Value  ParseScalar(const String& s);
	Value  ParseFlowStyle(const String& s);  // Handle {map} and [list]
	Value  ParseValue(int min_indent);
	Value  ParseList(int min_indent);
	Value  ParseMap(int min_indent);
	String Trim(const String& s);
	String Unquote(const String& s);
	String ParseAnchorName(String& s);  // Extract &name from string
	String ParseAliasName(const String& s);  // Extract *name from string
	void   SetAnchor(const String& name, const Value& v);
	Value  GetAnchor(const String& name);

public:
	YamlParser(const String& yaml);

	Value Parse();
};

YamlParser::YamlParser(const String& yaml)
{
	// Split into lines
	const char* p = yaml;
	while(*p) {
		String line;
		while(*p && *p != '\n' && *p != '\r')
			line << *p++;
		lines.Add(line);
		if(*p == '\r') p++;
		if(*p == '\n') p++;
	}
	line_idx = 0;
}

String YamlParser::Trim(const String& s)
{
	int i = 0, j = s.GetCount();
	while(i < j && (s[i] == ' ' || s[i] == '\t')) i++;
	while(j > i && (s[j-1] == ' ' || s[j-1] == '\t')) j--;
	return s.Mid(i, j - i);
}

String YamlParser::Unquote(const String& s)
{
	if(s.GetCount() < 2)
		return s;
	if((s[0] == '"' && s[s.GetCount()-1] == '"') || (s[0] == '\'' && s[s.GetCount()-1] == '\''))
		return s.Mid(1, s.GetCount() - 2);
	return s;
}

String YamlParser::ParseAnchorName(String& s)
{
	int amp_pos = s.Find('&');
	if(amp_pos < 0) return String();
	
	int name_start = amp_pos + 1;
	int name_end = name_start;
	while(name_end < s.GetCount() && s[name_end] != ' ' && s[name_end] != '\t' && s[name_end] != ':' && s[name_end] != '#')
		name_end++;
	
	if(name_end <= name_start) return String();
	
	String anchor_name = s.Mid(name_start, name_end - name_start);
	String result = s.Left(amp_pos);
	if(name_end < s.GetCount()) result << s.Mid(name_end);
	s = Trim(result);
	return anchor_name;
}

String YamlParser::ParseAliasName(const String& s)
{
	String trimmed = Trim(s);
	if(trimmed.IsEmpty() || trimmed[0] != '*') return String();
	
	int name_end = 1;
	while(name_end < trimmed.GetCount() && trimmed[name_end] != ' ' && trimmed[name_end] != '\t' && trimmed[name_end] != '#')
		name_end++;
	
	return name_end > 1 ? trimmed.Mid(1, name_end - 1) : String();
}

void YamlParser::SetAnchor(const String& name, const Value& v)
{
	// Check if anchor already exists
	for(int i = 0; i < anchor_names.GetCount(); i++) {
		if(anchor_names[i] == name) {
			anchor_values[i] = v;
			return;
		}
	}
	// Add new anchor
	anchor_names.Add(name);
	anchor_values.Add(v);
}

Value YamlParser::GetAnchor(const String& name)
{
	for(int i = 0; i < anchor_names.GetCount(); i++) {
		if(anchor_names[i] == name)
			return anchor_values[i];
	}
	return Null;
}

// Parse flow style: {key: value} or [item, item]
Value YamlParser::ParseFlowStyle(const String& s)
{
	String trimmed = Trim(s);
	if(trimmed.IsEmpty() || (trimmed[0] != '{' && trimmed[0] != '['))
		return Null;
	
	if(trimmed[0] == '{') {
		ValueMap m;
		String content = trimmed.Mid(1);
		int end = content.Find('}');
		if(end >= 0) content = content.Left(end);
		
		int pos = 0;
		while(pos < content.GetCount()) {
			while(pos < content.GetCount() && (content[pos] == ' ' || content[pos] == '\t' || content[pos] == '\n')) pos++;
			if(pos >= content.GetCount()) break;
			
			// Find key
			bool in_q = false; char q_c = 0;
			int key_start = pos;
			while(pos < content.GetCount()) {
				char c = content[pos];
				if((c == '"' || c == '\'') && (pos == 0 || content[pos-1] != '\\')) {
					in_q = !in_q; q_c = c;
				} else if(c == ':' && !in_q) break;
				pos++;
			}
			if(pos >= content.GetCount()) break;
			
			String key = Unquote(Trim(content.Mid(key_start, pos - key_start)));
			pos++; // skip ':'
			while(pos < content.GetCount() && (content[pos] == ' ' || content[pos] == '\t')) pos++;
			
			// Find value
			int v_start = pos, brace = 0, bracket = 0;
			in_q = false;
			while(pos < content.GetCount()) {
				char c = content[pos];
				if((c == '"' || c == '\'') && (pos == 0 || content[pos-1] != '\\')) in_q = !in_q;
				else if(!in_q) {
					if(c == '{') brace++; else if(c == '}') brace--;
					else if(c == '[') bracket++; else if(c == ']') bracket--;
					else if(c == ',' && brace == 0 && bracket == 0) break;
				}
				pos++;
			}
			
			String v_str = Trim(content.Mid(v_start, pos - v_start));
			Value v;
			if(v_str.GetCount() > 0 && (v_str[0] == '{' || v_str[0] == '['))
				v = ParseFlowStyle(v_str);
			else if(v_str.GetCount() > 0)
				v = ParseScalar(v_str);
			else
				v = Null;
			
			m.Add(key, v);
			if(pos < content.GetCount() && content[pos] == ',') pos++;
		}
		return m;
	}
	else {
		ValueArray va;
		String content = trimmed.Mid(1);
		int end = content.Find(']');
		if(end >= 0) content = content.Left(end);
		
		int pos = 0;
		while(pos < content.GetCount()) {
			while(pos < content.GetCount() && (content[pos] == ' ' || content[pos] == '\t' || content[pos] == '\n')) pos++;
			if(pos >= content.GetCount()) break;
			
			int item_start = pos, brace = 0, bracket = 0;
			bool in_q = false;
			while(pos < content.GetCount()) {
				char c = content[pos];
				if((c == '"' || c == '\'') && (pos == 0 || content[pos-1] != '\\')) in_q = !in_q;
				else if(!in_q) {
					if(c == '{') brace++; else if(c == '}') brace--;
					else if(c == '[') bracket++; else if(c == ']') bracket--;
					else if(c == ',' && brace == 0 && bracket == 0) break;
				}
				pos++;
			}
			
			String item = Trim(content.Mid(item_start, pos - item_start));
			if(item.GetCount() > 0) {
				if(item[0] == '{' || item[0] == '[')
					va.Add(ParseFlowStyle(item));
				else
					va.Add(ParseScalar(item));
			}
			if(pos < content.GetCount() && content[pos] == ',') pos++;
		}
		return va;
	}
}

int YamlParser::GetLineIndent(int line) const
{
	if(line >= lines.GetCount())
		return 0;
	const String& s = lines[line];
	int indent = 0;
	for(int i = 0; i < s.GetCount() && (s[i] == ' ' || s[i] == '\t'); i++) {
		if(s[i] == '\t')
			indent += 2;
		else
			indent++;
	}
	return indent;
}

String YamlParser::GetLine(int line) const
{
	if(line >= lines.GetCount())
		return "";
	return lines[line];
}

bool YamlParser::IsEnd() const
{
	return line_idx >= lines.GetCount();
}

void YamlParser::SkipCommentsAndBlanks()
{
	while(line_idx < lines.GetCount()) {
		String line = GetLine(line_idx);
		String trimmed = Trim(line);
		// Skip empty lines and comments
		if(trimmed.IsEmpty() || trimmed[0] == '#') {
			line_idx++;
			continue;
		}
		break;
	}
}

Value YamlParser::ParseScalar(const String& s)
{
	String trimmed = Trim(s);

	// Check for alias (*name) first
	String alias_name = ParseAliasName(trimmed);
	if(!alias_name.IsEmpty()) {
		Value v = GetAnchor(alias_name);
		return v.IsVoid() ? Null : v;
	}
	
	// Check for flow style
	if(trimmed.GetCount() > 0 && (trimmed[0] == '{' || trimmed[0] == '['))
		return ParseFlowStyle(trimmed);

	// Remove inline comment
	int comment_pos = -1;
	bool in_quote = false;
	char quote_char = 0;
	for(int i = 0; i < trimmed.GetCount(); i++) {
		char c = trimmed[i];
		if((c == '"' || c == '\'') && (i == 0 || trimmed[i-1] != '\\')) {
			in_quote = !in_quote; quote_char = c;
		} else if(c == '#' && !in_quote) {
			comment_pos = i; break;
		}
	}
	if(comment_pos >= 0) trimmed = Trim(trimmed.Left(comment_pos));
	if(trimmed.IsEmpty()) return Null;

	// Check for multi-line string (| or >)
	if(trimmed[0] == '|' || trimmed[0] == '>') {
		char ml = trimmed[0];
		String result;
		int base_indent = -1;
		// Note: line_idx was already incremented by ParseMap, so we're at the first content line
		
		while(!IsEnd()) {
			String line = GetLine(line_idx);
			int indent = GetLineIndent(line_idx);
			String t = Trim(line);
			
			if(t.IsEmpty() || t[0] == '#') {
				if(ml == '|') { if(result.GetCount()) result << '\n'; }
				else { if(result.GetCount() && *result.End() != '\n') result << "\n\n"; }
				line_idx++; continue;
			}
			
			if(base_indent < 0) base_indent = indent;
			if(indent < base_indent) break;
			
			String content = line.Mid(base_indent);
			if(ml == '>') {
				if(result.GetCount() && *result.End() != '\n') result << ' ';
				result << content;
			} else {
				if(result.GetCount()) result << '\n';
				result << content;
			}
			line_idx++;
		}
		return Value(result);
	}

	// Check for anchor (&name)
	String anchor_name = ParseAnchorName(trimmed);
	
	// Quoted strings
	if(trimmed[0] == '"') {
		String r;
		for(int i = 1; i < trimmed.GetCount(); i++) {
			if(trimmed[i] == '"' && trimmed[i-1] != '\\') break;
			if(trimmed[i] == '\\' && i+1 < trimmed.GetCount()) {
				i++;
				switch(trimmed[i]) {
					case 'n': r << '\n'; break; case 'r': r << '\r'; break;
					case 't': r << '\t'; break; case '\\': r << '\\'; break;
					case '"': r << '"'; break; case '\'': r << '\''; break;
					default: r << (char)trimmed[i];
				}
			} else r << (char)trimmed[i];
		}
		Value v(r);
		if(!anchor_name.IsEmpty()) SetAnchor(anchor_name, v);
		return v;
	}

	if(trimmed[0] == '\'') {
		String r;
		for(int i = 1; i < trimmed.GetCount(); i++) {
			if(trimmed[i] == '\'' && trimmed[i-1] != '\\') break;
			if(trimmed[i] == '\\' && i+1 < trimmed.GetCount() && trimmed[i+1] == '\'') { i++; r << '\''; }
			else r << (char)trimmed[i];
		}
		Value v(r);
		if(!anchor_name.IsEmpty()) SetAnchor(anchor_name, v);
		return v;
	}

	// Special values
	if(trimmed == "null" || trimmed == "~") return Null;
	if(trimmed == "true") return true;
	if(trimmed == "false") return false;

	// Numbers
	const char* p = trimmed;
	bool is_num = true, has_dot = false;
	if(*p == '-' || *p == '+') p++;
	while(*p) {
		if(*p >= '0' && *p <= '9') p++;
		else if(*p == '.' && !has_dot) { has_dot = true; p++; }
		else { is_num = false; break; }
	}

	Value v;
	if(is_num && trimmed.GetCount() > 0)
		v = has_dot ? ScanDouble(trimmed) : ScanInt(trimmed);
	else
		v = Value(trimmed);

	if(!anchor_name.IsEmpty()) SetAnchor(anchor_name, v);
	return v;
}

Value YamlParser::ParseValue(int min_indent)
{
	SkipCommentsAndBlanks();
	
	if(IsEnd())
		return Null;
	
	String line = GetLine(line_idx);
	String trimmed = Trim(line);
	int line_indent = GetLineIndent(line_idx);
	
	// Check if this line belongs to this indent level
	if(line_indent < min_indent)
		return Null;
	
	// Check if it's a list item
	if(trimmed.StartsWith("-")) {
		return ParseList(line_indent);
	}
	
	// Otherwise it's a map
	return ParseMap(line_indent);
}

Value YamlParser::ParseList(int min_indent)
{
	ValueArray va;
	
	while(!IsEnd()) {
		SkipCommentsAndBlanks();
		if(IsEnd())
			break;
		
		String line = GetLine(line_idx);
		int line_indent = GetLineIndent(line_idx);
		String trimmed = Trim(line);
		
		// If less indented than min_indent, end of list
		if(line_indent < min_indent)
			break;
		
		// Must start with "-" to be a list item at this level
		if(!trimmed.StartsWith("-"))
			break;
		
		// Get content after "- "
		String content;
		if(trimmed.GetCount() > 1) {
			if(trimmed[1] == ' ' || trimmed[1] == '\t')
				content = trimmed.Mid(2);
			else
				content = trimmed.Mid(1);
		}
		
		// Advance past this line
		line_idx++;
		
		// Empty list item or comment only
		if(content.IsEmpty() || content[0] == '#') {
			// Check for nested structure on next line
			SkipCommentsAndBlanks();
			if(!IsEnd()) {
				int next_indent = GetLineIndent(line_idx);
				String next_trimmed = Trim(GetLine(line_idx));
				if(next_indent > line_indent) {
					if(next_trimmed.StartsWith("-"))
						va.Add(ParseList(next_indent));
					else
						va.Add(ParseMap(next_indent));
				}
				else
					va.Add(Null);
			}
			else
				va.Add(Null);
			continue;
		}
		
		// Check if content has a colon (map entry like "- key: value")
		int colon_pos = -1;
		bool in_quote = false;
		char quote_char = 0;
		for(int i = 0; i < content.GetCount(); i++) {
			char c = content[i];
			if((c == '"' || c == '\'') && (i == 0 || content[i-1] != '\\')) {
				if(!in_quote) {
					in_quote = true;
					quote_char = c;
				}
				else if(c == quote_char) {
					in_quote = false;
				}
			}
			else if(c == ':' && !in_quote) {
				// Check if followed by space or end of string
				if(i+1 >= content.GetCount() || content[i+1] == ' ' || content[i+1] == '\t') {
					colon_pos = i;
					break;
				}
			}
		}
		
		if(colon_pos >= 0) {
			// It's a map entry - the whole line is a map item
			// Parse the key from content
			String key = Unquote(Trim(content.Left(colon_pos)));
			String value_part = content.Mid(colon_pos + 1);
			
			Value value;
			if(!value_part.IsEmpty() && value_part[0] != '#' && Trim(value_part).GetCount() > 0) {
				// Value on same line
				value = ParseScalar(value_part);
			}
			else {
				// Value on next line(s)
				SkipCommentsAndBlanks();
				if(!IsEnd()) {
					int next_indent = GetLineIndent(line_idx);
					if(next_indent > line_indent) {
						String next_trimmed = Trim(GetLine(line_idx));
						if(next_trimmed.StartsWith("-"))
							value = ParseList(next_indent);
						else if(next_trimmed.Find(':') >= 0)
							value = ParseMap(next_indent);
						else
							value = ParseScalar(GetLine(line_idx));
					}
					else
						value = Null;
				}
				else
					value = Null;
			}
			
			// Now parse remaining keys at the same indent level as the list item content
			ValueMap m;
			m.Add(key, value);
			
			// Continue parsing more keys for this map
			while(!IsEnd()) {
				SkipCommentsAndBlanks();
				if(IsEnd())
					break;

				String cont_line = GetLine(line_idx);
				int cont_indent = GetLineIndent(line_idx);
				String cont_trimmed = Trim(cont_line);
				
				// If less indented than list item content, end of this map
				if(cont_indent <= line_indent)
					break;
				
				// If it's a list item, end of this map
				if(cont_trimmed.StartsWith("-"))
					break;
				
				// Parse key: value
				int cp = -1;
				in_quote = false;
				quote_char = 0;
				for(int i = 0; i < cont_trimmed.GetCount(); i++) {
					char c = cont_trimmed[i];
					if((c == '"' || c == '\'') && (i == 0 || cont_trimmed[i-1] != '\\')) {
						if(!in_quote) {
							in_quote = true;
							quote_char = c;
						}
						else if(c == quote_char) {
							in_quote = false;
						}
					}
					else if(c == ':' && !in_quote) {
						cp = i;
						break;
					}
				}
				
				if(cp < 0) {
					line_idx++;
					continue;
				}

				String k = Unquote(Trim(cont_trimmed.Left(cp)));
				String v_part = cont_trimmed.Mid(cp + 1);
				
				line_idx++;
				
				Value v;
				if(!v_part.IsEmpty() && v_part[0] != '#' && Trim(v_part).GetCount() > 0) {
					v = ParseScalar(v_part);
				}
				else {
					SkipCommentsAndBlanks();
					if(!IsEnd()) {
						int ni = GetLineIndent(line_idx);
						String nt = Trim(GetLine(line_idx));
						if(ni > cont_indent) {
							if(nt.StartsWith("-"))
								v = ParseList(ni);
							else if(nt.Find(':') >= 0)
								v = ParseMap(ni);
							else
								v = ParseScalar(GetLine(line_idx));
						}
						else
							v = Null;
					}
					else
						v = Null;
				}
				
				m.Add(k, v);
			}
			
			va.Add(m);
		}
		else {
			// Simple scalar value
			va.Add(ParseScalar(content));
		}
	}
	
	return va;
}

Value YamlParser::ParseMap(int min_indent)
{
	ValueMap m;
	
	while(!IsEnd()) {
		SkipCommentsAndBlanks();
		if(IsEnd())
			break;
		
		String line = GetLine(line_idx);
		int line_indent = GetLineIndent(line_idx);
		String trimmed = Trim(line);
		
		// If less indented than min_indent, end of map
		if(line_indent < min_indent)
			break;
		
		// If it's a list item at same or less indent, end of map
		if(trimmed.StartsWith("-") && line_indent <= min_indent)
			break;
		
		// Parse key: value
		int colon_pos = -1;
		bool in_quote = false;
		char quote_char = 0;
		for(int i = 0; i < trimmed.GetCount(); i++) {
			char c = trimmed[i];
			if((c == '"' || c == '\'') && (i == 0 || trimmed[i-1] != '\\')) {
				if(!in_quote) {
					in_quote = true;
					quote_char = c;
				}
				else if(c == quote_char) {
					in_quote = false;
				}
			}
			else if(c == ':' && !in_quote) {
				colon_pos = i;
				break;
			}
		}
		
		if(colon_pos < 0) {
			// No colon found - skip malformed line
			line_idx++;
			continue;
		}

		String key = Unquote(Trim(trimmed.Left(colon_pos)));
		String value_part = trimmed.Mid(colon_pos + 1);
		
		// Check for anchor in value_part
		String anchor_name;
		String vp_trimmed = Trim(value_part);
		if(vp_trimmed.Find('&') >= 0) {
			anchor_name = ParseAnchorName(vp_trimmed);
		}

		line_idx++;

		// Check if value is on same line (excluding just anchor)
		if(!vp_trimmed.IsEmpty() && vp_trimmed[0] != '#' && !vp_trimmed.StartsWith("&")) {
			// Value on same line
			m.Add(key, ParseScalar(value_part));
		}
		else {
			// Value on next line(s) - check for nested structure
			SkipCommentsAndBlanks();

			if(IsEnd()) {
				m.Add(key, Null);
				break;
			}

			int next_indent = GetLineIndent(line_idx);
			String next_trimmed = Trim(GetLine(line_idx));

			// If next line is not more indented, value is null
			if(next_indent <= line_indent) {
				m.Add(key, Null);
			}
			else if(next_trimmed.StartsWith("-")) {
				// Nested list
				Value v = ParseList(next_indent);
				if(!anchor_name.IsEmpty()) SetAnchor(anchor_name, v);
				m.Add(key, v);
			}
			else {
				// Nested map or scalar
				Value v = ParseValue(next_indent);
				if(!anchor_name.IsEmpty()) SetAnchor(anchor_name, v);
				m.Add(key, v);
			}
		}
	}
	
	return m;
}

Value YamlParser::Parse()
{
	SkipCommentsAndBlanks();
	
	if(IsEnd())
		return Null;
	
	String line = GetLine(line_idx);
	String trimmed = Trim(line);
	int indent = GetLineIndent(line_idx);
	
	if(trimmed.StartsWith("-"))
		return ParseList(indent);
	else
		return ParseMap(indent);
}

Value ParseYAML(CParser& p, bool strict)
{
	(void)strict; // Not used in new parser
	String yaml = p.GetPtr();
	YamlParser yp(yaml);
	return yp.Parse();
}

Value ParseYAML(const char* s, bool strict)
{
	(void)strict; // Not used in new parser
	YamlParser yp(s);
	return yp.Parse();
}

String AsYAML(Time tm)
{
	return IsNull(tm) ? String("null") : Format("%04d-%02d-%02dT%02d:%02d:%02d",
		tm.year, tm.month, tm.day, tm.hour, tm.minute, tm.second);
}

String AsYAML(Date dt)
{
	return IsNull(dt) ? String("null") : Format("%04d-%02d-%02d", dt.year, dt.month, dt.day);
}

void Yaml::Indent()
{
	for(int i = 0; i < indent_level; i++)
		text << "  ";
}

Yaml& Yaml::CatRaw(const char* key, const String& val)
{
	if(text.GetCount() && *text.End() != '\n')
		text << "\n";
	Indent();
	text << AsYAML(key) << ": ";
	if(val.Find('\n') >= 0) {
		// Multi-line value
		text << "|\n";
		Vector<String> lines = Split(val, '\n');
		for(int i = 0; i < lines.GetCount(); i++) {
			Indent();
			text << "  " << lines[i];
			if(i < lines.GetCount() - 1)
				text << "\n";
		}
	}
	else
		text << val;
	text << "\n";
	return *this;
}

YamlArray& YamlArray::CatRaw(const String& val)
{
	if(text.GetCount() && *text.End() != '\n')
		text << "\n";
	Indent();
	text << "- " << val << "\n";
	return *this;
}

void YamlArray::Indent()
{
	for(int i = 0; i < indent_level; i++)
		text << "  ";
}

String AsYAML(const Value& v, const String& indent, bool pretty)
{
	String r;
	int level = indent.GetCount() / 2;

	if(v.GetType() == VALUEMAP_V) {
		ValueMap m = v;
		ValueArray va = m.GetValues();
		for(int i = 0; i < m.GetCount(); i++) {
			if(pretty)
				r << indent;
			r << AsYAML((String)m.GetKey(i)) << ": ";
			Value val = va[i];
			if(val.GetType() == VALUEMAP_V || val.GetType() == VALUEARRAY_V) {
				r << "\n";
				r << AsYAML(val, indent + "  ", pretty);
			}
			else {
				r << AsYAML(val, String(), false);
				if(pretty || i < m.GetCount() - 1)
					r << "\n";
			}
		}
		return r;
	}
	if(v.GetType() == VALUEARRAY_V) {
		ValueArray va = v;
		for(int i = 0; i < va.GetCount(); i++) {
			if(pretty)
				r << indent;
			r << "- ";
			Value val = va[i];
			if(val.GetType() == VALUEMAP_V || val.GetType() == VALUEARRAY_V) {
				r << "\n";
				r << AsYAML(val, indent + "  ", pretty);
			}
			else {
				r << AsYAML(val, String(), false);
				if(pretty || i < va.GetCount() - 1)
					r << "\n";
			}
		}
		return r;
	}
	if(IsNumber(v) && (IsNull(v) || IsNaN((double)v)))
		return "null";
	if(v.GetType() == INT_V)
		return FormatInt((int)v);
	if(v.GetType() == BOOL_V)
		return (bool)v ? "true" : "false";
	if(v.GetType() == FLOAT_V)
		return FormatG((double)v, 7);
	if(IsNumber(v))
		return FormatG((double)v, 15);
	if(IsString(v))
		return AsYAML((String)v);
	if(IsDateTime(v))
		return AsYAML((Time)v);
	if(IsNull(v))
		return "null";
	return "null";
}

void YamlIO::Set(const char* key, const Value& v)
{
	ASSERT(IsStoring());
	if(!map)
		map.Create();
	map->Add(key, v);
}

String AsYAML(const Value& v, bool pretty)
{
	return AsYAML(v, String(), pretty);
}

template<> void Yamlize(YamlIO& io, double& var)
{
	if(io.IsLoading()) {
		const Value& v = io.Get();
		if(IsNull(v)) {
			var = Null;
			return;
		}
		if(IsNumber(v)) {
			var = io.Get();
			return;
		}
		if(IsString(v)) {
			double h = ScanDouble((String)v);
			if(!IsNull(h)) {
				var = h;
				return;
			}
		}
		throw YamlizeError("number expected");
	}
	else
		io.Set(var);
}

template<> void Yamlize(YamlIO& io, float& var)
{
	if(io.IsLoading()) {
		const Value& v = io.Get();
		if(IsNull(v)) {
			var = Null;
			return;
		}
		if(IsNumber(v)) {
			var = io.Get();
			return;
		}
		if(IsString(v)) {
			double h = ScanDouble((String)v);
			if(!IsNull(h)) {
				var = (float)h;
				return;
			}
		}
		throw YamlizeError("number expected");
	}
	else
		io.Set(var);
}

template<> void Yamlize(YamlIO& io, int& var)
{
	double v = IntDbl(var);
	Yamlize(io, v);
	if(io.IsLoading()) {
		if(IsNull(v))
			var = Null;
		else
		if(v >= INT_MIN && v <= INT_MAX && (int)v == v)
			var = (int)v;
		else
			throw YamlizeError("number is not integer");
	}
}

template<> void Yamlize(YamlIO& io, uint32& var)
{
	double v = IntDbl(var);
	Yamlize(io, v);
	if(io.IsLoading()) {
		if(IsNull(v))
			var = 0;
		else
		if(v >= 0 && v <= UINT32_MAX && (int)v == v)
			var = (int)v;
		else
			throw YamlizeError("number is not integer");
	}
}

template<> void Yamlize(YamlIO& io, byte& var)
{
	double v = var;
	Yamlize(io, v);
	if(io.IsLoading()) {
		if(v >= 0 && v <= 255 && (int)v == v)
			var = (byte)v;
		else
			throw YamlizeError("integer 0-255 expected");
	}
}

template<> void Yamlize(YamlIO& io, int16& var)
{
	double v = var;
	Yamlize(io, v);
	if(io.IsLoading()) {
		if(v >= -32768 && v <= 32767 && (int)v == v)
			var = (int16)v;
		else
			throw YamlizeError("16-bit integer expected");
	}
}

template<> void Yamlize(YamlIO& io, bool& var)
{
	if(io.IsLoading()) {
		const Value& v = io.Get();
		if(IsNumber(v) && !IsNull(v))
			var = (bool)v;
		else
			throw YamlizeError("boolean expected");
	}
	else
		io.Set(var);
}

template<> void Yamlize(YamlIO& io, int64& var)
{
	if(io.IsLoading()) {
		const Value& v = io.Get();
		if(IsNull(v)) {
			var = Null;
			return;
		}
		if(v.Is<int64>() || v.Is<int>()) {
			var = v;
			return;
		}
		if(IsNumber(v)) {
			double d = v;
			if(FitsInInt64(d)) {
				var = (int64)d;
				return;
			}
		}
		else
		if(IsString(v)) {
			int64 h = ScanInt64((String)v);
			if(!IsNull(h)) {
				var = h;
				return;
			}
		}
		throw YamlizeError("invalid int64 value");
	}
	else
		if(IsNull(var))
			io.Set(Null);
		else
		if(var >= I64(-9007199254740992) && var <= I64(9007199254740991))
			io.Set(var);
		else
			io.Set(AsString(var));
}

#ifdef CPU_64
template<> void Yamlize(YamlIO& io, hash_t& var)
{
	Yamlize(io, (int64&)var);
}
#endif

template<> void Yamlize(YamlIO& io, String& var)
{
	if(io.IsLoading()) {
		const Value& v = io.Get();
		if(IsNull(v))
			var = Null;
		else
		if(IsString(v))
			var = v;
		else
			throw YamlizeError("string expected");
	}
	else
		io.Set(var);
}

template<> void Yamlize(YamlIO& io, WString& var)
{
	if(io.IsLoading()) {
		const Value& v = io.Get();
		if(IsNull(v))
			var = Null;
		else
		if(IsString(v))
			var = v;
		else
			throw YamlizeError("string expected");
	}
	else
		io.Set(var);
}

template<> void Yamlize(YamlIO& io, Date& var)
{
	if(io.IsLoading()) {
		const Value& v = io.Get();
		if(IsNull(v)) {
			var = Null;
			return;
		}
		if(IsString(v)) {
			String text = Filter(~v, CharFilterDigit);
			if(text.GetCount() >= 8) {
				Date d;
				d.year = ScanInt(text.Left(4));
				d.month = ScanInt(text.Mid(4, 2));
				d.day = ScanInt(text.Mid(6, 2));
				if(d.IsValid()) {
					var = d;
					return;
				}
			}
		}
		throw YamlizeError("string expected for Date value");
	}
	else
		if(IsNull(var))
			io.Set(Null);
		else
			io.Set(Format("%04d-%02d-%02d", var.year, var.month, var.day));
}

template<> void Yamlize(YamlIO& io, Time& var)
{
	if(io.IsLoading()) {
		const Value& v = io.Get();
		if(IsNull(v)) {
			var = Null;
			return;
		}
		if(IsString(v)) {
			String text = Filter(~v, CharFilterDigit);
			if(text.GetCount() >= 12) {
				Time tm;
				tm.year = ScanInt(text.Left(4));
				tm.month = ScanInt(text.Mid(4, 2));
				tm.day = ScanInt(text.Mid(6, 2));
				tm.hour = ScanInt(text.Mid(8, 2));
				tm.minute = ScanInt(text.Mid(10, 2));
				tm.second = text.GetCount() > 12 ? ScanInt(text.Mid(12)) : 0;
				if(tm.IsValid()) {
					var = tm;
					return;
				}
			}
		}
		throw YamlizeError("string expected for Time value");
	}
	else
		if(IsNull(var))
			io.Set(Null);
		else
			io.Set(Format("%04d-%02d-%02dT%02d:%02d:%02d",
				          var.year, var.month, var.day, var.hour, var.minute, var.second));
}

String sYamlFile(const char* file)
{
	return file ? String(file) : ConfigFile(GetExeTitle() + ".yaml");
}

}
