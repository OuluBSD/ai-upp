#include "ScriptCLI.h"

NAMESPACE_UPP

int RunFileCommand(const Vector<String>& args)
{
	if(args.GetCount() < 2) {
		Cerr() << "run: missing file argument\n";
		return SCRIPTCLI_USAGE_ERROR;
	}

	String path = args[1];
	if(!FileExists(path)) {
		Cerr() << "run: file not found: " << path << "\n";
		return SCRIPTCLI_INFRA_ERROR;
	}

	String code = LoadFile(path);
	PyVM vm;
	RunManager run(vm);

	bool had_error = false;
	run.WhenError = [&](const String& err) {
		had_error = true;
		Cerr() << "run: " << err << "\n";
	};

	run.Run(code, path);
	return had_error ? SCRIPTCLI_RUNTIME_ERROR : SCRIPTCLI_OK;
}

END_UPP_NAMESPACE
