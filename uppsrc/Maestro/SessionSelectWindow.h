
#ifndef _Maestro_SessionSelectWindow_h_
#define _Maestro_SessionSelectWindow_h_

struct RecentConfig {
	Vector<String> recent_dirs;
	
	void Jsonize(JsonIO& jio) {
		jio("recent_dirs", recent_dirs);
	}
	
	void Load() {
		String f = ConfigFile("memory.json");
		if(FileExists(f))
			LoadFromJsonFile(*this, f);
	}
	
	void Save() {
		StoreAsJsonFile(*this, ConfigFile("memory.json"));
	}
	
	void AddDir(const String& d) {
		if(d.IsEmpty()) return;
		int q = -1;
		for(int i = 0; i < recent_dirs.GetCount(); i++)
			if(recent_dirs[i] == d) { q = i; break; }
		if(q >= 0) recent_dirs.Remove(q);
		recent_dirs.Insert(0, d);
		if(recent_dirs.GetCount() > 20) recent_dirs.Drop();
		Save();
	}
};

class SessionListView : public ArrayCtrl {
public:
	void SetSessions(const Array<SessionInfo>& sessions);
	
	typedef SessionListView CLASSNAME;
	SessionListView();
};

class SessionSelectWindow : public TopWindow {
public:
	Splitter  split;
	ArrayCtrl dirs;
	SessionListView sessions; // Upgraded from ArrayCtrl
	
	CliMaestroEngine* engine = nullptr;
	String    selected_id;
	
	void DataDirectories();
	void OnDirCursor();
	void OnSessionDouble();
	void OnSessionMenu(Bar& bar);
	
	void Load(CliMaestroEngine& engine);

	typedef SessionSelectWindow CLASSNAME;
	SessionSelectWindow(CliMaestroEngine& engine);
};

class NewSessionWindow : public TopWindow {
public:
	EditString  dir;
	Button      recent_btn;
	DropList    backend;
	
	Button      btn_new;
	Button      btn_resume;
	Button      btn_cancel;
	
	String      session_id;
	String      selected_backend;
	String      selected_dir;
	RecentConfig& config;
	
	void OnRecent();
	void OnResume();
	void OnNew();

	typedef NewSessionWindow CLASSNAME;
	NewSessionWindow(RecentConfig& cfg);
};

#endif

