#include "CommandRegistry.h"

using namespace Upp;

static void Usage()
{
	Cout() << "Usage:\n"
	       << "  CommandRegistry --list [--json]\n"
	       << "  CommandRegistry --describe <name> [--json]\n"
	       << "  CommandRegistry --run <name> [--args-json <json>] [--arg key=value]... [--json]\n"
	       << "Examples:\n"
	       << "  CommandRegistry --list\n"
	       << "  CommandRegistry --describe sample.echo --json\n"
	       << "  CommandRegistry --run sample.pipeline --arg text=hello --arg a=4 --arg b=5 --json\n";
}

static ValueMap ParseArgsJson(const String& json)
{
	if(json.IsEmpty())
		return ValueMap();
	Value value = ParseJSON(json);
	if(value.IsError() || !value.Is<ValueMap>())
		return ValueMap();
	return ValueMap(value);
}

static void ApplyKeyValueArg(ValueMap& out, const String& key_value)
{
	int eq = key_value.Find('=');
	if(eq < 0)
		return;
	String key = key_value.Left(eq);
	String value = key_value.Mid(eq + 1);
	char *end = NULL;
	long number = strtol(value, &end, 10);
	Value parsed;
	if(end && *end == '\0')
		parsed = (int)number;
	else if(value == "true" || value == "false")
		parsed = value == "true";
	else
		parsed = value;
	int existing = out.Find(key);
	if(existing >= 0)
		out.SetAt(existing, parsed);
	else
		out.Add(key, parsed);
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	const Vector<String>& args = CommandLine();
	bool json = false;
	String mode;
	String name;
	String args_json;
	Vector<String> key_value_args;

	for(int i = 0; i < args.GetCount(); i++) {
		String arg = args[i];
		if(arg == "--json")
			json = true;
		else if(arg == "--list")
			mode = "list";
		else if(arg == "--describe") {
			mode = "describe";
			if(i + 1 < args.GetCount())
				name = args[++i];
		}
		else if(arg == "--run") {
			mode = "run";
			if(i + 1 < args.GetCount())
				name = args[++i];
		}
		else if(arg == "--args-json") {
			if(i + 1 < args.GetCount())
				args_json = args[++i];
		}
		else if(arg == "--arg") {
			if(i + 1 < args.GetCount())
				key_value_args.Add(args[++i]);
		}
		else if(arg == "--help") {
			Usage();
			return;
		}
	}

	if(mode == "list") {
		if(json) {
			Cout() << AsJSON(CoreCommandRegistry::ListJson(), true) << "\n";
			return;
		}
		for(const CoreCommandInfo& info : CoreCommandRegistry::List())
			Cout() << info.name << "\t" << info.category << "\t" << info.description << "\n";
		return;
	}

	if(mode == "describe") {
		if(name.IsEmpty()) {
			Cerr() << "--describe requires a command name\n";
			SetExitCode(2);
			return;
		}
		ValueMap out = CoreCommandRegistry::DescribeJson(name);
		if(json) {
			Cout() << AsJSON(out, true) << "\n";
			return;
		}
		if(!(bool)out.Get("ok", false)) {
			Cerr() << out.Get("error", "unknown error") << "\n";
			SetExitCode(1);
			return;
		}
		CoreCommandInfo info;
		LoadFromJson(info, AsJSON(out.Get("command", Value())));
		Cout() << info.name << "\n" << info.description << "\n";
		return;
	}

	if(mode == "run") {
		if(name.IsEmpty()) {
			Cerr() << "--run requires a command name\n";
			SetExitCode(2);
			return;
		}
		ValueMap parsed_args = ParseArgsJson(args_json);
		for(const String& key_value : key_value_args)
			ApplyKeyValueArg(parsed_args, key_value);
		CoreCommandResult result = CoreCommandRegistry::Run(name, parsed_args);
		if(json) {
			Cout() << AsJSON(result.ToValueMap(), true) << "\n";
		}
		else {
			Cout() << (result.ok ? "OK" : "FAIL") << " code=" << result.code << "\n";
			if(!result.message.IsEmpty())
				Cout() << result.message << "\n";
			if(!IsNull(result.value))
				Cout() << AsJSON(result.value, true) << "\n";
		}
		SetExitCode(result.code);
		return;
	}

	Usage();
	SetExitCode(2);
}
