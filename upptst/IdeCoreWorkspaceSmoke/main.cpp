#include "IdeCoreWorkspaceSmoke.h"

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	LOG("=== Running IdeCoreWorkspace Smoke Tests ===");

	IdeCoreWorkspace workspace;
	IdeCoreConsoleHost host;

	LOG("Testing ScanAllNests...");
	VectorMap<String, String> pkgs = workspace.ScanAllNests();
	LOG(Format("ScanAllNests found %d packages", pkgs.GetCount()));
	for(int i = 0; i < pkgs.GetCount(); i++) {
		LOG(Format("Package: %s at %s", pkgs.GetKey(i), pkgs[i]));
	}

	LOG("Testing streaming Execute...");
	String accumulated;
	auto on_output = [&](String chunk) {
		accumulated << chunk;
	};

	int code = host.Execute("echo Demo", on_output);
	LOG(Format("Execute returned code: %d", code));
	LOG(Format("Accumulated: %s", TrimBoth(accumulated)));

	ASSERT(code == 0);
	ASSERT(TrimBoth(accumulated) == "Demo");

	LOG("Smoke tests completed successfully!");
}
