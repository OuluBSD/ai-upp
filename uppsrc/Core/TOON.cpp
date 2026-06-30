#include "Core.h"

namespace Upp {

struct ToonToken : Moveable<ToonToken> {
	String value;
	bool is_quoted;
};

struct Line : Moveable<Line> {
	int line_index;
	String raw;
	int indent;
	bool is_blank;
};

struct ToonParser {
	Vector<Line> lines;
	int cur;
	bool strict;
	int indentSize;
	bool expandPaths;

	bool IsEof() const { return cur >= lines.GetCount(); }
};

static int ScanHex(const String& hex) {
	int val = 0;
	for (int i = 0; i < hex.GetLength(); i++) {
		char c = hex[i];
		val *= 16;
		if (c >= '0' && c <= '9') val += c - '0';
		else if (c >= 'a' && c <= 'f') val += c - 'a' + 10;
		else if (c >= 'A' && c <= 'F') val += c - 'A' + 10;
	}
	return val;
}

static String ParseToonQuotedString(const char*& p, bool strict)
{
	if (*p != '"') {
		throw CParser::Error("Expected starting double quote");
	}
	p++; // consume '"'
	String s;
	while (*p && *p != '"') {
		if (*p == '\\') {
			p++; // consume '\\'
			if (!*p) {
				throw CParser::Error("Unterminated escape sequence");
			}
			char esc = *p;
			if (esc == '\\') { s.Cat('\\'); p++; }
			else if (esc == '"') { s.Cat('"'); p++; }
			else if (esc == 'n') { s.Cat('\n'); p++; }
			else if (esc == 'r') { s.Cat('\r'); p++; }
			else if (esc == 't') { s.Cat('\t'); p++; }
			else if (esc == 'u') {
				p++; // consume 'u'
				String hex;
				for(int i = 0; i < 4; i++) {
					if (!*p) throw CParser::Error("Truncated unicode escape");
					hex.Cat(*p++);
				}
				for(int i = 0; i < 4; i++) {
					char c = hex[i];
					if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
						throw CParser::Error("Invalid hex digit in unicode escape");
					}
				}
				int code = ScanHex(hex);
				if (code >= 0xD800 && code <= 0xDFFF) {
					throw CParser::Error("Surrogate pairs / lone surrogates are rejected in TOON");
				}
				s.Cat(WString(code, 1).ToString());
			}
			else {
				throw CParser::Error(String("Invalid escape sequence: \\") + esc);
			}
		} else {
			if ((byte)*p < 0x20) {
				if (*p != '\t') {
					throw CParser::Error("Raw control characters are forbidden in quoted strings");
				}
			}
			s.Cat(*p++);
		}
	}
	if (*p != '"') {
		throw CParser::Error("Unterminated quoted string");
	}
	p++; // consume '"'
	return s;
}

static String ParseToonKey(const char*& p, bool strict)
{
	if (*p == '"') {
		return ParseToonQuotedString(p, strict);
	}
	String key;
	if (!((unsigned char)*p == '_' || IsAlpha(*p))) {
		throw CParser::Error("Unquoted key must start with a letter or underscore");
	}
	while (*p && (IsAlNum(*p) || *p == '_' || *p == '.')) {
		key.Cat(*p++);
	}
	return key;
}

static bool ParseHeader(const String& line_content, String& key, int& length, int& active_delim, Vector<String>& fields, bool& is_tabular, bool strict)
{
	const char* p = line_content.Begin();
	if (!*p) return false;

	try {
		if (*p == '[') {
			key = "";
		} else {
			if (*p == '"' || (IsAlpha(*p) || *p == '_')) {
				key = ParseToonKey(p, strict);
			} else {
				return false;
			}
		}
		
		if (*p != '[') {
			return false;
		}
		p++;
		
		if (!IsDigit(*p)) {
			return false;
		}
		if (strict && *p == '0' && IsDigit(*(p + 1))) {
			throw CParser::Error("Leading zeros in array length are forbidden in strict mode");
		}
		String len_str;
		while (IsDigit(*p)) {
			len_str.Cat(*p++);
		}
		length = StrInt(len_str);
		
		active_delim = ',';
		if (*p == '\t') {
			active_delim = '\t';
			p++;
		} else if (*p == '|') {
			active_delim = '|';
			p++;
		}
		
		if (*p != ']') {
			return false;
		}
		p++;
		
		is_tabular = false;
		fields.Clear();
		if (*p == '{') {
			is_tabular = true;
			p++;
			while (*p && *p != '}') {
				String field = ParseToonKey(p, strict);
				fields.Add(field);
				if (*p == '}') {
					break;
				}
				if (*p != active_delim) {
					throw CParser::Error("Tabular fields must be separated by the active delimiter");
				}
				p++;
			}
			if (*p != '}') {
				return false;
			}
			p++;
		}
		
		if (*p != ':') {
			return false;
		}
		p++;
		
		if (*p) {
			if (strict && *p != ' ') {
				throw CParser::Error("Exactly one space must follow array header before inline values in strict mode");
			}
		}
		
		return true;
	}
	catch (CParser::Error&) {
		if (strict) throw;
		return false;
	}
}

