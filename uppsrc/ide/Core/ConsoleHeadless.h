#ifndef _ide_ConsoleHeadless_h_
#define _ide_ConsoleHeadless_h_

#include "Core.h"

namespace Upp {

class IdeCliConsole {
public:
	IdeCliConsole();
	void Append(const String& s);
	int  Flush();
	int  Execute(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, bool noconvert = false);
	int  Execute(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false);
	int  AllocSlot();
	bool Run(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1);
	bool Run(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1);
	bool IsRunning();
	bool Wait();
	void Kill();

private:
	struct Slot {
		One<AProcess> process;
		Stream       *outfile = NULL;
		bool          quiet = false;
	};

	Array<Slot> processes;
};

class IdeCliContext : public IdeContext {
public:
	IdeCliContext();

	virtual bool   IsVerbose() const override { return true; }
	virtual void   PutConsole(const char *s) override;
	virtual void   PutVerbose(const char *s) override;
	virtual void   PutLinking() override;
	virtual void   PutLinkingEnd(bool ok) override;
	virtual const Workspace& IdeWorkspace() const override;
	virtual bool   IdeIsBuilding() const override;
	virtual String IdeGetOneFile() const override;
	virtual int    IdeConsoleExecute(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, bool noconvert = false) override;
	virtual int    IdeConsoleExecute(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false) override;
	virtual int    IdeConsoleExecuteWithInput(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert) override;
	virtual int    IdeConsoleAllocSlot() override;
	virtual bool   IdeConsoleRun(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1) override;
	virtual bool   IdeConsoleRun(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1) override;
	virtual void   IdeConsoleFlush() override;
	virtual void   IdeConsoleBeginGroup(String group) override;
	virtual void   IdeConsoleEndGroup() override;
	virtual bool   IdeConsoleWait() override;
	virtual bool   IdeConsoleWait(int slot) override;
	virtual bool   IdeConsoleGetError() override;
	virtual void   IdeConsoleOnFinish(Event<> cb) override;
	virtual void   IdeProcessEvents() override;
	virtual bool   IdeIsDebug() const override { return false; }
	virtual void   IdeEndDebug() override {}
	virtual void   IdeSetBottom(Ctrl& ctrl) override {}
	virtual void   IdeActivateBottom() override {}
	virtual void   IdeRemoveBottom(Ctrl& ctrl) override {}
	virtual void   IdeSetRight(Ctrl& ctrl) override {}
	virtual void   IdeRemoveRight(Ctrl& ctrl) override {}
	virtual String IdeGetFileName() const override { return Null; }
	virtual int    IdeGetFileLine() override { return 0; }
	virtual String IdeGetLine(int i) const override { return Null; }
	virtual void   IdeSetDebugPos(const String& fn, int line, const Image& img, int i) override {}
	virtual void   IdeHidePtr() override {}
	virtual bool   IdeDebugLock() override { return false; }
	virtual bool   IdeDebugUnLock() override { return false; }
	virtual bool   IdeIsDebugLock() const override { return false; }
	virtual void   IdeSetBar() override {}
	virtual void   IdeOpenTopicFile(const String& file) override {}
	virtual void   IdeFlushFile() override {}
	virtual String IdeGetFileName() override { return Null; }
	virtual String IdeGetNestFolder() override { return Null; }
	virtual String IdeGetIncludePath() override { return Null; }
	virtual bool   IsPersistentFindReplace() override { return false; }
	virtual int    IdeGetHydraThreads() override;
	virtual String IdeGetCurrentBuildMethod() override { return Null; }
	virtual String IdeGetCurrentMainPackage() override;
	virtual void   IdePutErrorLine(const String&) override {}

private:
	IdeCliConsole console;
	Workspace     workspace;
};

bool HandleConsoleIdeArgs(const Vector<String>& args);
String GetConsoleIdeExperimentalNotice();

} // namespace Upp

#endif
