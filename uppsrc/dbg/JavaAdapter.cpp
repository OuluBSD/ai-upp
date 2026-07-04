#include "dbg.h"

using namespace Upp;

String JavaBackendSession::GetBackendName() const
{
	return "java";
}

DbgRunResult JavaBackendSession::Run(const DbgLaunchRequest& request)
{
	DbgRunResult result;
	result.backend_name = "java";
	result.started = true;

	DbgBacktrace bt = GetJavaBacktrace(request.executable_path, request.arguments, request.working_directory, request.environment, result.transcript);
	result.error = bt.error;
	if (bt.error.IsEmpty()) {
		result.exit_code = 0;
	} else {
		result.exit_code = 1;
		result.crashed = true;
	}

	for (const auto& f : bt.frames) {
		DbgCallStackFrame& cf = result.call_stack.Add();
		cf.address = f.address;
		cf.function = f.function;
		cf.source_file = f.file;
		cf.line = f.line;
	}

	return result;
}
