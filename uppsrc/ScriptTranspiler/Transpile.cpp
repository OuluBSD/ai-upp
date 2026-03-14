#include "ScriptTranspiler.h"

NAMESPACE_UPP

void PyToJsResult::Jsonize(JsonIO& json)
{
	json("ok", ok)
	    ("javascript", javascript)
	    ("warnings", warnings)
	    ("errors", errors);
}

static String Indent(int level)
{
	return String(' ', max(0, level) * 2);
}

static String TrimRightCopy(String s)
{
	while(!s.IsEmpty() && (s[s.GetCount() - 1] == '\r' || s[s.GetCount() - 1] == ' ' || s[s.GetCount() - 1] == '\t'))
		s.TrimLast();
	return s;
}

static String StripInlineComment(const String& s)
{
	bool in_single = false;
	bool in_double = false;
	for(int i = 0; i < s.GetCount(); i++) {
		int prev = i > 0 ? s[i - 1] : 0;
		if(s[i] == '\'' && !in_double && prev != '\\')
			in_single = !in_single;
		else if(s[i] == '"' && !in_single && prev != '\\')
			in_double = !in_double;
		else if(s[i] == '#' && !in_single && !in_double)
			return s.Left(i);
	}
	return s;
}

static int CountIndent(const String& line, Vector<String>& warnings, int lineno)
{
	int spaces = 0;
	for(int i = 0; i < line.GetCount(); i++) {
		if(line[i] == ' ')
			spaces++;
		else if(line[i] == '\t') {
			warnings.Add(Format("%s:%d: tab indentation converted as 4 spaces", "<source>", lineno));
			spaces += 4;
		}
		else
			break;
	}
	return spaces;
}

static String ReplaceWordBoundaries(String s, const char* from, const char* to)
{
	s.Replace(from, to);
	return s;
}

static Vector<String> SplitTopLevel(const String& s, char ch)
{
	Vector<String> out;
	int start = 0;
	int depth_paren = 0;
	int depth_bracket = 0;
	int depth_brace = 0;
	bool in_single = false;
	bool in_double = false;
	for(int i = 0; i < s.GetCount(); i++) {
		int prev = i > 0 ? s[i - 1] : 0;
		if(s[i] == '\'' && !in_double && prev != '\\')
			in_single = !in_single;
		else if(s[i] == '"' && !in_single && prev != '\\')
			in_double = !in_double;
		else if(!in_single && !in_double) {
			if(s[i] == '(') depth_paren++;
			else if(s[i] == ')') depth_paren--;
			else if(s[i] == '[') depth_bracket++;
			else if(s[i] == ']') depth_bracket--;
			else if(s[i] == '{') depth_brace++;
			else if(s[i] == '}') depth_brace--;
			else if(s[i] == ch && depth_paren == 0 && depth_bracket == 0 && depth_brace == 0) {
				out.Add(TrimBoth(s.Mid(start, i - start)));
				start = i + 1;
			}
		}
	}
	out.Add(TrimBoth(s.Mid(start)));
	return out;
}

static bool IsWrappedTopLevel(const String& s, char open_ch, char close_ch)
{
	if(s.GetCount() < 2 || s[0] != open_ch || s[s.GetCount() - 1] != close_ch)
		return false;
	int depth = 0;
	bool in_single = false;
	bool in_double = false;
	for(int i = 0; i < s.GetCount(); i++) {
		int prev = i > 0 ? s[i - 1] : 0;
		if(s[i] == '\'' && !in_double && prev != '\\')
			in_single = !in_single;
		else if(s[i] == '"' && !in_single && prev != '\\')
			in_double = !in_double;
		else if(!in_single && !in_double) {
			if(s[i] == open_ch) depth++;
			else if(s[i] == close_ch) depth--;
			if(depth == 0 && i != s.GetCount() - 1)
				return false;
		}
	}
	return depth == 0;
}

static int GetOpenBracketDepth(const String& s)
{
	int depth_paren = 0;
	int depth_bracket = 0;
	int depth_brace = 0;
	bool in_single = false;
	bool in_double = false;
	for(int i = 0; i < s.GetCount(); i++) {
		int prev = i > 0 ? s[i - 1] : 0;
		if(s[i] == '\'' && !in_double && prev != '\\')
			in_single = !in_single;
		else if(s[i] == '"' && !in_single && prev != '\\')
			in_double = !in_double;
		else if(!in_single && !in_double) {
			if(s[i] == '#')
				break;
			if(s[i] == '(') depth_paren++;
			else if(s[i] == ')') depth_paren--;
			else if(s[i] == '[') depth_bracket++;
			else if(s[i] == ']') depth_bracket--;
			else if(s[i] == '{') depth_brace++;
			else if(s[i] == '}') depth_brace--;
		}
	}
	return depth_paren + depth_bracket + depth_brace;
}

