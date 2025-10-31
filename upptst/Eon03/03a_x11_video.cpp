#include "Eon03.h"

/*
machine x11.app:

	driver context:
		x11.context

	chain program:

		loop video:
			center.customer
			center.video.src.dbg_generator:
				mode = "noise"
			x11.video.pipe

 NamePart: x11
	NamePart: app
		MachineStmt:
			DriverStmt: context
				CompoundStmt:
					NamePart: x11
						AtomStmt: context
			ChainStmt: program
				CompoundStmt:
					LoopStmt: video
						CompoundStmt:
							NamePart: center
								AtomStmt: customer
								NamePart: video
									NamePart: src
										AtomStmt: dbg_generator
											CompoundStmt:
												ExprStmt:
													Unresolved: mode
														string: const("noise")
													assign: op(=)
							NamePart: x11
								NamePart: video
									AtomStmt: pipe

*/

NAMESPACE_UPP

void Run03aX11Video(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 2:
		// Note: Manual Context API for machine/driver/chain structure is not yet implemented
		// The machine/driver/chain structure is currently only supported via ScriptLoader (method 0)
		LOG("Method 2 (manual Context API) not yet implemented for machine/driver/chain structures");
		LOG("Using method 0 instead");
		sys->PostLoadFile(GetDataFile("03a_x11_video.eon"));
		break;
	case 1:
		// Note: Builder API for machine/driver/chain structure is not yet implemented
		// The machine/driver/chain structure is currently only supported via ScriptLoader (method 0)
		LOG("Method 1 (Builder API) not yet implemented for machine/driver/chain structures");
		LOG("Using method 0 instead");
		sys->PostLoadFile(GetDataFile("03a_x11_video.eon"));
		break;
	case 0:
		sys->PostLoadFile(GetDataFile("03a_x11_video.eon"));
		break;
	default:
		throw Exc(Format("Run03aX11Video: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