static Vector<ToonToken> SplitToonLine(const String& line_content, int delim, bool strict)
{
	Vector<ToonToken> tokens;
	const char* p = line_content.Begin();
	while (true) {
		while (*p == ' ') {
			p++;
		}
		
		ToonToken tok;
		if (*p == '"') {
			tok.value = ParseToonQuotedString(p, strict);
			tok.is_quoted = true;
			while (*p == ' ') {
				p++;
			}
		} else {
			tok.is_quoted = false;
			const char* start = p;
			const char* last_non_space = p;
			while (*p && *p != delim) {
				if (*p != ' ') {
					last_non_space = p + 1;
				}
				p++;
			}
			if (last_non_space > start) {
				tok.value = String(start, (int)(last_non_space - start));
			} else {
				tok.value = "";
			}
		}
		
		tokens.Add(tok);
		
		if (!*p) {
			break;
		}
		if (*p == delim) {
			p++;
			if (!*p) {
				ToonToken empty_tok;
				empty_tok.value = "";
				empty_tok.is_quoted = false;
				tokens.Add(empty_tok);
				break;
			}
		} else {
			if (strict) {
				throw CParser::Error("Unexpected characters in inline array/row");
			}
			p++;
		}
	}
	return tokens;
}

static Value DecodePrimitive(const String& token, bool is_quoted, bool strict)
{
	if (is_quoted) {
		return token;
	}
	
	if (token == "null") return Null;
	if (token == "true") return true;
	if (token == "false") return false;
	if (token == "[]") return ValueArray();
	
	bool is_number = false;
	const char* s = token.Begin();
	if (*s == '-') s++;
	
	const char* integer_start = s;
	while (*s && IsDigit(*s)) {
		s++;
	}
	const char* integer_end = s;
	int integer_len = (int)(integer_end - integer_start);
	
	if (integer_len > 0) {
		bool has_leading_zero = false;
		if (integer_len > 1 && *integer_start == '0') {
			has_leading_zero = true;
		}
		
		if (*s == '.') {
			s++;
			const char* frac_start = s;
			while (*s && IsDigit(*s)) {
				s++;
			}
			if (s == frac_start) {
				integer_len = 0;
			}
		}
		
		if (integer_len > 0 && (*s == 'e' || *s == 'E')) {
			s++;
			if (*s == '+' || *s == '-') {
				s++;
			}
			const char* exp_start = s;
			while (*s && IsDigit(*s)) {
				s++;
			}
			if (s == exp_start) {
				integer_len = 0;
			}
		}
		
		if (integer_len > 0 && *s == '\0') {
			if (has_leading_zero) {
				return token;
			} else {
				double val = ScanDouble(token);
				if (val == 0.0) return 0.0;
				return val;
			}
		}
	}
	
	return token;
}

static bool IsTabularRow(const String& line_content, int active_delim)
{
	int first_delim = -1;
	int first_colon = -1;
	
	const char* p = line_content.Begin();
	while (*p) {
		if (*p == '"') {
			try {
				p++;
				while (*p && *p != '"') {
					if (*p == '\\') {
						p++;
						if (*p) p++;
					} else {
						p++;
					}
				}
				if (*p == '"') p++;
			} catch(...) {
				break;
			}
		} else {
			if (*p == active_delim && first_delim < 0) {
				first_delim = (int)(p - line_content.Begin());
			}
			if (*p == ':' && first_colon < 0) {
				first_colon = (int)(p - line_content.Begin());
			}
			p++;
		}
	}
	
	if (first_colon < 0) {
		return true;
	}
	if (first_delim >= 0 && first_delim < first_colon) {
		return true;
	}
	return false;
}

static Value ParseValueAtDepth(ToonParser& parser, int depth, const String& first_line_override);
static void ParseObjectField(ToonParser& parser, int depth, ValueMap& vm, const String& first_line_override = "");

static Value ParseTabularArray(ToonParser& parser, int length, int active_delim, const Vector<String>& fields, int row_depth)
{
	ValueArray va;
	for (int i = 0; i < length; i++) {
		if (parser.IsEof()) {
			if (parser.strict) {
				throw CParser::Error("Unexpected EOF in tabular array");
			}
			break;
		}
		while (!parser.IsEof() && parser.lines[parser.cur].is_blank) {
			if (parser.strict) {
				throw CParser::Error("Blank lines inside tabular rows are forbidden in strict mode");
			}
			parser.cur++;
		}
		if (parser.IsEof()) break;
		
		const Line& line = parser.lines[parser.cur];
		int actual_depth = line.indent / parser.indentSize;
		if (actual_depth < row_depth) {
			if (parser.strict) {
				throw CParser::Error("Tabular array has fewer rows than declared");
			}
			break;
		}
		if (actual_depth > row_depth) {
			if (parser.strict) {
				throw CParser::Error("Unexpected indentation increase in tabular row");
			}
		}
		
		Vector<ToonToken> tokens = SplitToonLine(line.raw.Mid(line.indent), active_delim, parser.strict);
		if (parser.strict && tokens.GetCount() != fields.GetCount()) {
			throw CParser::Error("Tabular row width mismatch");
		}
		
		ValueMap row_map;
		for (int f = 0; f < fields.GetCount(); f++) {
			Value val;
			if (f < tokens.GetCount()) {
				val = DecodePrimitive(tokens[f].value, tokens[f].is_quoted, parser.strict);
			} else {
				val = "";
			}
			row_map.Add(fields[f], val);
		}
		va.Add(row_map);
		parser.cur++;
	}
	
	if (parser.strict && !parser.IsEof()) {
		int next_idx = parser.cur;
		while (next_idx < parser.lines.GetCount() && parser.lines[next_idx].is_blank) {
			next_idx++;
		}
		if (next_idx < parser.lines.GetCount()) {
			const Line& next_line = parser.lines[next_idx];
			int next_depth = next_line.indent / parser.indentSize;
			if (next_depth == row_depth && IsTabularRow(next_line.raw.Mid(next_line.indent), active_delim)) {
				throw CParser::Error("Tabular array has more rows than declared N");
			}
		}
	}
	
	return va;
}

