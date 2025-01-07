#ifndef _DropTerm_Console_h_
#define _DropTerm_Console_h_

class DropTerm;

class ConsoleCtrl : public ParentCtrl {
	
protected:
	CommandPrompt cmd;
	One<MetaExtCtrl> ext;
	Ctrl* active = 0;
	
	#ifdef flagHAVE_INTRANET
	One<FTPServer> ftpd;
	#endif
	
	DropTerm* bridge = NULL;
	int id = -1;
	
	ArrayMap<String, Callback1<String> > commands;
	String out, err;
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
	void RemoveExt();
	void AddProgram(String cmd, Callback1<String> cb);
	bool Command(const String& cmd);
	
	void ListFiles(String arg);
	void ChangeDirectory(String arg);
	void CreateDirectory(String arg);
	void RemoveFile(String arg);
	void ShowFile(String arg);
	void EditFile(String arg);
	void DownloadFile(String arg);
	
	#ifdef flagHAVE_INTRANET
	void StartFtpServer(String arg);
	#endif
	
	void Menu(Bar& bar);
	String GetTitle();
	
	inline void Put(const String& s)		{out << s;}
	inline void PutLine(const String& s)	{out << s << "\n";}
	const String& GetOutput() const			{return out;}
	const String& GetError() const			{return err;}
	
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

#endif
