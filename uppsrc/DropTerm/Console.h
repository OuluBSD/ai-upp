#ifndef _DropTerm_Console_h_
#define _DropTerm_Console_h_

class DropTerm;

class ConsoleCtrl : public ParentCtrl {
	
protected:
	CommandPrompt cmd;
	Word wordapp;
	
	#ifdef flagHAVE_INTRANET
	One<FTPServer> ftpd;
	#endif
	
	DropTerm* bridge = NULL;
	int id = -1;
	
	ArrayMap<String, Callback1<String> > commands;
	String out, err;
	String cwd;
	int view = VIEW_CMD;
	
	enum {VIEW_CMD, VIEW_WORD};
	void SetView(int i);
	void SetTitle(String s);
	
public:
	typedef ConsoleCtrl CLASSNAME;
	ConsoleCtrl();
	
	void AddProgram(String cmd, Callback1<String> cb);
	String Command(const String& cmd);
	
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
	String GetMenuTitle();
	String GetTitle();
	
	inline void Put(const String& s)		{out << s;}
	inline void PutLine(const String& s)	{out << s << "\n";}
	const String& GetOutput() const			{return out;}
	const String& GetError() const			{return err;}
	
	void SetBridge(DropTerm* bridge, int id) {this->bridge = bridge; this->id = id;}
	
	Callback WhenTitle;
	Callback WhenViewChange;
	
};

#endif
