#ifndef _ide_Shell_Shell_h_
#define _ide_Shell_Shell_h_

#include <CtrlLib/CtrlLib.h>
#include <CodeEditor/CodeEditor.h>
#include <ide/Core/Core.h>

#include "VFS.h"
#include "Mount.h"
#include "EscCmds.h"

NAMESPACE_UPP

struct IdeShellHostBase {
	IdeShellHostBase();
	virtual ~IdeShellHostBase();
	virtual bool Command(Value arg) = 0;
	virtual const String& GetOutput() const = 0;
	virtual const String& GetError() const = 0;
	
	ArrayMap<String, EscValue> vars;
};

struct IdeShellHost : IdeShellHostBase {
	typedef IdeShellHost CLASSNAME;
	IdeShellHost();
	bool Command(Value arg) override;
	const String& GetOutput() const override;
	const String& GetError() const override;
	void Put(const String& s);
	void PutLine(const String& s);
	void AddProgram(String cmd, Callback1<Value> cb);
	void ListFiles(Value arg);
	void ChangeDirectory(Value arg);
	
	#ifdef flagHAVE_INTRANET
	void StartIntranet(Value arg);
	#endif
	
	String out, err;
	ArrayMap<String, Callback1<Value>> commands;
};

struct IdeShell : Upp::CodeEditor {
	IdeShell(IdeShellHostBase& h);
	void    Execute();

	virtual bool Key(dword key, int count);
	virtual void LeftDouble(Point p, dword flags);

	ArrayMap<String, EscValue> vars;
	IdeShellHostBase& host;

};


END_UPP_NAMESPACE

#endif
