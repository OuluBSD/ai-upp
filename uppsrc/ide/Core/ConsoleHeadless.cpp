#include "Core.h"
#include "ConsoleHeadless.h"
#include <Core/LocalProcess.h>

namespace Upp {

IdeCliConsole::IdeCliConsole()
{
}

void IdeCliConsole::Append(const String& s)
{
	Cout() << s;
}

int IdeCliConsole::Flush()
{
	return 0;
}

int IdeCliConsole::Execute(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert)
{
	LocalProcess p;
	if(!p.Start(cmdline, envptr))
		return -1;
	String s;
	while(p.IsRunning()) {
		s = p.Get();
		if(out)
			out->Put(s);
		if(!quiet)
			Cout() << s;
	}
	return p.GetExitCode();
}

int IdeCliConsole::Execute(One<AProcess> process, const char *cmdline, Stream *out, bool quiet)
{
	LocalProcess *lp = dynamic_cast<LocalProcess *>(process.Get());
	if(!lp || !lp->Start(cmdline))
		return -1;
	String s;
	while(lp->IsRunning()) {
		s = lp->Get();
		if(out)
			out->Put(s);
		if(!quiet)
			Cout() << s;
	}
	return lp->GetExitCode();
}

int IdeCliConsole::AllocSlot()
{
	return processes.GetCount();
}

bool IdeCliConsole::Run(const char *cmdline, Stream *out, const char *envptr, bool quiet, int slot, String key, int blitz_count)
{
	Slot& s = processes.At(slot);
	s.process.Create<LocalProcess>();
	s.outfile = out;
	s.quiet = quiet;
	LocalProcess *lp = dynamic_cast<LocalProcess *>(s.process.Get());
	return lp && lp->Start(cmdline, envptr);
}

bool IdeCliConsole::Run(One<AProcess> process, const char *cmdline, Stream *out, bool quiet, int slot, String key, int blitz_count)
{
	Slot& s = processes.At(slot);
	s.process = pick(process);
	s.outfile = out;
	s.quiet = quiet;
	LocalProcess *lp = dynamic_cast<LocalProcess *>(s.process.Get());
	return lp && lp->Start(cmdline);
}

bool IdeCliConsole::IsRunning()
{
	for(int i = 0; i < processes.GetCount(); i++)
		if(processes[i].process && processes[i].process->IsRunning())
			return true;
	return false;
}

bool IdeCliConsole::Wait()
{
	while(IsRunning()) {
		for(int i = 0; i < processes.GetCount(); i++) {
			if(processes[i].process && processes[i].process->IsRunning()) {
				String s = processes[i].process->Get();
				if(processes[i].outfile)
					processes[i].outfile->Put(s);
				if(!processes[i].quiet)
					Cout() << s;
			}
		}
		Sleep(10);
	}
	return true;
}

void IdeCliConsole::Kill()
{
	for(int i = 0; i < processes.GetCount(); i++)
		if(processes[i].process)
			processes[i].process->Kill();
}

IdeCliContext::IdeCliContext()
{
}

void IdeCliContext::PutConsole(const char *s) { console.Append(s); }
void IdeCliContext::PutVerbose(const char *s) { if(IsVerbose()) console.Append(s); }
void IdeCliContext::PutLinking() { PutConsole("Linking...\n"); }
void IdeCliContext::PutLinkingEnd(bool ok) { PutConsole(ok ? "OK\n" : "FAILED\n"); }
const Workspace& IdeCliContext::IdeWorkspace() const { return workspace; }
bool IdeCliContext::IdeIsBuilding() const { return false; }
String IdeCliContext::IdeGetOneFile() const { return Null; }
int IdeCliContext::IdeConsoleExecute(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert)
{
	return console.Execute(cmdline, out, envptr, quiet, noconvert);
}
int IdeCliContext::IdeConsoleExecute(One<AProcess> process, const char *cmdline, Stream *out, bool quiet)
{
	return console.Execute(pick(process), cmdline, out, quiet);
}
int IdeCliContext::IdeConsoleExecuteWithInput(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert)
{
	return console.Execute(cmdline, out, envptr, quiet, noconvert);
}
int IdeCliContext::IdeConsoleAllocSlot() { return console.AllocSlot(); }
bool IdeCliContext::IdeConsoleRun(const char *cmdline, Stream *out, const char *envptr, bool quiet, int slot, String key, int blitz_count)
{
	return console.Run(cmdline, out, envptr, quiet, slot, key, blitz_count);
}
bool IdeCliContext::IdeConsoleRun(One<AProcess> process, const char *cmdline, Stream *out, bool quiet, int slot, String key, int blitz_count)
{
	return console.Run(pick(process), cmdline, out, quiet, slot, key, blitz_count);
}
void IdeCliContext::IdeConsoleFlush() { console.Flush(); }
void IdeCliContext::IdeConsoleBeginGroup(String group) {}
void IdeCliContext::IdeConsoleEndGroup() {}
bool IdeCliContext::IdeConsoleWait() { return console.Wait(); }
bool IdeCliContext::IdeConsoleWait(int slot) { return console.Wait(); }
bool IdeCliContext::IdeConsoleGetError() { return false; }
void IdeCliContext::IdeConsoleOnFinish(Event<> cb) {}
void IdeCliContext::IdeProcessEvents() {}
int IdeCliContext::IdeGetHydraThreads() { return 1; }
String IdeCliContext::IdeGetCurrentMainPackage() { return Null; }

bool HandleConsoleIdeArgs(const Vector<String>& args)
{
	if(args.GetCount() > 0 && args[0] == "--help") {
		Cout() << "Usage: ide [options]\n";
		return true;
	}
	return false;
}

String GetConsoleIdeExperimentalNotice()
{
	return "Console IDE is experimental.";
}

} // namespace Upp
