#include "ScriptCLI.h"

NAMESPACE_UPP

int LintFileCommand(const Vector<String>& args)
{
	if(args.GetCount() < 2) {
		Cerr() << "lint: missing file argument\n";
		return SCRIPTCLI_USAGE_ERROR;
	}

	ScriptServices services(GetCurrentDirectory());
	ScriptLintResult result = services.LintFile(args[1]);
	if(result.path.IsEmpty()) {
		Cerr() << "lint: path is required\n";
		return SCRIPTCLI_USAGE_ERROR;
	}
	if(!FileExists(result.path)) {
		Cerr() << "lint: file not found: " << result.path << "\n";
		return SCRIPTCLI_INFRA_ERROR;
	}

	if(result.issues.IsEmpty()) {
		Cout() << "lint: OK\n";
		return SCRIPTCLI_OK;
	}

	for(const auto& m : result.issues) {
		Cout() << result.path << ":" << m.line << ":" << m.column << ": "
		       << m.severity << ": " << m.text << "\n";
	}
	return SCRIPTCLI_RUNTIME_ERROR;
}

END_UPP_NAMESPACE
