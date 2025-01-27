#ifndef _ide_Shell_Console_h_
#define _ide_Shell_Console_h_

NAMESPACE_UPP

class DropTerm;

class ConsoleCtrl : public ParentCtrl, IdeShellHost {
	
protected:
	IdeShell cmd;
	One<WidgetCtrl> ext;
	Ctrl* active = 0;
	MenuBar menu;
	DropTerm* bridge = NULL;
	int id = -1;
	bool internal_menubar = false;
	
	String cwd;
	String filename;
	Event<> SaveEditPos;
	Event<> LoadEditPos;
	void SetView();
	void SetTitle(String s);
	void AddMenuBar();
	void RemoveMenuBar();
public:
	typedef ConsoleCtrl CLASSNAME;
	ConsoleCtrl();
	~ConsoleCtrl();
	
	void InternalMenuBar(bool b=true) {internal_menubar = b;}
	bool IsMenuBarVisible() const {return ext;}
	IdeShell& Shell() {return cmd;}
	bool RealizeFocus();
	void RemoveExt(bool fast_exit=false);
	void DraftFile(IdeShell& shell, Value value);
	
	void Menu(Bar& bar);
	String GetTitle();
	
	void SetBridge(DropTerm* bridge, int id) {this->bridge = bridge; this->id = id;}
	
	Callback WhenTitle;
	Callback WhenViewChange;
	
	template <class T> void SimpleExt(IdeShell& shell, Value value) {
		this->CreateExt<T>();
		SetView();
	}

	template <class T> void SaveExtPos() {
		if (!ext) return;
		auto& cache = EditPosCached<T>::EditPosCache();
		ASSERT(ext);
		T* o = dynamic_cast<T*>(&*ext);
		if (!o) return;
		JsonIO jio;
		o->EditPos(jio);
		cache.GetAdd(filename) = jio.GetResult();
		if (1) // TODO optimize (unnecessarily often here)
			EditPosCached<T>::SaveEditPosCache();
	}
	
	template <class T> void LoadExtPos() {
		if (!ext) return;
		auto& cache = EditPosCached<T>::EditPosCache();
		if (cache.IsEmpty())
			EditPosCached<T>::LoadEditPosCache();
		T* o = dynamic_cast<T*>(&*ext);
		if (!o) return;
		JsonIO jio(cache.GetAdd(filename));
		o->EditPos(jio);
	}
	
	template <class T> bool CreateExt() {
		if (ext) RemoveExt();
		T* o = new T;
		ext = o;
		o->WhenTitle << Proxy(WhenTitle);
		SaveEditPos = THISBACK(SaveExtPos<T>);
		LoadEditPos = THISBACK(LoadExtPos<T>);
		LoadExtPos<T>();
		return true;
	}
	
};

END_UPP_NAMESPACE

#endif
