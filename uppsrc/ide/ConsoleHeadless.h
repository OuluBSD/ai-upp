#ifndef _ide_ConsoleHeadless_h_
#define _ide_ConsoleHeadless_h_

#include <ide/Core/Core.h>
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
	void          BeginGroup(String group);
	void          EndGroup();
	bool          IsRunning();
	bool          IsRunning(int slot);
	void          Wait(int slot);
	bool          Wait();
	void          OnFinish(Event<> cb);
	void          FlushConsole();
	void          SetSlots(int count);
	void          Kill(int slot);
	void          Kill();
	void          ClearError();
	Vector<String>PickErrors();

	bool          verbosebuild = false;

private:
	struct Slot {
		Slot();

		One<AProcess> process;
		String        cmdline;
		String        output;
		String        key;
		String        group;
		Stream       *outfile = nullptr;
		bool          quiet = true;
		int           exitcode;
		int           last_msecs;
		int           serial;
	};

	struct Group {
		Group();

		int  count;
		int  start_time;
		bool finished;
		int  msecs;
		int  raw_msecs;
	};

	struct Finisher {
		int     serial = 0;
		Event<> cb;
	};

	Array<Slot>           processes;
	Array<Finisher>       finishers;
	ArrayMap<String, Group> groups;
	Vector<String>        error_keys;
	String                current_group;
	String                spooled_output;
	int                   console_lock = -1;
	bool                  wrap_text = true;
	int                   serial = 0;

	void CheckEndGroup();
};

class IdeCliContext : public IdeContext, public MakeBuild {
public:
	IdeCliContext();

	void         SetVerbose(bool b)                  { verbose = b; }
	void         SetMain(const String& m)            { main = m; }
	bool         ScanWorkspace(const Vector<String>& conf_flags);
	const Workspace& WorkspaceRef() const            { return wspc; }

	// IdeContext overrides
	bool         IsVerbose() const override;
	void         PutConsole(const char *s) override;
	void         PutVerbose(const char *s) override;
	void         PutLinking() override;
	void         PutLinkingEnd(bool ok) override;
	const Workspace& IdeWorkspace() const override;
	bool         IdeIsBuilding() const override;
	String       IdeGetOneFile() const override;
	int          IdeConsoleExecute(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, bool noconvert = false) override;
	int          IdeConsoleExecuteWithInput(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert = false) override;
	int          IdeConsoleExecute(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false) override;
	int          IdeConsoleAllocSlot() override;
	bool         IdeConsoleRun(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1) override;
	bool         IdeConsoleRun(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1) override;
	void         IdeConsoleFlush() override;
	void         IdeConsoleBeginGroup(String group) override;
	void         IdeConsoleEndGroup() override;
	bool         IdeConsoleWait() override;
	bool         IdeConsoleWait(int slot) override;
	void         IdeConsoleOnFinish(Event<> cb) override;
	void         IdeProcessEvents() override;
	bool         IdeIsDebug() const override;
	void         IdeEndDebug() override;
	void         IdeSetBottom(Ctrl& ctrl) override;
	void         IdeActivateBottom() override;
	void         IdeRemoveBottom(Ctrl& ctrl) override;
	void         IdeSetRight(Ctrl& ctrl) override;
	void         IdeRemoveRight(Ctrl& ctrl) override;
	String       IdeGetFileName() const override;
	int          IdeGetFileLine() override;
	String       IdeGetLine(int i) const override;
	void         IdeSetDebugPos(const String& fn, int line, const Image& img, int i) override;
	void         IdeHidePtr() override;
	bool         IdeDebugLock() override;
	bool         IdeDebugUnLock() override;
	bool         IdeIsDebugLock() const override;
	void         IdeSetBar() override;
	void         IdeOpenTopicFile(const String& file) override;
	void         IdeFlushFile() override;
	String       IdeGetFileName() override;
	String       IdeGetNestFolder() override;
	String       IdeGetIncludePath() override;
	bool         IsPersistentFindReplace() override;
	int          IdeGetHydraThreads() override;
	String       IdeGetCurrentBuildMethod() override;
	String       IdeGetCurrentMainPackage() override;
	void         IdePutErrorLine(const String& line) override;

	// MakeBuild hooks
	void         ConsoleShow() override;
	void         ConsoleSync() override;
	void         ConsoleClear() override;
	void         SetupDefaultMethod() override;
	Vector<String> PickErrors() override;
	void         BeginBuilding(bool clear_console) override;
	void         SetErrorEditor() override;
	void         EndBuilding(bool ok) override;
	void         DoProcessEvents() override;
	String       GetMain() override;

	// Utility
	void         Reset();

private:
	Workspace         wspc;
	IdeCliConsole     console;
	String            main;
	bool              verbose = false;
	bool              building = false;
	int               build_time = 0;
};

struct ConsoleIdeBuildArgs {
	String assembly;
	String main_package;
	String method;
	String config;
	bool   release_mode = false;
	bool   verbose = false;
	bool   use_target = false;
	String target_override;
	String one_file;
};

bool RunIdeHeadlessBuild(const ConsoleIdeBuildArgs& args);

} // namespace Upp

#endif
