#include "CodeEditor.h"

namespace Upp {

static Color PythonBlockColor(int level)
{
	if(HighlightSetup::hilite_scope == 1)
		return HighlightSetup::GetHlStyle(level & 1 ? HighlightSetup::PAPER_BLOCK1 : HighlightSetup::PAPER_NORMAL).color;
	if(HighlightSetup::hilite_scope == 2) {
		int q = level % 5;
		return HighlightSetup::GetHlStyle(q ? HighlightSetup::PAPER_BLOCK1 + q - 1 : HighlightSetup::PAPER_NORMAL).color;
	}
	return HighlightSetup::GetHlStyle(HighlightSetup::PAPER_NORMAL).color;
}

static bool LineEndsWithBlockColon(const wchar *s, const wchar *end)
{
	bool in_string = false;
	wchar delim = 0;
	bool escape = false;
	const wchar *limit = end;
	for(const wchar *p = s; p < end; p++) {
		wchar c = *p;
		if(in_string) {
			if(escape) {
				escape = false;
				continue;
			}
			if(c == '\\') {
				escape = true;
				continue;
			}
			if(c == delim)
				in_string = false;
			continue;
		}
		if(c == '\'' || c == '"') {
			in_string = true;
			delim = c;
			escape = false;
			continue;
		}
		if(c == '#') {
			limit = p;
			break;
		}
	}
	while(limit > s && (limit[-1] == ' ' || limit[-1] == '\t'))
		limit--;
	return limit > s && limit[-1] == ':';
}

void PythonSyntax::Clear()
{
	block_indent.Clear();
	block_indent.Add(0);
	expect_indent = false;
}

void PythonSyntax::ScanSyntax(const wchar *ln, const wchar *e, int line, int tab_size)
{
	const wchar *p = ln;
	int pos = 0;
	int lindent = 0;
	while(p < e && (*p == '\t' || *p == ' ')) {
		if(*p++ == '\t' || ++pos >= tab_size) {
			pos = 0;
			lindent++;
		}
	}
	bool significant = p < e && p[0] != '#';
	if(block_indent.IsEmpty())
		block_indent.Add(0);
	if(significant) {
		while(block_indent.GetCount() > 1 && lindent < block_indent.Top())
			block_indent.Drop();
		bool need_indent = expect_indent;
		if(need_indent && lindent > block_indent.Top())
			block_indent.Add(lindent);
		expect_indent = LineEndsWithBlockColon(ln, e);
	}
	(void)line;
}

void PythonSyntax::Serialize(Stream& s)
{
	s % block_indent % expect_indent;
}

void PythonSyntax::Highlight(const wchar *s, const wchar *end, HighlightOutput& hls, CodeEditor *editor, int line, int64 pos)
{
	int tabsize = editor ? editor->GetTabSize() : 4;
	int linelen = int(end - s);
	int lindent = 0;
	int wspos = 0;
	int lindent_chars = 0;
	const wchar *p0 = s;
	while(p0 < end && (*p0 == '\t' || *p0 == ' ')) {
		if(*p0++ == '\t' || ++wspos >= tabsize) {
			wspos = 0;
			lindent++;
		}
	}
	lindent_chars = int(p0 - s);
	int level = max(0, block_indent.GetCount() - 1);
	bool significant = p0 < end && p0[0] != '#';
	if(significant && !block_indent.IsEmpty()) {
		while(level > 0 && lindent < block_indent[level])
			level--;
		if(expect_indent && lindent > block_indent[level])
			level++;
	}
	if(hilite_scope) {
		int i = 0;
		int bid = 0;
		int tabpos = 0;
		while(bid < level && i < lindent_chars) {
			hls.SetPaper(i, 1, PythonBlockColor(bid));
			if(s[i] == '\t' || ++tabpos >= tabsize) {
				tabpos = 0;
				bid++;
			}
			i++;
		}
		hls.SetPaper(i, 1 + max(0, linelen - i), PythonBlockColor(level));
	}
	else
		hls.SetPaper(0, linelen + 1, hl_style[PAPER_NORMAL].color);

	const HlStyle& ink = hl_style[INK_NORMAL];
	while(s < end) {
		int c = *s;
		dword pair = MAKELONG(s[0], s[1]);
		if(c == '#') {
			hls.Put(int(end - s), hl_style[INK_COMMENT]);
			return;
		}
		else
		if(findarg(pair, MAKELONG('0', 'x'), MAKELONG('0', 'X'), MAKELONG('0', 'b'), MAKELONG('0', 'B'),
		                 MAKELONG('0', 'o'), MAKELONG('0', 'O')) >= 0)
			s = HighlightHexBin(hls, s, 2, thousands_separator);
		else
		if(IsDigit(c))
			s = HighlightNumber(hls, s, thousands_separator, false, false);
		else
		if(c == '\'' || c == '\"') {
			const wchar *s0 = s;
			s++;
			for(;;) {
				int c1 = *s;
				if(s >= end || c1 == c) {
					s++;
					hls.Put((int)(s - s0), hl_style[INK_CONST_STRING]);
					break;
				}
				s += 1 + (c1 == '\\' && s[1] == c);
			}
		}
		else
		if(IsAlpha(c) || c == '_') {
			static Index<String> kws = { "False", "await", "else", "import", "pass", "None", "break",
			                             "except", "in", "raise", "True", "class", "finally", "is",
			                             "return", "and", "continue", "for", "lambda", "try", "as",
			                             "def", "from", "nonlocal", "while", "assert", "del", "global",
			                             "not", "with", "async", "elif", "if", "or", "yield" };
			static Index<String> sws = { "self", "NotImplemented", "Ellipsis", "__debug__", "__file__", "__name__" };
			String w;
			while(s < end && (IsAlNum(*s) || *s == '_'))
				w.Cat(*s++);
			hls.Put(w.GetCount(), kws.Find(w) >= 0 ? hl_style[INK_KEYWORD] :
			                      sws.Find(w) >= 0 ? hl_style[INK_UPP] :
			                      ink);
		}
		else
		if(c == '\\' && s[1]) {
			hls.Put(2, ink);
			s += 2;
		}
		else {
			bool hl = findarg(c, '[', ']', '(', ')', ':', '-', '=', '{', '}', '/', '<', '>', '*',
			                     '#', '@', '\\', '.') >= 0;
			hls.Put(1, hl ? hl_style[INK_OPERATOR] : ink);
			s++;
		}
	}
}

void PythonSyntax::IndentInsert(CodeEditor& editor, int chr, int count)
{
	if(chr == '\n') {
		while(count--) {
			WString cursorLine = editor.GetWLine(editor.GetCursorLine());
			editor.InsertChar('\n', 1);
			
			Identation::Type idType = FindIdentationType(editor, cursorLine);
			char idChar = GetIdentationByType(idType);
			int mult = 1;
			if(idType == Identation::Space)
				mult = CalculateSpaceIndetationSize(editor);
			if(LineHasColon(cursorLine))
				editor.InsertChar(idChar, mult);
			editor.InsertChar(idChar, CalculateLineIndetations(cursorLine, idType));
		}
	}
	if(count > 0)
		editor.InsertChar(chr, count, true);
}

bool PythonSyntax::LineHasColon(const WString& line)
{
	for(int i = line.GetLength() - 1; i >= 0; i--) {
		if(line[i] == ':')
			return true;
	}
	return false;
}

int PythonSyntax::CalculateLineIndetations(const WString& line, Identation::Type type)
{
	int count = 0;
	for(int i = 0; i < line.GetLength(); i++) {
		if(type == Identation::Tab && line[i] == '\t')
			count++;
		else
		if(type == Identation::Space && line[i] == ' ')
			count++;
		else
			break;
	}
	return count;
}

PythonSyntax::Identation::Type PythonSyntax::FindIdentationType(CodeEditor& editor, const WString& line)
{
	Identation::Type type = Identation::Unknown;
	if(line.StartsWith("\t"))
		type = Identation::Tab;
	else
	if(line.StartsWith(" "))
		type = Identation::Space;
	else {
		for(int i = 0; i < editor.GetLineCount(); i++) {
			WString cLine = editor.GetWLine(i);
			if(cLine.StartsWith("\t")) {
				type = Identation::Tab;
				break;
			}
			else
			if(cLine.StartsWith(" ")) {
				type = Identation::Space;
				break;
			}
		}
	}
	return type;
}

int PythonSyntax::CalculateSpaceIndetationSize(CodeEditor& editor)
{
	int current = 0;
	for(int i = 0; i < editor.GetLineCount(); i++) {
		WString line = editor.GetWLine(i);
		for(int j = 0; j < line.GetLength(); j++) {
			if(line[j] == ' ')
				current++;
			else
				break;
		}
		
		if(current > 0)
			break;
	}
	
	// TODO: 4 is magic number - try to find the way to get this number from ide constants
	return current > 0 ? current : 4;
}

char PythonSyntax::GetIdentationByType(Identation::Type type)
{
	if(type == Identation::Space)
		return ' ';
	return '\t';
}

}
