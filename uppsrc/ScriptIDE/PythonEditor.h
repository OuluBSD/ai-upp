#ifndef _ScriptIDE_PythonEditor_h_
#define _ScriptIDE_PythonEditor_h_

class PythonEditor : public CodeEditor {
public:
	void ToggleComments() { ToggleLineComments(false); }
};

#endif