static Value ParseExpandedArray(ToonParser& parser, int length, int active_delim, int item_depth)
{
	ValueArray va;
	for (int i = 0; i < length; i++) {
		if (parser.IsEof()) {
			if (parser.strict) {
				throw CParser::Error("Unexpected EOF in expanded array");
			}
			break;
		}
		while (!parser.IsEof() && parser.lines[parser.cur].is_blank) {
			if (parser.strict) {
				throw CParser::Error("Blank lines inside expanded array are forbidden in strict mode");
			}
			parser.cur++;
		}
		if (parser.IsEof()) break;
		
		const Line& line = parser.lines[parser.cur];
		int actual_depth = line.indent / parser.indentSize;
		if (actual_depth < item_depth) {
			if (parser.strict) {
				throw CParser::Error("Expanded array has fewer items than declared N");
			}
			break;
		}
		if (actual_depth > item_depth) {
			if (parser.strict) {
				throw CParser::Error("Unexpected indentation increase in expanded array item");
			}
		}
		
		String raw_content = line.raw.Mid(line.indent);
		if (raw_content != "-" && !raw_content.StartsWith("- ")) {
			throw CParser::Error("Expanded array item must start with '-'");
		}
		
		Value item_value;
		if (raw_content == "-") {
			item_value = ValueMap();
			parser.cur++;
		} else {
			String rest = raw_content.Mid(2);
			item_value = ParseValueAtDepth(parser, item_depth + 1, rest);
		}
		va.Add(item_value);
	}
	
	if (parser.strict && !parser.IsEof()) {
		int next_idx = parser.cur;
		while (next_idx < parser.lines.GetCount() && parser.lines[next_idx].is_blank) {
			next_idx++;
		}
		if (next_idx < parser.lines.GetCount()) {
			const Line& next_line = parser.lines[next_idx];
			int next_depth = next_line.indent / parser.indentSize;
			if (next_depth == item_depth && (next_line.raw.Mid(next_line.indent) == "-" || next_line.raw.Mid(next_line.indent).StartsWith("- "))) {
				throw CParser::Error("Expanded array has more items than declared N");
			}
		}
	}
	
	return va;
}

static Value ParseValueAtDepth(ToonParser& parser, int depth, const String& first_line_override)
{
	String line_content;
	if (!first_line_override.IsEmpty()) {
		line_content = first_line_override;
	} else {
		if (parser.IsEof()) return Null;
		while (!parser.IsEof() && parser.lines[parser.cur].is_blank) {
			parser.cur++;
		}
		if (parser.IsEof()) return Null;
		
		const Line& line = parser.lines[parser.cur];
		int actual_depth = line.indent / parser.indentSize;
		if (actual_depth != depth) {
			return Null;
		}
		line_content = line.raw.Mid(line.indent);
	}
	
	int colon_idx = -1;
	const char* p = line_content.Begin();
	try {
		if (*p == '"' || (IsAlpha(*p) || *p == '_')) {
			ParseToonKey(p, parser.strict);
			if (*p == ':') {
				colon_idx = (int)(p - line_content.Begin());
			}
		}
	} catch (...) {}
	
	if (colon_idx >= 0) {
		ValueMap vm;
		ParseObjectField(parser, depth, vm, first_line_override);
		
		while (!parser.IsEof()) {
			while (!parser.IsEof() && parser.lines[parser.cur].is_blank) {
				if (parser.strict) {
					throw CParser::Error("Blank lines inside object are forbidden in strict mode");
				}
				parser.cur++;
			}
			if (parser.IsEof()) break;
			
			const Line& next_line = parser.lines[parser.cur];
			int next_depth = next_line.indent / parser.indentSize;
			if (next_depth < depth) {
				break;
			}
			if (next_depth > depth) {
				if (parser.strict) {
					throw CParser::Error("Unexpected indentation increase in object");
				}
				break;
			}
			
			String next_raw = next_line.raw.Mid(next_line.indent);
			if (next_raw.StartsWith("-")) {
				break;
			}
			
			ParseObjectField(parser, depth, vm);
		}
		return vm;
	}
	
	parser.cur++;
	
	String key;
	int length = 0;
	int active_delim = ',';
	Vector<String> fields;
	bool is_tabular = false;
	
	bool is_header = ParseHeader(line_content, key, length, active_delim, fields, is_tabular, parser.strict);
	
	if (is_header) {
		if (!key.IsEmpty()) {
			ValueMap vm;
			Value array_val;
			if (is_tabular) {
				array_val = ParseTabularArray(parser, length, active_delim, fields, depth + 1);
			} else {
				int col_idx = -1;
				const char* p_hdr = line_content.Begin();
				while (*p_hdr) {
					if (*p_hdr == '"') {
						try {
							p_hdr++;
							while (*p_hdr && *p_hdr != '"') {
								if (*p_hdr == '\\') { p_hdr++; if (*p_hdr) p_hdr++; }
								else p_hdr++;
							}
							if (*p_hdr == '"') p_hdr++;
						} catch(...) { break; }
					} else {
						if (*p_hdr == ':') {
							col_idx = (int)(p_hdr - line_content.Begin());
							break;
						}
						p_hdr++;
					}
				}
				String inline_content;
				if (col_idx >= 0 && col_idx + 1 < line_content.GetLength()) {
					inline_content = line_content.Mid(col_idx + 1);
					if (inline_content.StartsWith(" ")) {
						inline_content = inline_content.Mid(1);
					}
				}
				
				if (!inline_content.IsEmpty()) {
					Vector<ToonToken> tokens = SplitToonLine(inline_content, active_delim, parser.strict);
					if (parser.strict && tokens.GetCount() != length) {
						throw CParser::Error("Inline array value count mismatch");
					}
					ValueArray va;
					for (int i = 0; i < tokens.GetCount(); i++) {
						va.Add(DecodePrimitive(tokens[i].value, tokens[i].is_quoted, parser.strict));
					}
					array_val = va;
				} else {
					if (length == 0) {
						array_val = ValueArray();
					} else {
						array_val = ParseExpandedArray(parser, length, active_delim, depth + 1);
					}
				}
			}
			
			vm.Add(key, array_val);
			
			while (!parser.IsEof()) {
				while (!parser.IsEof() && parser.lines[parser.cur].is_blank) {
					if (parser.strict) {
						throw CParser::Error("Blank lines inside object are forbidden in strict mode");
					}
					parser.cur++;
				}
				if (parser.IsEof()) break;
				
				const Line& next_line = parser.lines[parser.cur];
				int next_depth = next_line.indent / parser.indentSize;
				if (next_depth < depth) {
					break;
				}
				if (next_depth > depth) {
					if (parser.strict) {
						throw CParser::Error("Unexpected indentation increase in object");
					}
					break;
				}
				
				String next_raw = next_line.raw.Mid(next_line.indent);
				if (next_raw.StartsWith("-")) {
					break;
				}
				
				ParseObjectField(parser, depth, vm);
			}
			return vm;
		} else {
			if (is_tabular) {
				return ParseTabularArray(parser, length, active_delim, fields, depth + 1);
			} else {
				int col_idx = -1;
				const char* p_hdr = line_content.Begin();
				while (*p_hdr) {
					if (*p_hdr == '"') {
						try {
							p_hdr++;
							while (*p_hdr && *p_hdr != '"') {
								if (*p_hdr == '\\') { p_hdr++; if (*p_hdr) p_hdr++; }
								else p_hdr++;
							}
							if (*p_hdr == '"') p_hdr++;
						} catch(...) { break; }
					} else {
						if (*p_hdr == ':') {
							col_idx = (int)(p_hdr - line_content.Begin());
							break;
						}
						p_hdr++;
					}
				}
				String inline_content;
				if (col_idx >= 0 && col_idx + 1 < line_content.GetLength()) {
					inline_content = line_content.Mid(col_idx + 1);
					if (inline_content.StartsWith(" ")) {
						inline_content = inline_content.Mid(1);
					}
				}
				
				if (!inline_content.IsEmpty()) {
					Vector<ToonToken> tokens = SplitToonLine(inline_content, active_delim, parser.strict);
					if (parser.strict && tokens.GetCount() != length) {
						throw CParser::Error("Inline array value count mismatch");
					}
					ValueArray va;
					for (int i = 0; i < tokens.GetCount(); i++) {
						va.Add(DecodePrimitive(tokens[i].value, tokens[i].is_quoted, parser.strict));
					}
					return va;
				} else {
					if (length == 0) {
						return ValueArray();
					} else {
						return ParseExpandedArray(parser, length, active_delim, depth + 1);
					}
				}
			}
		}
	}
	
	if (line_content.StartsWith("\"") && line_content.EndsWith("\"")) {
		const char* p_quoted = line_content.Begin();
		return ParseToonQuotedString(p_quoted, parser.strict);
	}
	return DecodePrimitive(line_content, false, parser.strict);
}

