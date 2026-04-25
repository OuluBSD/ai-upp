#include "ScriptIDE.h"

NAMESPACE_UPP

namespace {
int& ExternalProcessLaunchCount()
{
	static int count = 0;
	return count;
}

bool IsPythonCliBinary(const String& binary_path)
{
	String stem = ToLower(GetFileTitle(binary_path));
	return stem == "pythoncli";
}

String ResolveSiblingBinary(const String& name)
{
	String path = GetExeDirFile(name);
#ifdef PLATFORM_WIN32
	if(!FileExists(path) && FileExists(path + ".exe"))
		path += ".exe";
#endif
	return FileExists(path) ? path : String();
}

String ResolveConfiguredBinaryPath(const String& configured)
{
	if(configured.IsEmpty())
		return String();

	String path = configured;
	if(!IsFullPath(path))
		path = AppendFileName(GetFileDirectory(GetExeFilePath()), path);
#ifdef PLATFORM_WIN32
	if(!FileExists(path) && FileExists(path + ".exe"))
		path += ".exe";
#endif
	return FileExists(path) ? path : String();
}

}

bool ExternalProcessTarget::CanRun(const RunTargetContext& ctx) const
{
	return !ctx.file_path.IsEmpty();
}

void ExternalProcessTarget::SetConfig(const ExternalProcessSettings& cfg)
{
	config = cfg;
}

String ExternalProcessTarget::ResolveBinaryPath() const
{
	String configured = ResolveConfiguredBinaryPath(config.binary_path);
	if(!configured.IsEmpty())
		return configured;

	String cardide = ResolveSiblingBinary("CardIDE");
	if(!cardide.IsEmpty())
		return cardide;

	String launcher = ResolveSiblingBinary("GameLauncher");
	if(!launcher.IsEmpty())
		return launcher;

	return ResolveSiblingBinary("PythonCLI");
}

String ExternalProcessTarget::ResolvePythonCliEntry(const String& path) const
{
	String ext = ToLower(GetFileExt(path));
	if(ext != ".gamestate")
		return path;

	Value gs = ParseJSON(LoadFile(path));
	if(gs.IsVoid())
		return String();

	String entry_script = AsString(gs["entry_script"]);
	if(entry_script.IsEmpty())
		return String();

	String game_dir = GetFileDirectory(path);
	String entry_path = IsFullPath(entry_script)
		? entry_script
		: AppendFileName(game_dir, entry_script);
	return FileExists(entry_path) ? entry_path : String();
}

String ExternalProcessTarget::BuildCommandLine(const String& binary, const Vector<String>& args) const
{
	auto quote = [&](const String& a) {
		String out = "\"";
		for(int i = 0; i < a.GetCount(); i++) {
			byte c = (byte)a[i];
			if(c == '"' || c == '\\')
				out.Cat('\\');
			out.Cat(c);
		}
		out.Cat('"');
		return out;
	};

	String cmd = quote(binary);
	for(int i = 0; i < args.GetCount(); i++)
		cmd << " " << quote(args[i]);
	return cmd;
}

Vector<String> ExternalProcessTarget::ParseExtraArgs(const String& text) const
{
	Vector<String> out;
	String t = TrimBoth(text);
	if(t.IsEmpty())
		return out;

	String current;
	bool in_single = false;
	bool in_double = false;
	bool escaping = false;
	for(int i = 0; i < t.GetCount(); i++) {
		int c = t[i];
		if(escaping) {
			current.Cat(c);
			escaping = false;
			continue;
		}
		if(c == '\\' && !in_single) {
			escaping = true;
			continue;
		}
		if(c == '"' && !in_single) {
			in_double = !in_double;
			continue;
		}
		if(c == '\'' && !in_double) {
			in_single = !in_single;
			continue;
		}
		if(IsSpace(c) && !in_single && !in_double) {
			if(!current.IsEmpty()) {
				out.Add(current);
				current.Clear();
			}
			continue;
		}
		current.Cat(c);
	}
	if(escaping)
		current.Cat('\\');
	if(!current.IsEmpty())
		out.Add(current);
	if(in_single || in_double)
		LOG("ExternalProcessTarget: unmatched quote in extra args: " << text);
	return out;
}

void ExternalProcessTarget::Run(const RunTargetContext& ctx)
{
	if(!CanRun(ctx))
		return;

	String binary = ResolveBinaryPath();
	if(binary.IsEmpty()) {
		Exclamation("No external process binary found. Configure one in Run preferences.");
		return;
	}

	Vector<String> args = ParseExtraArgs(config.extra_args);
	bool python_cli = IsPythonCliBinary(binary);

	if(ctx.mode == RunMode::Debug && !python_cli)
		args.Add("--debug");

	String launch_path = python_cli ? ResolvePythonCliEntry(ctx.file_path) : ctx.file_path;
	if(launch_path.IsEmpty()) {
		Exclamation("External process target could not resolve launch script.");
		return;
	}
	args.Add(launch_path);

	String working_dir = ctx.working_dir.IsEmpty() ? GetFileDirectory(ctx.file_path) : ctx.working_dir;
	LOG("ExternalProcessTarget: launch binary=" << binary << " args=" << args.GetCount() << " wd=" << working_dir);

	if(config.show_terminal) {
		ExternalProcessLaunchCount()++;
		(new TerminalWindow())->Execute(BuildCommandLine(binary, args), working_dir);
		return;
	}

	LocalProcess p;
	if(!p.Start(binary, args, NULL, working_dir)) {
		Exclamation("Failed to start external process: " + binary);
		return;
	}
	ExternalProcessLaunchCount()++;
	if(config.wait_for_exit) {
		String out;
		int code = p.Finish(out);
		LOG("ExternalProcessTarget: process exited with code " << code);
	}
	else {
		p.Detach();
	}
}

ExternalProcessTarget* GetExternalProcessTarget()
{
	return dynamic_cast<ExternalProcessTarget*>(RunTargetRegistry::Get().Find("local.external_process"));
}

void RegisterExternalProcessTarget()
{
	static ExternalProcessTarget target;
	RunTargetRegistry::Get().Register(target);
}

int GetExternalProcessLaunchCount()
{
	return ExternalProcessLaunchCount();
}

void ResetExternalProcessLaunchCount()
{
	ExternalProcessLaunchCount() = 0;
}

END_UPP_NAMESPACE
