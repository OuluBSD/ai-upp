#include "Shell.h"

NAMESPACE_UPP

IdeShell::IdeShell(IdeShellHostBase& h) : host(h)
{
	cwd.Set(INTERNAL_ROOT_PATH);
	Highlight("calc");
	NoHorzScrollbar();
	HideBar();
	Escape(UscGlobal(), "pow(x,y)", EscPow);
	Escape(UscGlobal(), "sqrt(x)", EscSqrt);
}

int LfToSpaceFilter(int c)
{
	return c == '\n' ? ' ' : c;
}

void IdeShell::Execute()
{
	int li = GetLineCount() - 1;
	if(IsSelection()) {
		String s = GetSelection();
		if(s.GetLength() < 80) {
			SetCursor(GetLength());
			Paste(Filter(s, LfToSpaceFilter).ToWString());
		}
		return;
	}
	if(GetLine(GetCursor()) != li) {
		WString s = GetWLine(GetLine(GetCursor()));
		if(s[0] == '$') s = s.Mid(1);
		SetCursor(GetLength());
		Paste(s);
		return;
	}

	String txt;
	bool try_math = false;
	String s = TrimBoth(GetUtf8Line(li));
	if (s.IsEmpty()) return;
	
	bool succ = false;
	
	if (!succ) try {
		Vector<String> parts = Split(s, " ", true, true);
		ValueArray arr;
		for (String& s: parts) arr.Add(s);
		if (host.Command(*this,arr)) {
			succ = true;
			txt = host.GetOutput();
		}
	}
	catch (Exc e) {
		txt << "ERROR: " << e;
	}
	
	if (!succ) try {
		ArrayMap<String, EscValue>& g = UscGlobal();
		for(int i = 0; i < g.GetCount(); i++)
			vars.GetAdd(g.GetKey(i)) = g[i];
		if(IsNull(s))
			return;
		EscValue v = Evaluatex(s, vars);
		txt = v.ToString(GetSize().cx / max(1, GetFont().Info()['x']), 4, true);
		vars.GetAdd("_") = v;
	}
	catch(CParser::Error e) {
		const char *x = strchr(e, ':');
		if (!txt.IsEmpty()) txt.Cat('\n');
		txt << "ERROR: " << (x ? x + 1 : ~e);
	}
	
	SetCursor(GetPos(li));
	Paste("$");
	SetCursor(GetLength());
	Paste("\n");
	if (!txt.IsEmpty()) {
		Paste(txt.ToWString());
		Paste("\n");
	}
}

void IdeShell::LeftDouble(Point p, dword flags)
{
	CodeEditor::LeftDouble(p, flags);
	if(IsSelection())
		Execute();
}

bool IdeShell::Key(dword key, int count)
{
	if (key & K_DELTA && (key & K_CTRL || key & K_SHIFT || key & K_ALT)) {
		TopWindow* tw = GetTopWindow();
		if (tw && tw->HotKey(key))
			return true;
	}
	switch(key) {
	case K_ENTER:
		Execute();
		break;
	default:
		return CodeEditor::Key(key, count);
	}
	return true;
}

END_UPP_NAMESPACE
