#include "Eon00.h"

/*
PacketRouter DSL Test - Branching topology (fan-out without convergence)

Tests source fanning out to multiple independent destinations.
*/

NAMESPACE_UPP

void Run00gBranchNet(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/00g_branch_net.eon"));
		break;
	default:
		throw Exc(Format("Run00gBranchNet: method %d not implemented yet", method));
	}
}

END_UPP_NAMESPACE