static void ParseObjectField(ToonParser& parser, int depth, ValueMap& vm, const String& first_line_override)
{
	String line_content;
	if (!first_line_override.IsEmpty()) {
		line_content = first_line_override;
	} else {
		if (parser.IsEof()) return;
		const Line& line = parser.lines[parser.cur];
		line_content = line.raw.Mid(line.indent);
	}
	parser.cur++;
	
	String key;
	int length = 0;
	int active_delim = ',';
	Vector<String> fields;
	bool is_tabular = false;
	
	bool is_header = ParseHeader(line_content, key, length, active_delim, fields, is_tabular, parser.strict);
	
	if (is_header) {
		if (parser.strict && !vm[key].IsVoid()) {
			throw CParser::Error("Duplicate sibling key: " + key);
		}
		
		Value array_val;
		if (is_tabular) {
			array_val = ParseTabularArray(parser, length, active_delim, fields, depth + 1);
		} else {
			int colon_idx = -1;
			const char* p = line_content.Begin();
			while (*p) {
				if (*p == '"') {
					try {
						p++;
						while (*p && *p != '"') {
							if (*p == '\\') { p++; if (*p) p++; }
							else p++;
						}
						if (*p == '"') p++;
					} catch(...) { break; }
				} else {
					if (*p == ':') {
						colon_idx = (int)(p - line_content.Begin());
						break;
					}
					p++;
				}
			}
			String inline_content;
			if (colon_idx >= 0 && colon_idx + 1 < line_content.GetLength()) {
				inline_content = line_content.Mid(colon_idx + 1);
				if (inline_content.StartsWith(" ")) {
					inline_content = inline_content.Mid(1);
				}
			}
			
			if (!inline_content.IsEmpty()) {
				Vector<ToonToken> tokens = SplitToonLine(inline_content, active_delim, parser.strict);
				if (parser.strict && tokens.GetCount() != length) {
					throw CParser::Error("Inline array value count mismatch");
				}
				ValueArray va;
				for (int i = 0; i < tokens.GetCount(); i++) {
					va.Add(DecodePrimitive(tokens[i].value, tokens[i].is_quoted, parser.strict));
				}
				array_val = va;
			} else {
				if (length == 0) {
					array_val = ValueArray();
				} else {
					array_val = ParseExpandedArray(parser, length, active_delim, depth + 1);
				}
			}
		}
		
		vm.Set(key, array_val);
	} else {
		const char* p = line_content.Begin();
		key = ParseToonKey(p, parser.strict);
		if (*p != ':') {
			throw CParser::Error("Expected ':' after key");
		}
		p++;
		
		if (parser.strict && !vm[key].IsVoid()) {
			throw CParser::Error("Duplicate sibling key: " + key);
		}
		
		if (*p == ' ') {
			p++;
		}
		
		if (*p) {
			String val_str(p);
			Value val;
			if (val_str.StartsWith("\"") && val_str.EndsWith("\"")) {
				const char* p_quoted = val_str.Begin();
				val = ParseToonQuotedString(p_quoted, parser.strict);
			} else {
				val = DecodePrimitive(val_str, false, parser.strict);
			}
			vm.Set(key, val);
		} else {
			int child_depth = depth + 1;
			while (!parser.IsEof() && parser.lines[parser.cur].is_blank) {
				if (parser.strict) {
					throw CParser::Error("Blank lines inside object are forbidden in strict mode");
				}
				parser.cur++;
			}
			
			bool has_children = false;
			if (!parser.IsEof()) {
				const Line& next_line = parser.lines[parser.cur];
				int next_depth = next_line.indent / parser.indentSize;
				if (next_depth >= child_depth) {
					has_children = true;
				}
			}
			
			if (has_children) {
				vm.Set(key, ParseValueAtDepth(parser, child_depth, ""));
			} else {
				vm.Set(key, ValueMap());
			}
		}
	}
}

