#ifndef _SingerTrainer_ExerciseEditor_h_
#define _SingerTrainer_ExerciseEditor_h_

#include <CtrlLib/CtrlLib.h>
#include "Scripting.h"

namespace Upp {

class ExerciseEditor : public ParentCtrl {
	DocEdit editor;
	Button  save_btn;
	Button  run_btn;
	EditField filename_field;
	
	GuiBridge* scripting = nullptr;

public:
	typedef ExerciseEditor CLASSNAME;
	ExerciseEditor();
	
	void SetScripting(GuiBridge& s) { scripting = &s; }
	
	void Load(const String& path);
	void Save();
	void RunScript();
	
	String GetEditorContent() const { return editor.Get(); }
};

}

#endif
