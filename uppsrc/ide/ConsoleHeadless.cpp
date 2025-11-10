#include "ide.h"

#include "ConsoleHeadless.h"

#include <Core/LocalProcess.h>

namespace Upp {

namespace {

void HeadlessProcessEvents() {}

}

extern bool SilentMode;

IdeCliConsole::Slot::Slot()
{
	outfile = NULL;
	quiet = true;
	exitcode = Null;
	last_msecs = 0;
	serial = 0;
}

IdeCliConsole::Group::Group()
{
	count = 0;
	start_time = msecs();
	finished = false;
	msecs = 0;
	raw_msecs = 0;
}

IdeCliConsole::IdeCliConsole()
{
	processes.SetCount(1);
	console_lock = -1;
	wrap_text = true;
	serial = 0;
}

void IdeCliConsole::Append(const String& s)
{
	if(!SilentMode)
		Cout() << s;
}

void IdeCliConsole::CheckEndGroup()
{
	while(groups.GetCount() && groups[0].finished && groups[0].count == 0) {
		String name = groups.GetKey(0);
		Group  g    = groups[0];
		groups.Remove(0);
		if(!name.IsEmpty()) {
			String msg;
			msg << name << " finished in " << PrintTime(g.msecs) << '\n';
			Append(msg);
		}
	}
}

void IdeCliConsole::FlushConsole()
{
	if(console_lock < 0) {
		Append(spooled_output);
		spooled_output = Null;
	}
}

int IdeCliConsole::Flush()
{
	bool done_output = false;
	int  num_running = 0;
	for(int i = 0; i < processes.GetCount(); i++)
		if(!!processes[i].process)
			num_running++;
	if(num_running) {
		int time = msecs();
		for(int i = 0; i < processes.GetCount(); i++) {
			Slot& slot = processes[i];
			if(!slot.process)
				continue;
			Group& group = groups.GetAdd(slot.group);
			group.msecs += (time - slot.last_msecs) / num_running;
			group.raw_msecs += time - slot.last_msecs;
			slot.last_msecs = time;
		}
	}
	bool running = false;
	for(int i = 0; i < processes.GetCount(); i++) {
		Slot& slot = processes[i];
		if(!slot.process)
			continue;
		String s;
		slot.process->Read(s);
		if(!IsNull(s)) {
			done_output = true;
			if(slot.outfile)
				slot.outfile->Put(s);
			if(!slot.quiet) {
				if(console_lock < 0 || console_lock == i) {
					console_lock = i;
					Append(s);
				}
				else
					slot.output.Cat(s);
			}
		}
		if(!slot.process->IsRunning()) {
			Kill(i);
			if(slot.exitcode != 0 && verbosebuild)
				spooled_output.Cat("Error executing " + slot.cmdline + '\n');
			if(console_lock == i)
				console_lock = -1;
			FlushConsole();
			CheckEndGroup();
			continue;
		}
		running = true;
	}
	return !running ? -1 : done_output ? 1 : 0;
}

int IdeCliConsole::Execute(One<AProcess> process, const char *cmdline, Stream *out, bool quiet)
{
	Wait();
	if(!Run(pick(process), cmdline, out, quiet, 0))
		return -1;
	Wait();
	return processes[0].exitcode;
}

int IdeCliConsole::Execute(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool)
{
	try {
		Wait();
		One<AProcess> proc;
		if(proc.Create<LocalProcess>().Start(cmdline, envptr))
			return Execute(pick(proc), cmdline, out, quiet);
	}
	catch(...) {
	}
	HeadlessProcessEvents();
	return Null;
}

int IdeCliConsole::AllocSlot()
{
	int sleep_time = 0;
	for(;;) {
		for(int i = 0; i < processes.GetCount(); i++)
			if(!IsRunning(i))
				return i;
		switch(Flush()) {
		case -1:
			break;
		case 0:
			sleep_time = min(sleep_time + 5, 20);
			break;
		case 1:
			sleep_time = 0;
			break;
		}
		Sleep(sleep_time);
		HeadlessProcessEvents();
	}
}

bool IdeCliConsole::Run(const char *cmdline, Stream *out, const char *envptr, bool quiet, int slot, String key, int blitz_count)
{
	try {
		Wait(slot);
		One<AProcess> proc;
		if(proc.Create<LocalProcess>().Start(cmdline, envptr))
			return Run(pick(proc), cmdline, out, quiet, slot, key, blitz_count);
	}
	catch(Exc e) {
		Append(e);
	}
	catch(...) {
	}
	HeadlessProcessEvents();
	return false;
}

bool IdeCliConsole::Run(One<AProcess> process, const char *cmdline, Stream *out, bool quiet, int slot, String key, int blitz_count)
{
	if(!process) {
		if(verbosebuild)
			spooled_output << "Error running " << cmdline << '\n';
		FlushConsole();
		return false;
	}
	else if(verbosebuild)
		spooled_output << cmdline << '\n';
	Wait(slot);
	Slot& pslot = processes[slot];
	pslot.process = pick(process);
	pslot.cmdline = cmdline;
	pslot.outfile = out;
	pslot.output = Null;
	pslot.quiet = quiet;
	pslot.key = key;
	pslot.group = current_group;
	pslot.last_msecs = msecs();
	pslot.serial = ++serial;
	groups.GetAdd(pslot.group).count += blitz_count;
	if(processes.GetCount() == 1)
		Wait(slot);
	return true;
}

void IdeCliConsole::BeginGroup(String group)
{
	Flush();
	groups.GetAdd(current_group).finished = true;
	groups.GetAdd(current_group = group);
	CheckEndGroup();
}

void IdeCliConsole::EndGroup()
{
	groups.GetAdd(current_group).finished = true;
	CheckEndGroup();
	current_group = Null;
}

bool IdeCliConsole::IsRunning()
{
	for(int i = 0; i < processes.GetCount(); i++)
		if(IsRunning(i))
			return true;
	return false;
}

bool IdeCliConsole::IsRunning(int slot)
{
	if(slot < 0 || slot >= processes.GetCount() || !processes[slot].process)
		return false;
	return processes[slot].process->IsRunning();
}

void IdeCliConsole::Wait(int slot)
{
	int sleep_time = 0;
	while(slot >= 0 && slot < processes.GetCount() && processes[slot].process) {
		HeadlessProcessEvents();
		switch(Flush()) {
		case -1: return;
		case  0: sleep_time = min(sleep_time + 5, 20); break;
		case +1: sleep_time = 0; break;
		}
		Sleep(sleep_time);
	}
}

bool IdeCliConsole::Wait()
{
	while(IsRunning()) {
		HeadlessProcessEvents();
		if(Flush() < 0)
			break;
		Sleep(10);
	}
	return !IsRunning();
}

void IdeCliConsole::OnFinish(Event<> cb)
{
	if(!cb)
		return;
	Finisher& f = finishers.Add();
	f.serial = serial;
	f.cb = cb;
}

void IdeCliConsole::SetSlots(int count)
{
	processes.SetCount(count);
}

void IdeCliConsole::Kill(int slot)
{
	if(slot < 0 || slot >= processes.GetCount())
		return;
	Slot& s = processes[slot];
	if(!s.process)
		return;
	String h;
	s.process->Read(h);
	if(!h.IsEmpty())
		if(s.outfile)
			s.outfile->Put(h);
		else
			s.output.Cat(h);
	s.process->Kill();
	s.exitcode = s.process->GetExitCode();
	processes[slot].process.Clear();
	groups.GetAdd(s.group).count--;
	if(console_lock == slot)
		console_lock = -1;
	FlushConsole();
	CheckEndGroup();
}

void IdeCliConsole::Kill()
{
	for(int i = 0; i < processes.GetCount(); i++)
		Kill(i);
}

void IdeCliConsole::ClearError()
{
	error_keys.Clear();
}

Vector<String> IdeCliConsole::PickErrors()
{
	Vector<String> errs = pick(error_keys);
	error_keys.Clear();
	return errs;
}

IdeCliContext::IdeCliContext()
{
	verbose = false;
	build_time = 0;
	building = false;
}

bool IdeCliContext::ScanWorkspace(const Vector<String>& conf_flags)
{
	wspc.Clear();
	try {
		if(main.IsEmpty())
			return false;
		wspc.Scan(main, conf_flags);
	}
	catch(...) {
		return false;
	}
	return wspc.GetCount() > 0;
}

void IdeCliContext::Reset()
{
	console.Kill();
	console.ClearError();
}

bool IdeCliContext::IsVerbose() const
{
	return verbose;
}

void IdeCliContext::PutConsole(const char *s)
{
	console.Append(String(s) + '\n');
}

void IdeCliContext::PutVerbose(const char *s)
{
	if(console.verbosebuild)
		PutConsole(s);
}

void IdeCliContext::PutLinking()
{
	PutConsole("Linking...");
}

void IdeCliContext::PutLinkingEnd(bool ok)
{
	PutConsole(ok ? "Linking done." : "Linking failed.");
}

const Workspace& IdeCliContext::IdeWorkspace() const
{
	return wspc;
}

bool IdeCliContext::IdeIsBuilding() const
{
	return building;
}

String IdeCliContext::IdeGetOneFile() const
{
	return onefile;
}

int IdeCliContext::IdeConsoleExecute(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert)
{
	return console.Execute(cmdline, out, envptr, quiet, noconvert);
}

int IdeCliContext::IdeConsoleExecuteWithInput(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool)
{
	return console.Execute(cmdline, out, envptr, quiet);
}

int IdeCliContext::IdeConsoleExecute(One<AProcess> process, const char *cmdline, Stream *out, bool quiet)
{
	return console.Execute(pick(process), cmdline, out, quiet);
}

int IdeCliContext::IdeConsoleAllocSlot()
{
	return console.AllocSlot();
}

bool IdeCliContext::IdeConsoleRun(const char *cmdline, Stream *out, const char *envptr, bool quiet, int slot, String key, int blitz_count)
{
	return console.Run(cmdline, out, envptr, quiet, slot, key, blitz_count);
}

bool IdeCliContext::IdeConsoleRun(One<AProcess> process, const char *cmdline, Stream *out, bool quiet, int slot, String key, int blitz_count)
{
	return console.Run(pick(process), cmdline, out, quiet, slot, key, blitz_count);
}

void IdeCliContext::IdeConsoleFlush()
{
	console.FlushConsole();
}

void IdeCliContext::IdeConsoleBeginGroup(String group)
{
	console.BeginGroup(group);
}

void IdeCliContext::IdeConsoleEndGroup()
{
	console.EndGroup();
}

bool IdeCliContext::IdeConsoleWait()
{
	return console.Wait();
}

bool IdeCliContext::IdeConsoleWait(int slot)
{
	console.Wait(slot);
	return true;
}

void IdeCliContext::IdeConsoleOnFinish(Event<> cb)
{
	console.OnFinish(cb);
}

void IdeCliContext::IdeProcessEvents()
{
}

bool IdeCliContext::IdeIsDebug() const
{
	return false;
}

void IdeCliContext::IdeEndDebug()
{
}

void IdeCliContext::IdeSetBottom(Ctrl&)
{
}

void IdeCliContext::IdeActivateBottom()
{
}

void IdeCliContext::IdeRemoveBottom(Ctrl&)
{
}

void IdeCliContext::IdeSetRight(Ctrl&)
{
}

void IdeCliContext::IdeRemoveRight(Ctrl&)
{
}

String IdeCliContext::IdeGetFileName() const
{
	return Null;
}

int IdeCliContext::IdeGetFileLine()
{
	return 0;
}

String IdeCliContext::IdeGetLine(int) const
{
	return Null;
}

void IdeCliContext::IdeSetDebugPos(const String&, int, const Image&, int)
{
}

void IdeCliContext::IdeHidePtr()
{
}

bool IdeCliContext::IdeDebugLock()
{
	return false;
}

bool IdeCliContext::IdeDebugUnLock()
{
	return false;
}

bool IdeCliContext::IdeIsDebugLock() const
{
	return false;
}

void IdeCliContext::IdeSetBar()
{
}

void IdeCliContext::IdeOpenTopicFile(const String&)
{
}

void IdeCliContext::IdeFlushFile()
{
}

String IdeCliContext::IdeGetFileName()
{
	return Null;
}

String IdeCliContext::IdeGetNestFolder()
{
	return Null;
}

String IdeCliContext::IdeGetIncludePath()
{
	return Null;
}

bool IdeCliContext::IsPersistentFindReplace()
{
	return false;
}

int IdeCliContext::IdeGetHydraThreads()
{
	return CPU_Cores();
}

String IdeCliContext::IdeGetCurrentBuildMethod()
{
	return method;
}

String IdeCliContext::IdeGetCurrentMainPackage()
{
	return main;
}

void IdeCliContext::IdePutErrorLine(const String& line)
{
	PutConsole(line);
}

void IdeCliContext::ConsoleShow()
{
}

void IdeCliContext::ConsoleSync()
{
}

void IdeCliContext::ConsoleClear()
{
	console.ClearError();
}

Vector<String> IdeCliContext::PickErrors()
{
	return console.PickErrors();
}

void IdeCliContext::BeginBuilding(bool clear_console)
{
	building = true;
	if(clear_console)
		console.ClearError();
	SetupDefaultMethod();
	SetHdependDirs();
	HdependTimeDirty();
	build_time = msecs();
}

void IdeCliContext::SetErrorEditor()
{
}

void IdeCliContext::EndBuilding(bool ok)
{
	building = false;
	console.EndGroup();
	console.Wait();
	Vector<String> errors = console.PickErrors();
	for(const String& path : errors)
		DeleteFile(path);
	if(!errors.IsEmpty())
		ok = false;
	PutConsole("");
	PutConsole((ok ? "OK. " : "There were errors. ") + GetPrintTime(build_time));
}

void IdeCliContext::DoProcessEvents()
{
}

String IdeCliContext::GetMain()
{
	return main;
}

void IdeCliContext::SetupDefaultMethod()
{
	if(!IsNull(method))
		return;
	String default_method = GetDefaultMethod();
	if(!IsNull(default_method))
		method = default_method;
	else
	if(FileExists(ConfigFile("CLANG.bm")))
		method = "CLANG";
	else
	if(FileExists(ConfigFile("GCC.bm")))
		method = "GCC";
	else {
		FindFile ff(ConfigFile("*.bm"));
		if(ff)
			method = GetFileTitle(ff.GetName());
	}
	if(IsNull(method))
		return;
	VectorMap<String, String> map = GetMethodVars(method);
	debug.linkmode = atoi(map.Get("DEBUG_LINKMODE", "0"));
	debug.def.debug = atoi(map.Get("DEBUG_INFO", "0"));
	debug.def.blitz = MapFlag(map, "DEBUG_BLITZ");
	release.linkmode = atoi(map.Get("RELEASE_LINKMODE", "0"));
	release.def.debug = 0;
	release.def.blitz = MapFlag(map, "RELEASE_BLITZ");
}

bool RunIdeHeadlessBuild(const ConsoleIdeBuildArgs& args)
{
	if(IsNull(args.main_package)) {
		Cerr() << "theide build: missing --main argument.\n";
		SetExitCode(1);
		return true;
	}
	String assembly = args.assembly;
	if(!LoadVars(assembly.IsEmpty() ? "default" : assembly)) {
		Cerr() << "theide build: unable to load assembly '" << (assembly.IsEmpty() ? String("default") : assembly) << "'.\n";
		SetExitCode(1);
		return true;
	}
	IdeCliContext ctx;
	ctx.SetVerbose(args.verbose);
	ctx.mainconfigparam = args.config;
	ctx.method = args.method;
	ctx.targetmode = args.release_mode ? 1 : 0;
	ctx.use_target = args.use_target;
	ctx.target = args.target_override;
	ctx.onefile = args.one_file;
	ctx.SetMain(args.main_package);
	Vector<String> flags = SplitFlags(~ctx.mainconfigparam, false);
	if(!ctx.ScanWorkspace(flags)) {
		Cerr() << "theide build: unable to load main package '" << args.main_package << "'.\n";
		SetExitCode(1);
		return true;
	}
	SetTheIde(&ctx);
	bool ok = ctx.Build();
	SetTheIde(nullptr);
	SetExitCode(ok ? 0 : 1);
	return true;
}

} // namespace Upp