static void MergeValue(ValueMap& dest, const String& segment, const Value& src, bool strict)
{
	Value existing = dest[segment];
	if (existing.IsVoid()) {
		dest.Add(segment, src);
		return;
	}
	
	if (existing.GetType() == VALUEMAP_V && src.GetType() == VALUEMAP_V) {
		ValueMap dest_sub = existing;
		ValueMap src_sub = src;
		for (int i = 0; i < src_sub.GetCount(); i++) {
			MergeValue(dest_sub, src_sub.GetKey(i), src_sub.GetValue(i), strict);
		}
		dest.Set(segment, dest_sub);
	} else {
		if (strict) {
			throw CParser::Error("Expansion conflict at segment: " + segment);
		} else {
			dest.Set(segment, src);
		}
	}
}

static Value ExpandPaths(const Value& v, bool strict)
{
	if (v.GetType() == VALUEMAP_V) {
		ValueMap old_map = v;
		ValueMap new_map;
		for (int i = 0; i < old_map.GetCount(); i++) {
			String key = old_map.GetKey(i);
			Value val = ExpandPaths(old_map.GetValue(i), strict);
			
			bool eligible = false;
			if (key.Find('.') >= 0) {
				const char* p = key.Begin();
				if (*p == '_' || IsAlpha(*p)) {
					eligible = true;
					while (*p) {
						if (!(IsAlNum(*p) || *p == '_' || *p == '.')) {
							eligible = false;
							break;
						}
						p++;
					}
				}
				if (eligible) {
					Vector<String> segments = Split(key, '.');
					for (int s = 0; s < segments.GetCount(); s++) {
						const String& seg = segments[s];
						if (seg.IsEmpty() || (!IsAlpha(seg[0]) && seg[0] != '_')) {
							eligible = false;
							break;
						}
						for (int ch = 1; ch < seg.GetLength(); ch++) {
							if (!IsAlNum(seg[ch]) && seg[ch] != '_') {
								eligible = false;
								break;
							}
						}
					}
				}
			}
			
			if (eligible) {
				Vector<String> segments = Split(key, '.');
				Value current_val = val;
				for (int s = segments.GetCount() - 1; s >= 1; s--) {
					ValueMap parent_map;
					parent_map.Add(segments[s], current_val);
					current_val = parent_map;
				}
				MergeValue(new_map, segments[0], current_val, strict);
			} else {
				MergeValue(new_map, key, val, strict);
			}
		}
		return new_map;
	} else if (v.GetType() == VALUEARRAY_V) {
		ValueArray old_arr = v;
		ValueArray new_arr;
		for (int i = 0; i < old_arr.GetCount(); i++) {
			new_arr.Add(ExpandPaths(old_arr[i], strict));
		}
		return new_arr;
	}
	return v;
}

