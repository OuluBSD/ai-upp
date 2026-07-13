#ifndef _CommandRegistryGui_CommandRegistryGui_h_
#define _CommandRegistryGui_CommandRegistryGui_h_

#include <CtrlLib/CtrlLib.h>

NAMESPACE_UPP

struct RegistryCommandInfo : Moveable<RegistryCommandInfo> {
	String name;
	String category;
	String description;
};

struct RegistryProcessResult : Moveable<RegistryProcessResult> {
	bool ok = false;
	int exit_code = -1;
	String stdout_text;
	String error;
	Value json;
};

class CommandRegistryClient {
public:
	CommandRegistryClient() {}
	CommandRegistryClient(const String& app_path) : app_path(app_path) {}

	void SetAppPath(const String& path) { app_path = path; }
	String GetAppPath() const { return app_path; }

	RegistryProcessResult List(ValueArray& commands) const;
	RegistryProcessResult Describe(const String& name, ValueMap& command) const;
	RegistryProcessResult Run(const String& name, const Vector<String>& key_value_args) const;

	ValueMap HeadlessSmoke(const String& smoke_command = String()) const;

private:
	String app_path;

	RegistryProcessResult Execute(const Vector<String>& args) const;
};

class CommandRegistryGuiWindow : public TopWindow {
public:
	typedef CommandRegistryGuiWindow CLASSNAME;

	CommandRegistryGuiWindow(const String& target_app);

	void Layout() override;

private:
	CommandRegistryClient client;

	EditString app_path;
	ArrayCtrl command_list;
	LineEdit run_args;
	LineEdit output;
	Button refresh_button;
	Button describe_button;
	Button run_button;
	Label app_label;
	Label args_label;

	void RefreshCommands();
	void DescribeSelected();
	void RunSelected();
	String SelectedCommandName() const;
	Vector<String> SplitArgs() const;
	void ShowResult(const RegistryProcessResult& result);
};

String ParseCommandRegistryGuiTargetApp(const Vector<String>& args);
String ParseCommandRegistryGuiSmokeCommand(const Vector<String>& args);

END_UPP_NAMESPACE

#endif
