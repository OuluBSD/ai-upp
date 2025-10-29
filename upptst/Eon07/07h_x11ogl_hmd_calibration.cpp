#include "Eon07.h"

/*
machine sdl.app:
	
	driver context:
		x11.ogl.context
	
	chain program:
		state event.register
		
		loop center.events.kb:
			center.customer
			x11.event.pipe
			state.event.pipe: target = event.register
		
		loop center.events.hmd:
			center.customer
			x11.ogl.holo.events:
				device.hmd.idx = -1
				device.left.idx = -1
				device.right.idx = -1
			state.event.pipe:
				target: event.register
		
		loop ogl.fbo:
			ogl.customer:
			x11.ogl.fbo.standalone:
				close_machine =	  true
				fullscreen =		 true
				find.vr.screen =	 true
				env =				event.register
				type =				 stereo
				shader.frag.path =	"shaders/tests/07d_fragment.glsl"
				shader.vtx.path =	"shaders/tests/07d_vertex.glsl"


world ecs.dummy:
	system rendering
	system events
	system interaction:
		env = event.register
		hmd = state
		calibration = true
	system physics
	system player
	
	pool world:
		entity ground:
			comp transform3
			comp model: builtin = plane
			comp physics: bind = physics, test.fn = fixed
		
		entity calib.left:
			comp transform3: y = 1.75, x = -1.1, cx = 0.05, cy = 0.05, cz = 0.05
			comp body
			comp model: builtin = box
			comp physics: bind = physics, test.fn = fixed
		
		entity box2:
			comp transform3: y = 1.75, x = +1.1, cx = 0.05, cy = 0.05, cz = 0.05
			comp body
			comp model: builtin = box
			comp physics: bind = physics, test.fn = fixed
		
		entity player.body:
			comp transform3: x = 0, y = 0, z = 2.2
			comp physics: bind = physics
			comp player.body: height = 1.74
		
		entity player.head:
			comp transform3
			comp viewable
			comp viewport: fov = 110
			comp camera.chase
			comp player.head: body = world.player.body
		
		entity player.hand.left:
			comp transform3
			comp body
			comp model: builtin = "cylinder,0.02,0.2", yaw = -90
			comp player.hand: hand = left, body = world.player.body
		
		entity player.hand.right:
			comp transform3
			comp body
			comp model: builtin = "cylinder,0.02,0.2", yaw = -90
			comp player.hand: hand = right, body = world.player.body

*/

NAMESPACE_UPP

void Run07hX11oglHmdCalibration(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run07hX11oglHmdCalibration: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("07h_x11ogl_hmd_calibration.eon"));
		break;
	default:
		throw Exc(Format("Run07hX11oglHmdCalibration: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