Value ParseTOON(const char *s, bool strict, int indentSize, bool expandPaths)
{
	Vector<Line> lines;
	const char* ptr = s;
	int line_index = 0;
	while (*ptr) {
		String raw;
		while (*ptr && *ptr != '\n' && *ptr != '\r') {
			raw.Cat(*ptr++);
		}
		if (*ptr == '\r') ptr++;
		if (*ptr == '\n') ptr++;
		
		Line line;
		line.line_index = line_index++;
		line.raw = raw;
		
		int indent = 0;
		bool has_invalid_indent = false;
		for (int i = 0; i < raw.GetLength(); i++) {
			char c = raw[i];
			if (c == ' ') {
				indent++;
			} else if (c == '\t') {
				if (strict) {
					has_invalid_indent = true;
				} else {
					indent += indentSize;
				}
			} else {
				break;
			}
		}
		
		if (strict && has_invalid_indent) {
			return ErrorValue("Tabs used as indentation are forbidden in strict mode");
		}
		if (strict && indent % indentSize != 0) {
			return ErrorValue("Indentation must be an exact multiple of indentSize in strict mode");
		}
		
		line.indent = indent;
		String trimmed = TrimBoth(raw);
		line.is_blank = trimmed.IsEmpty();
		
		lines.Add(line);
	}
	
	Vector<int> non_blank_indices;
	for (int i = 0; i < lines.GetCount(); i++) {
		if (!lines[i].is_blank) {
			non_blank_indices.Add(i);
		}
	}
	
	if (non_blank_indices.IsEmpty()) {
		return ValueMap();
	}
	
	if (non_blank_indices.GetCount() == 1) {
		const Line& single_line = lines[non_blank_indices[0]];
		String content = TrimBoth(single_line.raw);
		
		if (content == "[]") {
			return ValueArray();
		}
		
		String key;
		int length = 0;
		int active_delim = ',';
		Vector<String> fields;
		bool is_tabular = false;
		bool is_header = ParseHeader(content, key, length, active_delim, fields, is_tabular, strict);
		
		if (is_header && key.IsEmpty()) {
			// Parse as root array header
		} else if (!is_header && content.Find(':') < 0) {
			try {
				if (content.StartsWith("\"") && content.EndsWith("\"")) {
					const char* p_quoted = content.Begin();
					return ParseToonQuotedString(p_quoted, strict);
				}
				return DecodePrimitive(content, false, strict);
			}
			catch (CParser::Error e) {
				return ErrorValue(e);
			}
		}
	}
	
	ToonParser parser;
	parser.lines = pick(lines);
	parser.cur = 0;
	parser.strict = strict;
	parser.indentSize = indentSize;
	parser.expandPaths = expandPaths;
	
	try {
		while (!parser.IsEof() && parser.lines[parser.cur].is_blank) {
			parser.cur++;
		}
		if (parser.IsEof()) {
			return ValueMap();
		}
		
		const Line& first_line = parser.lines[parser.cur];
		int first_depth = first_line.indent / parser.indentSize;
		if (first_depth != 0) {
			throw CParser::Error("First non-empty line must be at depth 0");
		}
		
		String first_content = first_line.raw.Mid(first_line.indent);
		String key;
		int length = 0;
		int active_delim = ',';
		Vector<String> fields;
		bool is_tabular = false;
		bool is_header = ParseHeader(first_content, key, length, active_delim, fields, is_tabular, strict);
		
		Value result;
		if (is_header && key.IsEmpty()) {
			result = ParseValueAtDepth(parser, 0, "");
		} else {
			ValueMap vm;
			while (!parser.IsEof()) {
				while (!parser.IsEof() && parser.lines[parser.cur].is_blank) {
					parser.cur++;
				}
				if (parser.IsEof()) break;
				
				const Line& line = parser.lines[parser.cur];
				int depth = line.indent / parser.indentSize;
				if (depth != 0) {
					throw CParser::Error("Unexpected indentation at root level");
				}
				
				ParseObjectField(parser, 0, vm);
			}
			result = vm;
		}
		
		while (!parser.IsEof()) {
			if (!parser.lines[parser.cur].is_blank && strict) {
				throw CParser::Error("Extra content at the end of the document");
			}
			parser.cur++;
		}
		
		if (expandPaths) {
			result = ExpandPaths(result, strict);
		}
		
		return result;
	}
	catch (CParser::Error e) {
		return ErrorValue(e);
	}
}

static bool NeedsQuoting(const String& s, int delimiter)
{
	if (s.IsEmpty()) return true;
	if (s[0] == ' ' || s[s.GetLength() - 1] == ' ') return true;
	if (s == "true" || s == "false" || s == "null") return true;
	if (s[0] == '-') return true;
	
	bool is_numeric = false;
	const char* p = s.Begin();
	if (*p == '-') p++;
	const char* int_start = p;
	while (*p && IsDigit(*p)) p++;
	if (p > int_start) {
		is_numeric = true;
		if (*p == '.') {
			p++;
			const char* frac_start = p;
			while (*p && IsDigit(*p)) p++;
			if (p == frac_start) is_numeric = false;
		}
		if (is_numeric && (*p == 'e' || *p == 'E')) {
			p++;
			if (*p == '+' || *p == '-') p++;
			const char* exp_start = p;
			while (*p && IsDigit(*p)) p++;
			if (p == exp_start) is_numeric = false;
		}
		if (is_numeric && *p != '\0') is_numeric = false;
	}
	if (is_numeric) return true;
	
	for (int i = 0; i < s.GetLength(); i++) {
		char c = s[i];
		if (c == ':' || c == '"' || c == '\\') return true;
		if (c == '[' || c == ']' || c == '{' || c == '}') return true;
		if ((byte)c < 0x20) return true;
		if (c == delimiter) return true;
	}
	
	return false;
}

static String ToonEscapeString(const String& s)
{
	String res;
	const char* p = s.Begin();
	while (*p) {
		char c = *p;
		if (c == '\\') { res << "\\\\"; p++; }
		else if (c == '"') { res << "\\\""; p++; }
		else if (c == '\n') { res << "\\n"; p++; }
		else if (c == '\r') { res << "\\r"; p++; }
		else if (c == '\t') { res << "\\t"; p++; }
		else if ((byte)c < 0x20) {
			res << Format("\\u%04x", (int)(byte)c);
			p++;
		} else {
			res.Cat(*p++);
		}
	}
	return res;
}

static String FormatToonString(const String& s, int delimiter)
{
	if (NeedsQuoting(s, delimiter)) {
		return "\"" + ToonEscapeString(s) + "\"";
	}
	return s;
}

static String FormatToonKey(const String& key)
{
	bool needs_quote = false;
	if (key.IsEmpty()) {
		needs_quote = true;
	} else {
		if (!((unsigned char)key[0] == '_' || IsAlpha(key[0]))) {
			needs_quote = true;
		} else {
			for (int i = 0; i < key.GetLength(); i++) {
				char c = key[i];
				if (!(IsAlNum(c) || c == '_' || c == '.')) {
					needs_quote = true;
					break;
				}
			}
		}
	}
	if (needs_quote) {
		return "\"" + ToonEscapeString(key) + "\"";
	}
	return key;
}

