#include "ScriptIDE.h"

NAMESPACE_UPP

bool TerminalRunTarget::CanRun(const RunTargetContext& ctx) const
{
	return !ctx.file_path.IsEmpty();
}

void TerminalRunTarget::Run(const RunTargetContext& ctx)
{
	if(!CanRun(ctx))
		return;

	String cli = AppendFileName(GetFileDirectory(GetExeFilePath()), "PythonCLI");
	String cmdline = cli;
	if(ctx.mode == RunMode::Debug)
		cmdline << " --debug";
	cmdline << " " << ctx.file_path;

	String working_dir = ctx.working_dir.IsEmpty() ? GetFileDirectory(ctx.file_path) : ctx.working_dir;
	(new TerminalWindow())->Execute(cmdline, working_dir);
}

void RegisterTerminalRunTarget()
{
	static TerminalRunTarget target;
	RunTargetRegistry::Get().Register(target);
}

END_UPP_NAMESPACE
