#ifndef _DropTerm_Console_h_
#define _DropTerm_Console_h_

NAMESPACE_UPP

class DropTerm;

class ConsoleCtrl : public ParentCtrl, IdeShellHost {
	
protected:
	IdeShell cmd;
	One<MetaExtCtrl> ext;
	Ctrl* active = 0;
	
	DropTerm* bridge = NULL;
	int id = -1;
	
	String cwd;
	String filename;
	Event<> SaveEditPos;
	Event<> LoadEditPos;
	
	void SetView();
	void SetTitle(String s);
	
public:
	typedef ConsoleCtrl CLASSNAME;
	ConsoleCtrl();
	~ConsoleCtrl();
	
	bool RealizeFocus();
	void RemoveExt(bool fast_exit=false);
	void ListFiles(String arg);
	void ChangeDirectory(String arg);
	void CreateDirectory(String arg);
	void RemoveFile(String arg);
	void ShowFile(String arg);
	void EditFile(String arg);
	void DownloadFile(String arg);
	
	void Menu(Bar& bar);
	String GetTitle();
	
	void SetBridge(DropTerm* bridge, int id) {this->bridge = bridge; this->id = id;}
	
	Callback WhenTitle;
	Callback WhenViewChange;
	
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
