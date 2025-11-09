#include "Eon07.h"

/*
machine ecs.app:
    pass

world ecs.dummy:
    system rendering:
        dummy = true
    system events
    system physics:
        //log: debug
    
    pool world:
        entity ground:
            comp transform3
            comp model:
                builtin = plane
            
            comp physics:
                bind = physics
                test.fn = fixed
        
        entity ball:
            comp transform3
            comp body
            comp model:
                builtin = sphere
            
            comp physics:
                bind = physics
                test.fn = do.circle
        
        entity player:
            comp transform3:
                x = 0
                y = 0
                z = 6
            comp viewable
            comp viewport
            comp camera.chase:
                target = world.ball
                log = test
            comp physics:
                bind = physics
                test.fn = fixed

*/

NAMESPACE_UPP

void Run07aEcsDummy(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run07aEcsDummy: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/07a_ecs_dummy.eon"));
		break;
	default:
		throw Exc(Format("Run07aEcsDummy: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
