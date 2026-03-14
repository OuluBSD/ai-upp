#include "ScriptCLI.h"

NAMESPACE_UPP

int PluginCommand(const Vector<String>& args)
{
	if(args.GetCount() < 2) {
		Cerr() << "plugin: expected subcommand (list|test)\n";
		return SCRIPTCLI_USAGE_ERROR;
	}

	if(args[1] == "list") {
		Cout() << "Internal Plugins:\n";
		ScriptServices services(GetCurrentDirectory());
		for(const auto& info : services.ListPlugins())
			Cout() << "- " << info.id << " (" << info.name << "): " << info.description << "\n";
		return SCRIPTCLI_OK;
	}

	if(args[1] == "test") {
		if(args.GetCount() < 3) {
			Cerr() << "plugin test: missing <plugin-id>\n";
			return SCRIPTCLI_USAGE_ERROR;
		}
		
		String plugin_id = args[2];
		String case_name;
		for(int i = 3; i < args.GetCount(); i++) {
			const String& a = args[i];
			if(a == "--case" && i + 1 < args.GetCount())
				case_name = args[++i];
			else if(a.StartsWith("--case="))
				case_name = a.Mid(7);
			else {
				Cerr() << "plugin test: unknown argument '" << a << "'\n";
				return SCRIPTCLI_USAGE_ERROR;
			}
		}

		ScriptServices services(GetCurrentDirectory());
		ScriptPluginTestResult result = services.TestPlugin(plugin_id, case_name);
		if(!result.error.IsEmpty()) {
			Cerr() << "plugin test: " << result.error << "\n";
			return SCRIPTCLI_INFRA_ERROR;
		}

		Cout() << "Testing plugin: " << result.plugin_id << "\n";
		if(result.results.IsEmpty()) {
			Cout() << "No tests found\n";
			return SCRIPTCLI_OK;
		}

		for(const auto& item : result.results) {
			Cout() << "Running test: " << item.name << "... ";
			if(item.ok)
				Cout() << "PASSED\n";
			else if(item.message.IsEmpty())
				Cout() << "FAILED\n";
			else
				Cout() << "FAILED: " << item.message << "\n";
		}

		Cout() << "Summary: " << result.passed << " passed, " << result.failed << " failed.\n";
		return result.ok ? SCRIPTCLI_OK : SCRIPTCLI_INFRA_ERROR;
	}

	Cerr() << "plugin: unknown subcommand '" << args[1] << "'\n";
	return SCRIPTCLI_USAGE_ERROR;
}

END_UPP_NAMESPACE
