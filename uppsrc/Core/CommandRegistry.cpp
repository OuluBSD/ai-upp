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

} // namespace Upp
