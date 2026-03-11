#include "ScriptCLI.h"

NAMESPACE_UPP

int LintFileCommand(const Vector<String>& args)
{
	if(args.GetCount() < 2) {
		Cerr() << "lint: missing file argument\n";
		return SCRIPTCLI_USAGE_ERROR;
	}

	String path = args[1];
	if(!FileExists(path)) {
		Cerr() << "lint: file not found: " << path << "\n";
		return SCRIPTCLI_INFRA_ERROR;
	}

	String code = LoadFile(path);
	Linter lint;
	Vector<Linter::Message> msgs = lint.Analyze(code, path);

	if(msgs.IsEmpty()) {
		Cout() << "lint: OK\n";
		return SCRIPTCLI_OK;
	}

	for(const auto& m : msgs) {
		Cout() << path << ":" << m.line << ":" << m.column << ": "
		       << (m.is_error ? "error" : "warning") << ": " << m.text << "\n";
	}
	return SCRIPTCLI_RUNTIME_ERROR;
}

END_UPP_NAMESPACE
