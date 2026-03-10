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
};

#endif
