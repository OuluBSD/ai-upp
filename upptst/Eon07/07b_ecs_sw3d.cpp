#include "Eon07.h"

/*
machine sdl.app:

	driver context:
		x11.sw.context

	chain program:
		state event.register

		loop video:
			center.customer
			x11.sw.fbo.program:
				drawmem =		"false"
				program =		"ecs_view"
				shader.default.frag.name =	"obj_view"
				shader.default.vtx.name =	"obj_view"

			x11.sw.video.pipe:
				close_machine =	true
				sizeable =		true
				env =			event.register
				program =		"obj_view"
				recv.data =      true


world ecs.dummy:
    system rendering
    system events
    system physics:
        //log: debug

    pool world:
        entity ground:
            comp transform3:
                x = 0
                y = 0
                z = 0
            comp model:
                builtin = plane
            comp physics:
                bind = false
                test.fn = fixed

        entity ball:
            comp transform3:
                x = -1
                y = 1
                z = 0
            comp body
            comp model:
                builtin = box

            comp physics:
                bind = false
                test.fn = fixed

        entity player:
            comp transform3:
                x = 0
                y = 1.6
                z = -6
            comp viewable
            comp viewport
            comp camera.chase:
                target = world.ball
                log = test
            comp physics:
                bind = true
                test.fn = do.circle

*/

NAMESPACE_UPP

void Run07bEcsSw3d(Engine& eng, int method) {
	// Add required systems for ECS functionality
	eng.GetAdd<InteractionSystem>();
	eng.GetAdd<RenderingSystem>();
	eng.GetAdd<EventSystem>();
	eng.GetAdd<PhysicsSystem>();

	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run07bEcsSw3d: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/07b_ecs_sw3d.eon"));
		break;
	default:
		throw Exc(Format("Run07bEcsSw3d: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
