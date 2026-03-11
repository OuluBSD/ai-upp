#include "ScriptCLI.h"

NAMESPACE_UPP

int PluginCommand(const Vector<String>& args)
{
	if(args.GetCount() < 2) {
		Cerr() << "plugin: expected subcommand (list|test)\n";
		return SCRIPTCLI_USAGE_ERROR;
	}

	if(args[1] == "list") {
		Cout() << "plugin list: not implemented yet\n";
		return SCRIPTCLI_OK;
	}

	if(args[1] == "test") {
		if(args.GetCount() < 3) {
			Cerr() << "plugin test: missing <plugin-id>\n";
			return SCRIPTCLI_USAGE_ERROR;
		}
		Cout() << "plugin test: not implemented yet for plugin '" << args[2] << "'\n";
		return SCRIPTCLI_OK;
	}

	Cerr() << "plugin: unknown subcommand '" << args[1] << "'\n";
	return SCRIPTCLI_USAGE_ERROR;
}

END_UPP_NAMESPACE
