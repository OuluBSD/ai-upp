#ifndef _ide_ConsoleHeadless_h_
#define _ide_ConsoleHeadless_h_
#include "Core.h"
#include <ide/Builders/Builders.h>

namespace Upp {

class IdeCliConsole {
public:
	IdeCliConsole();
	void          Append(const String& s);
	int           Flush();
	int           Execute(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, bool noconvert = false);
	int           Execute(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false);
	int           AllocSlot();
	bool          Run(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1);
	bool          Run(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1);
	bool          IsRunning();
	bool          Wait();
	void          Kill();
private:
	struct Slot {
		One<AProcess> process;
		String            output;
		Stream           *outfile;
		bool              quiet;
	};
	Array<Slot> processes;
};
class IdeCliContext : public IdeContext, public MakeBuild {
public:
	IdeCliContext();
	virtual bool             IsVerbose() const override { return true; }
	virtual void             PutConsole(const char *s) override;
	virtual void             PutVerbose(const char *s) override;
	virtual void             PutLinking() override;
	virtual void             PutLinkingEnd(bool ok) override;
	virtual const Workspace& IdeWorkspace() const override;
	virtual bool             IdeIsBuilding() const override;
	virtual String           IdeGetOneFile() const override;
	virtual int              ConsoleExecute(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, bool noconvert = false) override;
	virtual int              ConsoleExecute(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false) override;
	virtual int              ConsoleExecuteWithInput(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert) override;
	virtual int              ConsoleAllocSlot() override;
	virtual bool             ConsoleRun(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1) override;
	virtual bool             ConsoleRun(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1) override;
	virtual void             ConsoleFlush() override;
	virtual void             ConsoleBeginGroup(String group) override;
	virtual void             ConsoleEndGroup() override;
	virtual bool             ConsoleWait() override;
	virtual bool             ConsoleWait(int slot) override;
	virtual bool             ConsoleGetError() override;
	virtual bool             ConsoleIsRunning() override;
	virtual void             ConsoleKill(int slot) override;
	virtual void             ConsoleKill() override;
	virtual void             ConsoleClear() override;
	virtual void             ConsoleOnFinish(Event<>  cb) override;
	virtual void             IdeProcessEvents() override;
	virtual void             IdeSetBottom(Ctrl& ctrl) override {}
	virtual void             IdeHideBottom() override {}
	virtual void             IdeRemoveBottom(Ctrl& ctrl) override {}
	virtual void             IdeSetRight(Ctrl& ctrl) override {}
	virtual void             IdeRemoveRight(Ctrl& ctrl) override {}
	virtual void             IdeHideRight() override {}
	virtual void             IdeSetFocus() override {}
	virtual void             IdeSetMain(const String& package) override {}
	virtual void             IdeEditFile(const String& path) override {}
	virtual void             IdeSetDebugPos(const String& fn, int line, const Image& img, int i) override {}
	virtual void             IdeHideDebugPos() override {}
	virtual String           IdeGetCurrentMainPackage() override;
	virtual int              IdeGetHydraThreads() override;
	virtual String           GetMethodName(const String& method) override;
	virtual void             IdeSetBar() override {}
	// MakeBuild overrides
	virtual void ConsoleShow() override {}
	virtual void ConsoleSync() override {}
	virtual void SetupDefaultMethod() override;
	virtual Vector<String> PickErrors() override;
	virtual void BeginBuilding(bool clear_console) override;
	virtual void SetErrorEditor() override;
	virtual void EndBuilding(bool ok) override;
	virtual void DoProcessEvents() override;
	virtual String GetMain() override;
	virtual bool   IsBuilding() override;
private:
	IdeCliConsole console;
	Workspace     workspace;
};
bool HandleConsoleIdeArgs(const Vector<String>& args);
String GetConsoleIdeExperimentalNotice();

} // namespace Upp

#endif