static bool NeedsContinuation(const String& s)
{
	String code = TrimRightCopy(StripInlineComment(s));
	if(code.EndsWith("\\"))
		return true;
	return GetOpenBracketDepth(code) > 0;
}

static int FindTopLevelToken(const String& s, const char* token)
{
	int token_len = (int)strlen(token);
	int depth_paren = 0;
	int depth_bracket = 0;
	int depth_brace = 0;
	bool in_single = false;
	bool in_double = false;
	for(int i = 0; i + token_len <= s.GetCount(); i++) {
		int prev = i > 0 ? s[i - 1] : 0;
		if(s[i] == '\'' && !in_double && prev != '\\') {
			in_single = !in_single;
			continue;
		}
		if(s[i] == '"' && !in_single && prev != '\\') {
			in_double = !in_double;
			continue;
		}
		if(in_single || in_double)
			continue;
		if(s[i] == '(') depth_paren++;
		else if(s[i] == ')') depth_paren--;
		else if(s[i] == '[') depth_bracket++;
		else if(s[i] == ']') depth_bracket--;
		else if(s[i] == '{') depth_brace++;
		else if(s[i] == '}') depth_brace--;
		if(depth_paren == 0 && depth_bracket == 0 && depth_brace == 0 && s.Mid(i, token_len) == token)
			return i;
	}
	return -1;
}

static int FindTopLevelAugAssign(const String& s, String& op)
{
	static const char* ops[] = {"+=", "-=", "*=", "/=", "%="};
	for(const char* tok : ops) {
		int pos = FindTopLevelToken(s, tok);
		if(pos >= 0) {
			op = tok;
			return pos;
		}
	}
	return -1;
}

static String ConvertTupleLiteral(String expr)
{
	expr = TrimBoth(expr);
	if(IsWrappedTopLevel(expr, '(', ')')) {
		String inner = TrimBoth(expr.Mid(1, expr.GetCount() - 2));
		Vector<String> items = SplitTopLevel(inner, ',');
		if(items.GetCount() > 1) {
			String out = "[";
			for(int i = 0; i < items.GetCount(); i++) {
				if(i) out << ", ";
				out << items[i];
			}
			out << "]";
			return out;
		}
	}
	return expr;
}

static String JoinFrom(const Vector<String>& parts, int begin)
{
	String out;
	for(int i = begin; i < parts.GetCount(); i++) {
		if(i > begin)
			out << ", ";
		out << parts[i];
	}
	return out;
}

static bool IsIdentifier(const String& s);
static String ConvertEmbeddedConditional(String expr);
static String ConvertSimpleStatement(const String& stripped);

static String ConvertExpr(String expr)
{
	expr = TrimBoth(expr);
	expr = ConvertTupleLiteral(expr);
	expr = ConvertEmbeddedConditional(expr);
	int if_pos = FindTopLevelToken(expr, " if ");
	if(if_pos >= 0) {
		int else_pos = FindTopLevelToken(expr, " else ");
		if(else_pos > if_pos) {
			String when_true = TrimBoth(expr.Left(if_pos));
			String cond = TrimBoth(expr.Mid(if_pos + 4, else_pos - (if_pos + 4)));
			String when_false = TrimBoth(expr.Mid(else_pos + 6));
			expr = "(" + ConvertExpr(cond) + " ? " + ConvertExpr(when_true) + " : " + ConvertExpr(when_false) + ")";
			return expr;
		}
	}
	expr = ReplaceWordBoundaries(expr, " is not None", " !== null");
	expr = ReplaceWordBoundaries(expr, " is None", " === null");
	expr = ReplaceWordBoundaries(expr, " is not ", " !== ");
	expr = ReplaceWordBoundaries(expr, " is ", " === ");
	expr = ReplaceWordBoundaries(expr, " and ", " && ");
	expr = ReplaceWordBoundaries(expr, " or ", " || ");
	expr = ReplaceWordBoundaries(expr, "True", "true");
	expr = ReplaceWordBoundaries(expr, "False", "false");
	expr = ReplaceWordBoundaries(expr, "None", "null");
	expr = ReplaceWordBoundaries(expr, ".append(", ".push(");
	expr = ReplaceWordBoundaries(expr, "not ", "!");
	if(expr.StartsWith("print("))
		expr = "console.log" + expr.Mid(5);
	int in_pos = FindTopLevelToken(expr, " in ");
	if(in_pos >= 0) {
		String lhs = TrimBoth(expr.Left(in_pos));
		String rhs = TrimBoth(expr.Mid(in_pos + 4));
		expr = "__py_in__(" + lhs + ", " + rhs + ")";
	}
	return expr;
}

