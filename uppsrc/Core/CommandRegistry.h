#ifndef _Core_CommandRegistry_h_
#define _Core_CommandRegistry_h_

class CoreCommandContext;

struct CoreCommandResult : Moveable<CoreCommandResult> {
	bool ok = true;
	int code = 0;
	String message;
	Value value;

	void Jsonize(JsonIO& json);
	ValueMap ToValueMap() const;
};

struct CoreCommandInfo : Moveable<CoreCommandInfo> {
	String name;
	String description;
	String category;
	Vector<String> tags;
	ValueMap args_schema;

	void Jsonize(JsonIO& json);
	ValueMap ToValueMap() const;
};

struct CoreCommand : Moveable<CoreCommand> {
	String name;
	String description;
	String category;
	Vector<String> tags;
	ValueMap args_schema;
	Function<CoreCommandResult(CoreCommandContext&)> handler;

	CoreCommandInfo GetInfo() const;
};

class CoreCommandContext {
public:
	CoreCommandContext() {}
	CoreCommandContext(const ValueMap& args) : args(args) {}

	const ValueMap& GetArgs() const { return args; }
	Value Get(const String& key, const Value& def = Null) const;
	void SetResult(const Value& value) { result_value = value; }
	Value GetResult() const { return result_value; }
	void Log(const String& line) { log.Add(line); }
	const Vector<String>& GetLog() const { return log; }

	CoreCommandResult Run(const String& name, const ValueMap& child_args = ValueMap());

private:
	ValueMap args;
	Value result_value;
	Vector<String> log;
};

class CoreCommandRegistry {
public:
	template <class T>
	static void Register(const String& name)
	{
		CoreCommand command;
		command.name = name;
		T::Setup(command);
		Register(pick(command));
	}

	static void Register(CoreCommand command);
	static const CoreCommand *Find(const String& name);
	static Vector<CoreCommandInfo> List();
	static CoreCommandResult Run(const String& name, const ValueMap& args = ValueMap());
	static ValueMap ListJson();
	static ValueMap DescribeJson(const String& name);

private:
	static Array<CoreCommand>& Commands();
};

#endif
