#include "Core.h"
#include "ConsoleHost.h"

#include <Core/LocalProcess.h>

IdeCoreConsoleHost::IdeCoreConsoleHost()
{
}

int IdeCoreConsoleHost::Execute(const char *cmdline, String *output, const char *envptr, bool quiet)
{
	if(!Run(cmdline, envptr))
		return -1;
	int code = Wait(output);
	if(!quiet)
		Cout() << buffer;
	return code;
}

int IdeCoreConsoleHost::Execute(const char *cmdline, Event<String> on_output, const char *envptr, bool quiet)
{
	if(!Run(cmdline, envptr))
		return -1;
	while(IsRunning()) {
		String s = process->Get();
		if(!s.IsEmpty()) {
			buffer << s;
			on_output(s);
			if(!quiet)
				Cout() << s;
		}
		Sleep(10);
	}
	if(process) {
		String s = process->Get();
		if(!s.IsEmpty()) {
			buffer << s;
			on_output(s);
			if(!quiet)
				Cout() << s;
		}
		exit_code = process->GetExitCode();
		process.Clear();
	}
	return exit_code;
}

bool IdeCoreConsoleHost::Run(const char *cmdline, const char *envptr)
{
	if(IsRunning())
		return false;
	process.Create<LocalProcess>();
	LocalProcess *lp = dynamic_cast<LocalProcess *>(process.Get());
	if(!lp || !lp->Start(cmdline, envptr)) {
		process.Clear();
		exit_code = -1;
		return false;
	}
	buffer.Clear();
	exit_code = -1;
	return true;
}

bool IdeCoreConsoleHost::IsRunning() const
{
	const AProcess *p = process.Get();
	return p && const_cast<AProcess *>(p)->IsRunning();
}

int IdeCoreConsoleHost::Wait(String *output)
{
	while(IsRunning()) {
		String s = process->Get();
		if(!s.IsEmpty())
			buffer << s;
		Sleep(10);
	}

	if(process) {
		String s = process->Get();
		if(!s.IsEmpty())
			buffer << s;
		exit_code = process->GetExitCode();
		process.Clear();
	}

	if(output) {
		output->Clear();
		*output = buffer;
	}

	return exit_code;
}

void IdeCoreConsoleHost::Kill()
{
	if(process)
		process->Kill();
}
