#ifndef _VisualStateWorkbench_AppState_h_
#define _VisualStateWorkbench_AppState_h_

struct AppState {
	String active_tab_name;
	int    active_tab_index = 0;
	String last_session_path;

	void Jsonize(JsonIO& json) {
		json("active_tab_name",  active_tab_name)
		    ("active_tab_index", active_tab_index)
		    ("last_session_path", last_session_path);
	}
};

#endif