static String DelimSymbol(int delim)
{
	if (delim == '\t') return "\t";
	if (delim == '|') return "|";
	return "";
}

static bool IsTabularArray(const ValueArray& va, Index<String>& fields)
{
	if (va.GetCount() == 0) return false;
	fields.Clear();
	
	Value first = va[0];
	if (first.GetType() != VALUEMAP_V) return false;
	ValueMap first_map = first;
	if (first_map.GetCount() == 0) return false;
	
	for (int i = 0; i < first_map.GetCount(); i++) {
		String key = first_map.GetKey(i);
		fields.Add(key);
	}
	
	for (int i = 0; i < va.GetCount(); i++) {
		Value elem = va[i];
		if (elem.GetType() != VALUEMAP_V) return false;
		ValueMap elem_map = elem;
		if (elem_map.GetCount() != fields.GetCount()) return false;
		
		for (int f = 0; f < fields.GetCount(); f++) {
			Value val = elem_map[fields[f]];
			if (val.IsVoid()) return false;
			if (val.GetType() == VALUEMAP_V || val.GetType() == VALUEARRAY_V) return false;
		}
	}
	return true;
}

static String FormatToonNumber(double n)
{
	if (n == 0.0) {
		return "0";
	}
	double abs_n = abs(n);
	if (abs_n >= 1e-6 && abs_n < 1e21) {
		String s = FormatDouble(n, 16);
		if (s.Find('.') >= 0) {
			while (s.EndsWith("0")) {
				s = s.Mid(0, s.GetLength() - 1);
			}
			if (s.EndsWith(".")) {
				s = s.Mid(0, s.GetLength() - 1);
			}
		}
		return s;
	} else {
		String s = Format("%.15e", n);
		s = ToLower(s);
		return s;
	}
}

static String FormatToonValueString(const Value& v, int delimiter)
{
	if (v.GetType() == INT_V) return FormatInt((int)v);
	if (v.GetType() == INT64_V) return FormatInt64((int64)v);
	if (v.GetType() == DOUBLE_V) return FormatToonNumber((double)v);
	if (v.GetType() == BOOL_V) return (bool)v ? "true" : "false";
	if (IsNull(v)) return "null";
	return FormatToonString(AsString(v), delimiter);
}

static void AddLine(String& out, int indent, const String& content)
{
	if (!out.IsEmpty()) {
		out << "\n";
	}
	out << String(' ', indent) << content;
}

static void EncodeValue(const Value& v, const String& key, int indent, int indentSize, int delimiter, String& out)
{
	String key_prefix;
	if (!key.IsEmpty()) {
		key_prefix = FormatToonKey(key);
	}
	
	if (v.GetType() == VALUEMAP_V) {
		ValueMap vm = v;
		if (vm.GetCount() == 0) {
			if (!key.IsEmpty()) {
				AddLine(out, indent, key_prefix + ":");
			}
			return;
		}
		
		if (!key.IsEmpty()) {
			AddLine(out, indent, key_prefix + ":");
			for (int i = 0; i < vm.GetCount(); i++) {
				EncodeValue(vm.GetValue(i), vm.GetKey(i), indent + indentSize, indentSize, delimiter, out);
			}
		} else {
			for (int i = 0; i < vm.GetCount(); i++) {
				EncodeValue(vm.GetValue(i), vm.GetKey(i), indent, indentSize, delimiter, out);
			}
		}
	}
	else if (v.GetType() == VALUEARRAY_V) {
		ValueArray va = v;
		if (va.GetCount() == 0) {
			if (!key.IsEmpty()) {
				AddLine(out, indent, key_prefix + ": []");
			} else {
				AddLine(out, indent, "[]");
			}
			return;
		}
		
		Index<String> fields;
		if (IsTabularArray(va, fields)) {
			String header = key_prefix + "[" + AsString(va.GetCount()) + DelimSymbol(delimiter) + "]{";
			for (int f = 0; f < fields.GetCount(); f++) {
				if (f > 0) header << String(delimiter, 1);
				header << FormatToonKey(fields[f]);
			}
			header << "}:";
			AddLine(out, indent, header);
			
			for (int i = 0; i < va.GetCount(); i++) {
				ValueMap row = va[i];
				String row_str;
				for (int f = 0; f < fields.GetCount(); f++) {
					if (f > 0) row_str << String(delimiter, 1);
					row_str << FormatToonValueString(row[fields[f]], delimiter);
				}
				AddLine(out, indent + indentSize, row_str);
			}
		}
		else {
			bool all_primitives = true;
			for (int i = 0; i < va.GetCount(); i++) {
				if (va[i].GetType() == VALUEMAP_V || va[i].GetType() == VALUEARRAY_V) {
					all_primitives = false;
					break;
				}
			}
			
			if (all_primitives) {
				String inline_str = key_prefix + "[" + AsString(va.GetCount()) + DelimSymbol(delimiter) + "]: ";
				for (int i = 0; i < va.GetCount(); i++) {
					if (i > 0) inline_str << String(delimiter, 1);
					inline_str << FormatToonValueString(va[i], delimiter);
				}
				AddLine(out, indent, inline_str);
			}
			else {
				String header = key_prefix + "[" + AsString(va.GetCount()) + DelimSymbol(delimiter) + "]:";
				AddLine(out, indent, header);
				
				for (int i = 0; i < va.GetCount(); i++) {
					Value item = va[i];
					if (item.GetType() == VALUEMAP_V) {
						ValueMap item_map = item;
						if (item_map.GetCount() == 0) {
							AddLine(out, indent + indentSize, "-");
						} else {
							Index<String> sub_fields;
							bool first_is_tabular = false;
							String first_key = item_map.GetKey(0);
							Value first_val = item_map.GetValue(0);
							if (first_val.GetType() == VALUEARRAY_V && IsTabularArray(first_val, sub_fields)) {
								first_is_tabular = true;
							}
							
							if (first_is_tabular) {
								String first_key_prefix = FormatToonKey(first_key);
								ValueArray sub_va = first_val;
								String sub_header = "- " + first_key_prefix + "[" + AsString(sub_va.GetCount()) + DelimSymbol(delimiter) + "]{";
								for (int f = 0; f < sub_fields.GetCount(); f++) {
									if (f > 0) sub_header << String(delimiter, 1);
									sub_header << FormatToonKey(sub_fields[f]);
								}
								sub_header << "}:";
								AddLine(out, indent + indentSize, sub_header);
								
								for (int r = 0; r < sub_va.GetCount(); r++) {
									ValueMap row = sub_va[r];
									String row_str;
									for (int f = 0; f < sub_fields.GetCount(); f++) {
										if (f > 0) row_str << String(delimiter, 1);
										row_str << FormatToonValueString(row[sub_fields[f]], delimiter);
									}
									AddLine(out, indent + 2 * indentSize, row_str);
								}
								
								for (int f = 1; f < item_map.GetCount(); f++) {
									EncodeValue(item_map.GetValue(f), item_map.GetKey(f), indent + 2 * indentSize, indentSize, delimiter, out);
								}
							} else {
								String temp_out;
								EncodeValue(first_val, first_key, indent + indentSize, indentSize, delimiter, temp_out);
								
								int first_line_spaces = indent + indentSize;
								String prefix(' ', first_line_spaces);
								if (temp_out.StartsWith(prefix)) {
									temp_out = temp_out.Mid(first_line_spaces);
								}
								out << "\n" << String(' ', indent + indentSize) << "- " << temp_out;
								
								for (int f = 1; f < item_map.GetCount(); f++) {
									EncodeValue(item_map.GetValue(f), item_map.GetKey(f), indent + 2 * indentSize, indentSize, delimiter, out);
								}
							}
						}
					} else if (item.GetType() == VALUEARRAY_V) {
						String temp_out;
						EncodeValue(item, "", indent + indentSize, indentSize, delimiter, temp_out);
						int first_line_spaces = indent + indentSize;
						String prefix(' ', first_line_spaces);
						if (temp_out.StartsWith(prefix)) {
							temp_out = temp_out.Mid(first_line_spaces);
						}
						out << "\n" << String(' ', indent + indentSize) << "- " << temp_out;
					} else {
						AddLine(out, indent + indentSize, "- " + FormatToonValueString(item, delimiter));
					}
				}
			}
		}
	}
	else {
		String val_str = FormatToonValueString(v, delimiter);
		if (!key.IsEmpty()) {
			AddLine(out, indent, key_prefix + ": " + val_str);
		} else {
			AddLine(out, indent, val_str);
		}
	}
}

