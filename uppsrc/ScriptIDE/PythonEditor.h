#ifndef _ScriptIDE_PythonEditor_h_
#define _ScriptIDE_PythonEditor_h_

class PythonEditor : public CodeEditor {
public:
	void ToggleComments() { ToggleLineComments(false); }
	void ToggleBlockComments() { ToggleStarComments(); }
	
	void DoSelectAll() { SelectAll(); }
	void DoFind(bool replace, bool word) { Find(replace, word); }
	void DoFindNext() { FindNext(); }
	void DoFindPrev() { FindPrev(); }
	void DoReplace() { Replace(); }
	
	String GetCurrentCell() {
		int line = GetLine(GetCursor());
		int start = 0;
		for(int i = line; i >= 0; i--) {
			if(Upp::TrimLeft(GetUtf8Line(i)).StartsWith("# %%")) {
				start = i;
				break;
			}
		}
		int end = GetLineCount();
		for(int i = line + 1; i < GetLineCount(); i++) {
			if(Upp::TrimLeft(GetUtf8Line(i)).StartsWith("# %%")) {
				end = i;
				break;
			}
		}
		String res;
		for(int i = start; i < end; i++) {
			res << GetUtf8Line(i) << "\n";
		}
		return res;
	}
};

#endif
