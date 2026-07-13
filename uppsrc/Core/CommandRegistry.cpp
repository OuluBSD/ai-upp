#include "Core.h"

namespace Upp {

void CoreCommandResult::Jsonize(JsonIO& json)
{
	json
		("ok", ok)
		("code", code)
		("message", message)
		("value", value)
	;
}

ValueMap CoreCommandResult::ToValueMap() const
{
	ValueMap out;
	out.Add("ok", ok);
	out.Add("code", code);
	out.Add("message", message);
	out.Add("value", value);
	return out;
}

void CoreCommandInfo::Jsonize(JsonIO& json)
{
	json
		("name", name)
		("description", description)
		("category", category)
		("tags", tags)
		("args_schema", args_schema)
	;
}

ValueMap CoreCommandInfo::ToValueMap() const
{
	ValueMap out;
	out.Add("name", name);
	out.Add("description", description);
	out.Add("category", category);
	ValueArray tag_values;
	for(const String& tag : tags)
		tag_values.Add(tag);
	out.Add("tags", tag_values);
	out.Add("args_schema", args_schema);
	return out;
}

CoreCommandInfo CoreCommand::GetInfo() const
{
	CoreCommandInfo info;
	info.name = name;
	info.description = description;
	info.category = category;
	info.tags = clone(tags);
	info.args_schema = clone(args_schema);
	return info;
}

Value CoreCommandContext::Get(const String& key, const Value& def) const
{
	int index = args.Find(key);
	return index >= 0 ? args.GetValue(index) : def;
}

CoreCommandResult CoreCommandContext::Run(const String& name, const ValueMap& child_args)
{
	CoreCommandResult result = CoreCommandRegistry::Run(name, child_args);
	String prefix = name + ": ";
	if(!result.message.IsEmpty()) {
		Vector<String> lines = Split(result.message, '\n', false);
		for(const String& line : lines)
			if(!line.IsEmpty())
				Log(prefix + line);
	}
	return result;
}

Array<CoreCommand>& CoreCommandRegistry::Commands()
{
	static Array<CoreCommand> commands;
	return commands;
}

void CoreCommandRegistry::Register(CoreCommand command)
{
	ASSERT(!command.name.IsEmpty());
	ASSERT(command.handler);
	for(const CoreCommand& existing : Commands())
		ASSERT_(existing.name != command.name, "Duplicate command registration: " + command.name);
	Commands().Add(pick(command));
}

const CoreCommand *CoreCommandRegistry::Find(const String& name)
{
	for(const CoreCommand& command : Commands())
		if(command.name == name)
			return &command;
	return NULL;
}

Vector<CoreCommandInfo> CoreCommandRegistry::List()
{
	Vector<CoreCommandInfo> out;
	for(const CoreCommand& command : Commands())
		out.Add(command.GetInfo());
	Sort(out, [](const CoreCommandInfo& a, const CoreCommandInfo& b) { return a.name < b.name; });
	return out;
}

CoreCommandResult CoreCommandRegistry::Run(const String& name, const ValueMap& args)
{
	CoreCommandResult result;
	const CoreCommand *command = Find(name);
	if(!command) {
		result.ok = false;
		result.code = 127;
		result.message = "Command not found: " + name;
		return result;
	}
	CoreCommandContext context(args);
	result = command->handler(context);
	if(IsNull(result.value) && !IsNull(context.GetResult()))
		result.value = context.GetResult();
	return result;
}

ValueMap CoreCommandRegistry::ListJson()
{
	ValueArray items;
	for(CoreCommandInfo& info : List())
		items.Add(info.ToValueMap());
	ValueMap out;
	out.Add("commands", items);
	return out;
}

ValueMap CoreCommandRegistry::DescribeJson(const String& name)
{
	ValueMap out;
	const CoreCommand *command = Find(name);
	if(!command) {
		out.Add("ok", false);
		out.Add("error", "Command not found: " + name);
		return out;
	}
	out.Add("ok", true);
	out.Add("command", command->GetInfo().ToValueMap());
	return out;
}

static void CoreCommandRegistryUsage()
{
	Cout() << "Usage:\n"
	       << "  <app> --list [--json]\n"
	       << "  <app> --describe <name> [--json]\n"
	       << "  <app> --run <name> [--args-json <json>] [--arg key=value]... [--json]\n"
	       << "Examples:\n"
	       << "  <app> --list\n"
	       << "  <app> --describe sample.echo --json\n"
	       << "  <app> --run sample.pipeline --arg text=hello --arg a=4 --arg b=5 --json\n";
}

static ValueMap ParseCoreCommandArgsJson(const String& json)
{
	if(json.IsEmpty())
		return ValueMap();
	Value value = ParseJSON(json);
	if(value.IsError() || !value.Is<ValueMap>())
		return ValueMap();
	return ValueMap(value);
}

static bool CoreCommandValueContainsExtraAssignment(const String& value)
{
	for(int i = 0; i < value.GetCount(); i++) {
		if((byte)value[i] > ' ')
			continue;
		int j = i + 1;
		while(j < value.GetCount() && (byte)value[j] <= ' ')
			j++;
		int begin = j;
		while(j < value.GetCount() && (byte)value[j] > ' ')
			j++;
		if(begin < j && value.Mid(begin, j - begin).Find('=') >= 0)
			return true;
		i = j;
	}
	return false;
}

static void CoreCommandArgError(bool json, const String& message)
{
	if(json) {
		CoreCommandResult result;
		result.ok = false;
		result.code = 2;
		result.message = message;
		Cout() << AsJSON(result.ToValueMap(), true) << "\n";
	}
	else
		Cerr() << message << "\n";
	SetExitCode(2);
}

static bool ApplyCoreCommandKeyValueArg(ValueMap& out, const String& key_value, String& error)
{
	int eq = key_value.Find('=');
	if(eq < 0) {
		error = "Malformed --arg value, expected key=value: " + key_value;
		return false;
	}
	String key = key_value.Left(eq);
	String value = key_value.Mid(eq + 1);
	if(key.IsEmpty()) {
		error = "Malformed --arg value, key is empty: " + key_value;
		return false;
	}
	if(CoreCommandValueContainsExtraAssignment(value)) {
		error = "Malformed --arg value contains multiple key=value pairs; pass one --arg per pair: " + key_value;
		return false;
	}
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
	return true;
}

void CoreCommandRegistryMain()
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
			CoreCommandRegistryUsage();
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
		ValueMap parsed_args = ParseCoreCommandArgsJson(args_json);
		for(const String& key_value : key_value_args) {
			String error;
			if(!ApplyCoreCommandKeyValueArg(parsed_args, key_value, error)) {
				CoreCommandArgError(json, error);
				return;
			}
		}
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

	CoreCommandRegistryUsage();
	SetExitCode(2);
}

} // namespace Upp
