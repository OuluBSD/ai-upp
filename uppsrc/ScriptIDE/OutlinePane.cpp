#include "ScriptIDE.h"

namespace Upp {

OutlinePane::OutlinePane()
{
	Title("Outline");
	Icon(CtrlImg::plus()); // Use plus as placeholder
	
	Add(tree.SizePos());
	tree.WhenSel = [=] { OnSelect(); };
}

void OutlinePane::UpdateOutline(const String& code)
{
	tree.Clear();
	tree.SetRoot(CtrlImg::File(), "Symbols");
	
	Vector<String> lines = Split(code, '\n', false);
	for(int i = 0; i < lines.GetCount(); i++) {
		String line = TrimBoth(lines[i]);
		if(line.StartsWith("class ")) {
			String name = line.Mid(6);
			int q = name.Find(':');
			if(q >= 0) name = name.Left(q);
			tree.Add(0, CtrlImg::Dir(), i + 1, "class " + TrimBoth(name));
		}
		else if(line.StartsWith("def ")) {
			String name = line.Mid(4);
			int q = name.Find('(');
			if(q >= 0) name = name.Left(q);
			tree.Add(0, CtrlImg::plus(), i + 1, "def " + TrimBoth(name));
		}
	}
	
	tree.OpenDeep(0);
}

void OutlinePane::Clear()
{
	tree.Clear();
}

void OutlinePane::OnSelect()
{
	int id = tree.GetCursor();
	if(id > 0) {
		Value v = tree.Get(id);
		if(!v.IsNull())
			WhenSelectLine((int)v);
	}
}

}