static String ConvertSimpleStatement(const String& stripped)
{
	String s = TrimBoth(stripped);
	if(s.IsEmpty())
		return String();
	if(s == "pass")
		return "/* pass */";
	if(s == "break" || s == "continue")
		return s + ";";
	if(s.StartsWith("return")) {
		String rest = TrimBoth(s.Mid(6));
		String out = "return";
		if(!rest.IsEmpty())
			out << " " << ConvertExpr(rest);
		out << ";";
		return out;
	}
	if(s.StartsWith("assert ")) {
		String rest = TrimBoth(s.Mid(7));
		Vector<String> parts = SplitTopLevel(rest, ',');
		if(parts.GetCount() == 1)
			return "__py_assert__(" + ConvertExpr(parts[0]) + ");";
		return "__py_assert__(" + ConvertExpr(parts[0]) + ", " + ConvertExpr(JoinFrom(parts, 1)) + ");";
	}
	if(s.StartsWith("global "))
		return "/* global " + TrimBoth(s.Mid(7)) + " */";
	if(s.EndsWith(".clear()"))
		return ConvertExpr(s.Left(s.GetCount() - 8)) + ".length = 0;";
	int remove_pos = s.Find(".remove(");
	if(remove_pos > 0 && s.EndsWith(")"))
		return "__py_remove__(" + ConvertExpr(s.Left(remove_pos)) + ", " + ConvertExpr(s.Mid(remove_pos + 8, s.GetCount() - remove_pos - 9)) + ");";

	String aug_op;
	int aug_pos = FindTopLevelAugAssign(s, aug_op);
	if(aug_pos > 0)
		return ConvertExpr(TrimBoth(s.Left(aug_pos))) + " " + aug_op + " " + ConvertExpr(TrimBoth(s.Mid(aug_pos + aug_op.GetCount()))) + ";";

	int eq = s.Find('=');
	bool assignment = eq > 0 && (eq + 1 >= s.GetCount() || s[eq + 1] != '=') && s[eq - 1] != '!' && s[eq - 1] != '<' && s[eq - 1] != '>';
	if(assignment) {
		String lhs = TrimBoth(s.Left(eq));
		String rhs = TrimBoth(s.Mid(eq + 1));
		Vector<String> tuple_lhs = SplitTopLevel(lhs, ',');
		if(tuple_lhs.GetCount() > 1) {
			bool ok = true;
			for(const String& part : tuple_lhs)
				ok = ok && (part == "_" || IsIdentifier(part));
			if(ok) {
				String out = "let [";
				for(int i = 0; i < tuple_lhs.GetCount(); i++) {
					if(i) out << ", ";
					out << tuple_lhs[i];
				}
				out << "] = " << ConvertExpr(rhs) << ";";
				return out;
			}
		}
		return ConvertExpr(lhs) + " = " + ConvertExpr(rhs) + ";";
	}

	return ConvertExpr(s) + ";";
}

