#ifndef _DockingTemplate2_AppState_h_
#define _DockingTemplate2_AppState_h_

struct AppState {
	String active_tab_name;    // primary: stable tab label
	int    active_tab_index = 0; // fallback if label not found
	int    editor_mode      = 0; // 0 = View, 1 = Edit

	void Jsonize(JsonIO& jio) {
		jio("active_tab_name",  active_tab_name)
		   ("active_tab_index", active_tab_index)
		   ("editor_mode",      editor_mode);
	}
};

#endif
