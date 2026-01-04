#include "Eon00.h"

/*
PacketRouter DSL Test - Diamond topology (parallel paths converging)

Tests two paths from source to sink converging.
*/

NAMESPACE_UPP

void Run00fDiamondNet(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/00f_diamond_net.eon"));
		break;
	default:
		throw Exc(Format("Run00fDiamondNet: method %d not implemented yet", method));
	}
}

END_UPP_NAMESPACE