static String ConvertEmbeddedConditional(String expr)
{
	bool in_single = false;
	bool in_double = false;
	int depth_paren = 0;
	int depth_bracket = 0;
	int depth_brace = 0;
	int if_pos = -1;
	int else_pos = -1;
	int token_depth = -1;
	for(int i = 0; i < expr.GetCount(); i++) {
		int prev = i > 0 ? expr[i - 1] : 0;
		if(expr[i] == '\'' && !in_double && prev != '\\')
			in_single = !in_single;
		else if(expr[i] == '"' && !in_single && prev != '\\')
			in_double = !in_double;
		else if(!in_single && !in_double) {
			if(expr[i] == '(') depth_paren++;
			else if(expr[i] == ')') depth_paren--;
			else if(expr[i] == '[') depth_bracket++;
			else if(expr[i] == ']') depth_bracket--;
			else if(expr[i] == '{') depth_brace++;
			else if(expr[i] == '}') depth_brace--;
			int depth = depth_paren + depth_bracket + depth_brace;
			if(if_pos < 0 && i + 4 <= expr.GetCount() && expr.Mid(i, 4) == " if ") {
				if_pos = i;
				token_depth = depth;
			}
			else if(if_pos >= 0 && i + 6 <= expr.GetCount() && expr.Mid(i, 6) == " else " && depth == token_depth) {
				else_pos = i;
				break;
			}
		}
	}
	if(if_pos < 0 || else_pos < 0)
		return expr;

	int start_true = 0;
	in_single = in_double = false;
	depth_paren = depth_bracket = depth_brace = 0;
	for(int i = 0; i < if_pos; i++) {
		int prev = i > 0 ? expr[i - 1] : 0;
		if(expr[i] == '\'' && !in_double && prev != '\\')
			in_single = !in_single;
		else if(expr[i] == '"' && !in_single && prev != '\\')
			in_double = !in_double;
		else if(!in_single && !in_double) {
			if(expr[i] == '(') depth_paren++;
			else if(expr[i] == ')') depth_paren--;
			else if(expr[i] == '[') depth_bracket++;
			else if(expr[i] == ']') depth_bracket--;
			else if(expr[i] == '{') depth_brace++;
			else if(expr[i] == '}') depth_brace--;
			int depth = depth_paren + depth_bracket + depth_brace;
			if((expr[i] == ',' || expr[i] == '(' || expr[i] == '[' || expr[i] == '{') && depth == token_depth)
				start_true = i + 1;
		}
	}

	int end_false = expr.GetCount();
	in_single = in_double = false;
	depth_paren = depth_bracket = depth_brace = 0;
	for(int i = else_pos + 6; i < expr.GetCount(); i++) {
		int prev = i > 0 ? expr[i - 1] : 0;
		if(expr[i] == '\'' && !in_double && prev != '\\')
			in_single = !in_single;
		else if(expr[i] == '"' && !in_single && prev != '\\')
			in_double = !in_double;
		else if(!in_single && !in_double) {
			if(expr[i] == '(') depth_paren++;
			else if(expr[i] == ')') depth_paren--;
			else if(expr[i] == '[') depth_bracket++;
			else if(expr[i] == ']') depth_bracket--;
			else if(expr[i] == '{') depth_brace++;
			else if(expr[i] == '}') depth_brace--;
			int depth = token_depth + depth_paren + depth_bracket + depth_brace;
			if((expr[i] == ',' || expr[i] == ')' || expr[i] == ']' || expr[i] == '}') && depth == token_depth) {
				end_false = i;
				break;
			}
		}
	}

	String prefix = expr.Left(start_true);
	String when_true = TrimBoth(expr.Mid(start_true, if_pos - start_true));
	String cond = TrimBoth(expr.Mid(if_pos + 4, else_pos - (if_pos + 4)));
	String when_false = TrimBoth(expr.Mid(else_pos + 6, end_false - (else_pos + 6)));
	String suffix = expr.Mid(end_false);
	if(when_true.IsEmpty() || cond.IsEmpty() || when_false.IsEmpty())
		return expr;
	return prefix + "(" + ConvertExpr(cond) + " ? " + ConvertExpr(when_true) + " : " + ConvertExpr(when_false) + ")" + suffix;
}

static bool IsIdentifier(const String& s)
{
	if(s.IsEmpty())
		return false;
	if(!(IsAlpha(s[0]) || s[0] == '_'))
		return false;
	for(int i = 1; i < s.GetCount(); i++)
		if(!(IsAlNum(s[i]) || s[i] == '_'))
			return false;
	return true;
}

