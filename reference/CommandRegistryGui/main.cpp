#include "CommandRegistryGui.h"

namespace Upp {

static String JsonOrRaw(const RegistryProcessResult& result)
{
	if(!result.json.IsVoid() && !result.json.IsError())
		return AsJSON(result.json, true);
	return result.stdout_text;
}

RegistryProcessResult CommandRegistryClient::Execute(const Vector<String>& args) const
{
	RegistryProcessResult result;
	if(app_path.IsEmpty()) {
		result.error = "Target command application path is empty";
		return result;
	}
	if(!FileExists(app_path)) {
		result.error = "Target command application not found: " + app_path;
		return result;
	}
	String out;
	result.exit_code = Sys(app_path, args, out);
	result.stdout_text = out;
	if(result.exit_code != 0) {
		result.error = Format("Target command application exited with code %d", result.exit_code);
		return result;
	}
	Value parsed = ParseJSON(out);
	if(parsed.IsError()) {
		result.error = "Target command application did not return valid JSON";
		return result;
	}
	result.json = parsed;
	result.ok = true;
	return result;
}

RegistryProcessResult CommandRegistryClient::List(ValueArray& commands) const
{
	Vector<String> args;
	args << "--list" << "--json";
	RegistryProcessResult result = Execute(args);
	if(!result.ok)
		return result;
	if(!result.json.Is<ValueMap>()) {
		result.ok = false;
		result.error = "List response is not a JSON object";
		return result;
	}
	ValueMap root(result.json);
	Value items = root.Get("commands", ValueArray());
	if(!items.Is<ValueArray>()) {
		result.ok = false;
		result.error = "List response does not contain commands array";
		return result;
	}
	commands = ValueArray(items);
	return result;
}

RegistryProcessResult CommandRegistryClient::Describe(const String& name, ValueMap& command) const
{
	Vector<String> args;
	args << "--describe" << name << "--json";
	RegistryProcessResult result = Execute(args);
	if(!result.ok)
		return result;
	if(!result.json.Is<ValueMap>()) {
		result.ok = false;
		result.error = "Describe response is not a JSON object";
		return result;
	}
	ValueMap root(result.json);
	Value command_value = root.Get("command", Value());
	if(!command_value.Is<ValueMap>()) {
		result.ok = false;
		result.error = "Describe response does not contain command object";
		return result;
	}
	command = ValueMap(command_value);
	return result;
}

RegistryProcessResult CommandRegistryClient::Run(const String& name, const Vector<String>& key_value_args) const
{
	Vector<String> args;
	args << "--run" << name << "--json";
	for(const String& key_value : key_value_args)
		if(!TrimBoth(key_value).IsEmpty())
			args << "--arg" << TrimBoth(key_value);
	return Execute(args);
}

static bool CommandArrayContains(const ValueArray& commands, const String& name)
{
	for(int i = 0; i < commands.GetCount(); i++) {
		Value item_value = commands[i];
		if(!item_value.Is<ValueMap>())
			continue;
		ValueMap item(item_value);
		if(item.Get("name", "") == name)
			return true;
	}
	return false;
}

static String FirstCommandName(const ValueArray& commands)
{
	for(int i = 0; i < commands.GetCount(); i++) {
		Value item_value = commands[i];
		if(!item_value.Is<ValueMap>())
			continue;
		ValueMap item(item_value);
		String name = item.Get("name", "");
		if(!name.IsEmpty())
			return name;
	}
	return String();
}

ValueMap CommandRegistryClient::HeadlessSmoke(const String& smoke_command) const
{
	ValueMap report;
	report.Add("app", app_path);
	report.Add("smoke_command", smoke_command);

	ValueArray commands;
	RegistryProcessResult list_result = List(commands);
	report.Add("list_ok", list_result.ok);
	report.Add("list_error", list_result.error);
	report.Add("command_count", commands.GetCount());

	String describe_name = !smoke_command.IsEmpty() ? smoke_command : (CommandArrayContains(commands, "sample.echo") ? "sample.echo" : FirstCommandName(commands));
	ValueMap describe;
	RegistryProcessResult describe_result = describe_name.IsEmpty() ? RegistryProcessResult() : Describe(describe_name, describe);
	report.Add("describe_ok", describe_result.ok);
	report.Add("describe_error", describe_result.error);
	report.Add("describe_command", describe_name);

	String run_name = smoke_command;
	Vector<String> run_args;
	if(run_name.IsEmpty() && CommandArrayContains(commands, "sample.echo")) {
		run_name = "sample.echo";
		run_args << "text=gui-smoke";
	}
	RegistryProcessResult run_result;
	if(!run_name.IsEmpty())
		run_result = Run(run_name, run_args);
	else {
		run_result.ok = true;
		run_result.exit_code = 0;
	}
	report.Add("run_ok", run_result.ok);
	report.Add("run_error", run_result.error);
	report.Add("run_command", run_name);
	report.Add("run_skipped", run_name.IsEmpty());
	report.Add("run", run_result.json);

	bool ok = list_result.ok && describe_result.ok && run_result.ok && commands.GetCount() > 0;
	report.Add("ok", ok);
	return report;
}

CommandRegistryGuiWindow::CommandRegistryGuiWindow(const String& target_app)
{
	Title("Command Registry GUI");
	Sizeable().Zoomable();
	SetRect(0, 0, 1000, 700);

	client.SetAppPath(target_app);
	app_label.SetLabel("Application");
	args_label.SetLabel("Run args: key=value, one per line");
	app_path <<= client.GetAppPath();

	command_list.AddColumn("Name");
	command_list.AddColumn("Category");
	command_list.AddColumn("Description");
	command_list.WhenSel = [=] { DescribeSelected(); };
	command_list.ColumnWidths("220 120 520");

	output.SetReadOnly();
	run_args.SetData("text=hello");

	refresh_button.SetLabel("Refresh");
	describe_button.SetLabel("Describe");
	run_button.SetLabel("Run");
	refresh_button << [=] { RefreshCommands(); };
	describe_button << [=] { DescribeSelected(); };
	run_button << [=] { RunSelected(); };

	Add(app_label);
	Add(app_path);
	Add(refresh_button);
	Add(describe_button);
	Add(run_button);
	Add(command_list);
	Add(args_label);
	Add(run_args);
	Add(output);

	RefreshCommands();
}

void CommandRegistryGuiWindow::Layout()
{
	Size size = GetSize();
	int margin = DPI(8);
	int top = margin;
	int labelw = DPI(80);
	int buttonw = DPI(90);
	int lineh = DPI(24);
	int gap = DPI(6);

	app_label.SetRect(margin, top + DPI(3), labelw, lineh);
	run_button.SetRect(size.cx - margin - buttonw, top, buttonw, lineh);
	describe_button.SetRect(size.cx - margin - 2 * buttonw - gap, top, buttonw, lineh);
	refresh_button.SetRect(size.cx - margin - 3 * buttonw - 2 * gap, top, buttonw, lineh);
	app_path.SetRect(margin + labelw, top, size.cx - 2 * margin - labelw - 3 * buttonw - 3 * gap, lineh);

	int list_top = top + lineh + margin;
	int args_h = DPI(72);
	int output_h = max(DPI(180), size.cy / 3);
	int list_h = max(DPI(120), size.cy - list_top - args_h - output_h - 4 * margin);
	command_list.SetRect(margin, list_top, size.cx - 2 * margin, list_h);

	int args_top = list_top + list_h + margin;
	args_label.SetRect(margin, args_top, size.cx - 2 * margin, lineh);
	run_args.SetRect(margin, args_top + lineh, size.cx - 2 * margin, args_h - lineh);

	int output_top = args_top + args_h + margin;
	output.SetRect(margin, output_top, size.cx - 2 * margin, size.cy - output_top - margin);
}

String CommandRegistryGuiWindow::SelectedCommandName() const
{
	if(!command_list.IsCursor())
		return String();
	return command_list.Get(0).ToString();
}

Vector<String> CommandRegistryGuiWindow::SplitArgs() const
{
	Vector<String> out;
	Vector<String> lines = Split(run_args.GetData().ToString(), '\n', false);
	for(String line : lines) {
		line = TrimBoth(line);
		if(!line.IsEmpty())
			out.Add(line);
	}
	return out;
}

void CommandRegistryGuiWindow::RefreshCommands()
{
	client.SetAppPath(app_path.GetData().ToString());
	ValueArray commands;
	RegistryProcessResult result = client.List(commands);
	command_list.Clear();
	if(result.ok) {
		for(int i = 0; i < commands.GetCount(); i++) {
			Value item_value = commands[i];
			if(!item_value.Is<ValueMap>())
				continue;
			ValueMap item(item_value);
			command_list.Add(item.Get("name", ""), item.Get("category", ""), item.Get("description", ""));
		}
	}
	ShowResult(result);
}

void CommandRegistryGuiWindow::DescribeSelected()
{
	String name = SelectedCommandName();
	if(name.IsEmpty())
		return;
	client.SetAppPath(app_path.GetData().ToString());
	ValueMap command;
	ShowResult(client.Describe(name, command));
}

void CommandRegistryGuiWindow::RunSelected()
{
	String name = SelectedCommandName();
	if(name.IsEmpty())
		return;
	client.SetAppPath(app_path.GetData().ToString());
	ShowResult(client.Run(name, SplitArgs()));
}

void CommandRegistryGuiWindow::ShowResult(const RegistryProcessResult& result)
{
	String text;
	text << "ok=" << result.ok << " exit_code=" << result.exit_code << "\n";
	if(!result.error.IsEmpty())
		text << "error=" << result.error << "\n";
	if(!result.stdout_text.IsEmpty())
		text << "\nstdout:\n" << result.stdout_text;
	else if(!result.json.IsVoid())
		text << "\njson:\n" << JsonOrRaw(result);
	output.SetData(text);
}

String ParseCommandRegistryGuiTargetApp(const Vector<String>& args)
{
	for(int i = 0; i < args.GetCount(); i++) {
		String arg = args[i];
		if((arg == "--app" || arg == "--smoke-command") && i + 1 < args.GetCount()) {
			if(arg == "--app")
				return args[i + 1];
			i++;
		}
	}
	for(int i = 0; i < args.GetCount(); i++) {
		String arg = args[i];
		if(arg == "--headless-smoke")
			continue;
		if(arg.StartsWith("--")) {
			if((arg == "--app" || arg == "--smoke-command") && i + 1 < args.GetCount())
				i++;
			continue;
		}
		return arg;
	}
	return String();
}

String ParseCommandRegistryGuiSmokeCommand(const Vector<String>& args)
{
	for(int i = 0; i < args.GetCount(); i++) {
		String arg = args[i];
		if(arg == "--smoke-command" && i + 1 < args.GetCount())
			return args[i + 1];
	}
	return String();
}

} // namespace Upp

using namespace Upp;

GUI_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	const Vector<String>& args = CommandLine();
	bool headless = false;
	for(const String& arg : args)
		if(arg == "--headless-smoke")
			headless = true;
	String target_app = ParseCommandRegistryGuiTargetApp(args);
	String smoke_command = ParseCommandRegistryGuiSmokeCommand(args);
	if(headless) {
		CommandRegistryClient client(target_app);
		ValueMap report = client.HeadlessSmoke(smoke_command);
		Cout() << AsJSON(report, true) << "\n";
		Exit((bool)report.Get("ok", false) ? 0 : 1);
	}

	CommandRegistryGuiWindow(target_app).Run();
}
