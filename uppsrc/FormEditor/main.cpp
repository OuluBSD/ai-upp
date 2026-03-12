#include "FormEditor.h"

#ifdef flagMAIN
GUI_APP_MAIN
{
	int mode = 0;
	String open_path;
	if(CommandLine().GetCount())
		open_path = CommandLine()[0];
	
	if (mode == 0) {
		DockableFormEdit editor;
		if(!open_path.IsEmpty())
			editor.OpenPath(open_path);
		editor.Run();
	}
	else if (mode == 1) {
		FormEditWindow editor;
		if(!open_path.IsEmpty())
			editor.OpenPath(open_path);
		editor.Run();
	}
	else {
		TopWindow tw;
		FormEditCtrl c;
		if(!open_path.IsEmpty())
			c.OpenPath(open_path);
		tw.Add(c.SizePos());
		tw.Run();
	}
}
#endif
