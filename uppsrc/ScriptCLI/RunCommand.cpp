#include "ScriptCLI.h"

NAMESPACE_UPP

int RunFileCommand(const Vector<String>& args)
{
	if(args.GetCount() < 2) {
		Cerr() << "run: missing file argument\n";
		return SCRIPTCLI_USAGE_ERROR;
	}

	ScriptServices services(GetCurrentDirectory());
	ScriptRunRequest req;
	req.path = args[1];
	req.WhenStdout = [&](const String& s) { Cout() << s; };
	req.WhenStderr = [&](const String& s) { Cerr() << "run: " << s << "\n"; };

	ScriptRunResult result = services.RunFile(req);
	if(result.ok)
		return SCRIPTCLI_OK;

	Cerr() << "run: " << result.error << "\n";
	return result.runtime_error ? SCRIPTCLI_RUNTIME_ERROR : SCRIPTCLI_INFRA_ERROR;
}

END_UPP_NAMESPACE