static Value FoldPaths(const Value& v, bool keyFolding, int flattenDepth)
{
	if (!keyFolding) return v;
	
	if (v.GetType() == VALUEMAP_V) {
		ValueMap old_map = v;
		ValueMap new_map;
		for (int i = 0; i < old_map.GetCount(); i++) {
			String key = old_map.GetKey(i);
			Value val = old_map.GetValue(i);
			
			Vector<String> chain;
			chain.Add(key);
			Value current = val;
			int depth = 1;
			while (current.GetType() == VALUEMAP_V && depth < flattenDepth) {
				ValueMap sub_map = current;
				if (sub_map.GetCount() != 1) {
					break;
				}
				String sub_key = sub_map.GetKey(0);
				bool is_id = !sub_key.IsEmpty() && (sub_key[0] == '_' || IsAlpha(sub_key[0]));
				for (int ch = 1; ch < sub_key.GetLength(); ch++) {
					if (!IsAlNum(sub_key[ch]) && sub_key[ch] != '_') {
						is_id = false;
						break;
					}
				}
				if (!is_id) {
					break;
				}
				chain.Add(sub_key);
				current = sub_map.GetValue(0);
				depth++;
			}
			
			bool all_id = true;
			for (int s = 0; s < chain.GetCount(); s++) {
				const String& seg = chain[s];
				bool is_id = !seg.IsEmpty() && (seg[0] == '_' || IsAlpha(seg[0]));
				for (int ch = 1; ch < seg.GetLength(); ch++) {
					if (!IsAlNum(seg[ch]) && seg[ch] != '_') {
						is_id = false;
						break;
					}
				}
				if (!is_id) {
					all_id = false;
					break;
				}
			}
			
			if (all_id && chain.GetCount() > 1) {
				String folded_key = Join(chain, ".");
				if (new_map[folded_key].IsVoid()) {
					new_map.Add(folded_key, FoldPaths(current, keyFolding, flattenDepth));
				} else {
					new_map.Add(key, FoldPaths(val, keyFolding, flattenDepth));
				}
			} else {
				new_map.Add(key, FoldPaths(val, keyFolding, flattenDepth));
			}
		}
		return new_map;
	} else if (v.GetType() == VALUEARRAY_V) {
		ValueArray old_arr = v;
		ValueArray new_arr;
		for (int i = 0; i < old_arr.GetCount(); i++) {
			new_arr.Add(FoldPaths(old_arr[i], keyFolding, flattenDepth));
		}
		return new_arr;
	}
	return v;
}

String AsTOON(const Value& v, int indentSize, int delimiter, bool keyFolding, int flattenDepth)
{
	Value folded = FoldPaths(v, keyFolding, flattenDepth);
	String out;
	EncodeValue(folded, "", 0, indentSize, delimiter, out);
	return out;
}

}
