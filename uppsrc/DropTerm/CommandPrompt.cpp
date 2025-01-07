#include "DropTerm.h"

void EscPow(EscEscape& e)
{
	if (e[0].IsNumber() && e[1].IsNumber()) {
		double a = e[0].GetNumber();
		double b = e[1].GetNumber();
		e = pow(a, b);
	}
}

void EscSqrt(EscEscape& e)
{
	if (e[0].IsNumber()) {
		double a = e[0].GetNumber();
		e = sqrt(a);
	}
}

CommandPrompt::CommandPrompt(ConsoleCtrl* cons) : cons(cons)
{
	Highlight("calc");
	NoHorzScrollbar();
	HideBar();
	Escape(UscGlobal(), "pow(x,y)", EscPow);
	Escape(UscGlobal(), "sqrt(x)", EscSqrt);
}

int LfToSpaceFilter(int c);

void CommandPrompt::Execute()
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
		if (cons->Command(s)) {
			succ = true;
			txt = cons->GetOutput();
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

void CommandPrompt::LeftDouble(Point p, dword flags)
{
	CodeEditor::LeftDouble(p, flags);
	if(IsSelection())
		Execute();
}

bool CommandPrompt::Key(dword key, int count)
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