PyToJsResult TranspilePythonToJavascript(const String& source, const String& filename, const PyToJsOptions& options)
{
	PyToJsResult out;
	Vector<String> physical_lines = Split(source, '\n');
	Vector<String> lines;
	Vector<int> line_numbers;
	String current_line;
	int current_lineno = 1;
	for(int i = 0; i < physical_lines.GetCount(); i++) {
		String raw = TrimRightCopy(physical_lines[i]);
		if(current_line.IsEmpty()) {
			current_line = raw;
			current_lineno = i + 1;
		}
		else
			current_line << " " << TrimBoth(raw);

		String cur = TrimRightCopy(current_line);
		bool slash_continue = cur.EndsWith("\\");
		if(slash_continue)
			cur = TrimRightCopy(cur.Left(cur.GetCount() - 1));
		current_line = cur;

		if(slash_continue || NeedsContinuation(current_line))
			continue;

		lines.Add(current_line);
		line_numbers.Add(current_lineno);
		current_line.Clear();
	}
	if(!current_line.IsEmpty()) {
		lines.Add(current_line);
		line_numbers.Add(current_lineno);
	}
	Vector<int> indent_stack;
	Vector<Index<String>> declared;
	indent_stack.Add(0);
	declared.Add();
	bool expect_block_indent = false;
	int pending_block_indent = 0;

	String js;
	if(options.emit_prelude) {
		js << "// transpiled from python: " << filename << "\n";
		js << (options.browser_module ? "\"use strict\";\n\n" : "");
		js << "function len(x) { return x && typeof x.length === \"number\" ? x.length : Object.keys(x || {}).length; }\n";
		js << "function str(x) { return String(x); }\n";
		js << "function __py_assert__(cond, msg) { if(!cond) throw new Error(msg || \"assertion failed\"); }\n";
		js << "function __py_in__(needle, haystack) { if(Array.isArray(haystack) || typeof haystack === \"string\") return haystack.includes(needle); return !!(haystack && Object.prototype.hasOwnProperty.call(haystack, needle)); }\n";
		js << "function __py_remove__(arr, item) { const i = arr.indexOf(item); if(i >= 0) arr.splice(i, 1); }\n\n";
	}

	for(int linei = 0; linei < lines.GetCount(); linei++) {
		int lineno = line_numbers[linei];
		String raw = TrimRightCopy(lines[linei]);
		String raw_stripped = TrimBoth(raw);
		if(raw_stripped.StartsWith("#")) {
			js << Indent(indent_stack.GetCount() - 1) << "//" << raw_stripped.Mid(1) << "\n";
			continue;
		}
		String code = TrimRightCopy(StripInlineComment(raw));
		String stripped = TrimBoth(code);
		if(stripped.IsEmpty()) {
			js << "\n";
			continue;
		}

		int indent = CountIndent(raw, out.warnings, lineno);
		if(expect_block_indent) {
			if(indent <= indent_stack.Top()) {
				out.errors.Add(Format("%s:%d: expected indented block", filename, lineno));
				return out;
			}
			indent_stack.Add(indent);
			declared.Add();
			expect_block_indent = false;
		}

		while(indent < indent_stack.Top()) {
			indent_stack.Drop();
			declared.Drop();
			js << Indent(indent_stack.GetCount() - 1) << "}\n";
		}
		if(indent > indent_stack.Top()) {
			out.warnings.Add(Format("%s:%d: implicit indentation scope accepted", filename, lineno));
			indent_stack.Add(indent);
			declared.Add();
		}
		if(indent != indent_stack.Top()) {
			out.errors.Add(Format("%s:%d: unsupported indentation change", filename, lineno));
			return out;
		}

		String ind = Indent(indent_stack.GetCount() - 1);
		if(stripped.StartsWith("#")) {
			js << ind << "//" << stripped.Mid(1) << "\n";
			continue;
		}
		if(stripped.StartsWith("class ")) {
			out.errors.Add(Format("%s:%d: class is not yet supported", filename, lineno));
			return out;
		}
		if(stripped.StartsWith("def ") && stripped.EndsWith(":")) {
			String sig = TrimBoth(stripped.Mid(4, stripped.GetCount() - 5));
			int lp = sig.Find('(');
			int rp = sig.ReverseFind(')');
			if(lp < 0 || rp < lp) {
				out.errors.Add(Format("%s:%d: invalid function signature", filename, lineno));
				return out;
			}
			String name = TrimBoth(sig.Left(lp));
			String args = TrimBoth(sig.Mid(lp + 1, rp - lp - 1));
			js << ind << "function " << name << "(" << args << ") {\n";
			expect_block_indent = true;
			continue;
		}
		int inline_if_colon = stripped.StartsWith("if ") ? FindTopLevelToken(stripped, ": ") : -1;
		if(inline_if_colon > 0) {
			String cond = TrimBoth(stripped.Mid(3, inline_if_colon - 3));
			String stmt = TrimBoth(stripped.Mid(inline_if_colon + 2));
			js << ind << "if (" << ConvertExpr(cond) << ") { " << ConvertSimpleStatement(stmt) << " }\n";
			continue;
		}
		if(stripped.StartsWith("if ") && stripped.EndsWith(":")) {
			js << ind << "if (" << ConvertExpr(stripped.Mid(3, stripped.GetCount() - 4)) << ") {\n";
			expect_block_indent = true;
			continue;
		}
		if(stripped.StartsWith("elif ") && stripped.EndsWith(":")) {
			js << ind << "else if (" << ConvertExpr(stripped.Mid(5, stripped.GetCount() - 6)) << ") {\n";
			expect_block_indent = true;
			continue;
		}
		if(stripped == "else:") {
			js << ind << "else {\n";
			expect_block_indent = true;
			continue;
		}
		if(stripped.StartsWith("while ") && stripped.EndsWith(":")) {
			js << ind << "while (" << ConvertExpr(stripped.Mid(6, stripped.GetCount() - 7)) << ") {\n";
			expect_block_indent = true;
			continue;
		}
		if(stripped.StartsWith("for ") && stripped.EndsWith(":")) {
			String rest = stripped.Mid(4, stripped.GetCount() - 5);
			int in_pos = rest.Find(" in ");
			if(in_pos < 0) {
				out.errors.Add(Format("%s:%d: unsupported for syntax", filename, lineno));
				return out;
			}
			String var = TrimBoth(rest.Left(in_pos));
			String expr = TrimBoth(rest.Mid(in_pos + 4));
			if(expr.StartsWith("range(") && expr.EndsWith(")")) {
				String inside = expr.Mid(6, expr.GetCount() - 7);
				Vector<String> parts = Split(inside, ',');
				if(parts.GetCount() == 1)
					js << ind << "for (let " << var << " = 0; " << var << " < " << ConvertExpr(parts[0]) << "; " << var << "++) {\n";
				else if(parts.GetCount() == 2)
					js << ind << "for (let " << var << " = " << ConvertExpr(parts[0]) << "; " << var << " < " << ConvertExpr(parts[1]) << "; " << var << "++) {\n";
				else {
					out.errors.Add(Format("%s:%d: range with this arity is unsupported", filename, lineno));
					return out;
				}
			}
			else
				js << ind << "for (const " << var << " of " << ConvertExpr(expr) << ") {\n";
			expect_block_indent = true;
			continue;
		}
		if(stripped == "pass" || stripped.StartsWith("global ") || stripped == "break" || stripped == "continue" || stripped.StartsWith("assert ") || stripped.StartsWith("return")) {
			js << ind << ConvertSimpleStatement(stripped) << "\n";
			continue;
		}
		if(stripped.StartsWith("import ")) {
			String mod = TrimBoth(stripped.Mid(7));
			js << ind << "const " << mod << " = __py_import__(\"" << mod << "\");\n";
			continue;
		}
		if(stripped.StartsWith("from ")) {
			int import_pos = stripped.Find(" import ");
			if(import_pos < 0) {
				out.errors.Add(Format("%s:%d: invalid from import syntax", filename, lineno));
				return out;
			}
			String mod = TrimBoth(stripped.Mid(5, import_pos - 5));
			String names = TrimBoth(stripped.Mid(import_pos + 8));
			js << ind << "const {" << names << "} = __py_import__(\"" << mod << "\");\n";
			continue;
		}

		int eq = stripped.Find('=');
		bool assignment = eq > 0 && (eq + 1 >= stripped.GetCount() || stripped[eq + 1] != '=') && stripped[eq - 1] != '!' && stripped[eq - 1] != '<' && stripped[eq - 1] != '>';
		if(assignment) {
			String lhs = TrimBoth(stripped.Left(eq));
			String rhs = TrimBoth(stripped.Mid(eq + 1));
			if(IsIdentifier(lhs)) {
				bool declared_here = false;
				for(int i = declared.GetCount() - 1; i >= 0 && !declared_here; i--)
					declared_here = declared[i].Find(lhs) >= 0;
				if(!declared_here) {
					declared.Top().Add(lhs);
					js << ind << "let " << lhs << " = " << ConvertExpr(rhs) << ";\n";
				}
				else
					js << ind << lhs << " = " << ConvertExpr(rhs) << ";\n";
				continue;
			}
		}

		js << ind << ConvertSimpleStatement(stripped) << "\n";
	}

	while(indent_stack.GetCount() > 1) {
		indent_stack.Drop();
		declared.Drop();
		js << Indent(indent_stack.GetCount() - 1) << "}\n";
	}

	out.ok = out.errors.IsEmpty();
	out.javascript = js;
	return out;
}

END_UPP_NAMESPACE
