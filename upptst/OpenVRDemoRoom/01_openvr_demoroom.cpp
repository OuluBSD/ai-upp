#include "OpenVRDemoRoom.h"

/*
machine openvr.app:
	
	driver context:
		sdl.ogl.context
	
	chain program:
		state event.register
		
		loop center.events:
			center.customer
			openvr.ogl.holo.events:
				device.hmd.idx = -1
				device.left.idx = -1
				device.right.idx = -1
			state.event.pipe:
				target = "/event/register"
		
		loop ogl.fbo:
			ogl.customer
			sdl.ogl.fbo.program:
				drawmem =		"false"
				program =		"ecs_view"
				shader.default.frag.path =		"shaders/tests/07j_fragment.glsl"
				shader.default.vtx.path =		"shaders/tests/07j_vertex.glsl"
				shader.sky.frag.path =			"shaders/tests/03n_skybox_fragment.glsl"
				shader.sky.vtx.path =			"shaders/tests/03n_skybox_vertex.glsl"
				type =			                stereo
			
			sdl.fbo.sink:
				close_machine =     true
				sizeable =		    true
				fullscreen =        true
				find.vr.screen =	true
				env = "/event/register"
				type =			    stereo


world ecs.dummy:
    system rendering
    system events
    system interaction: env = "/event/register", hmd = state
    system physics: rm.outsiders = true, rm.area.size = 20
    system player
    system paintstroke
    system toolbox: test.tool.changer = true
    system shooting
    system painting
    system throwing
    
    
    pool world:
        entity skybox:
            comp model:
				skybox.diffuse =	"bg5"
				skybox.irradiance =	"bg1"
                builtin =           "skybox"
        
        entity ground:
            comp transform3: x = 0, y = 0, z = 0
            comp model: path = "plane.obj"
            comp physics: bind = physics, test.fn = fixed
        
        entity mdl:
            comp transform3: y = 1
            comp body
            comp model: path = "diablo3_pose/diablo3_pose.obj"
            comp physics: bind = physics, test.fn = fixed
        
        entity player.body:
            comp transform3: x = 0, y = 0, z = 6
            comp physics: bind = physics
            comp player.body: height = 1.74
        
        entity player.head:
            comp transform3
            comp viewable
            comp viewport: fov = 90
            comp camera.chase
            comp player.head: body = world.player.body
        
        entity player.hand.left:
            comp transform3
            comp body
            comp model: builtin ="cylinder,0.02,0.2", pitch =-90
            comp player.hand: hand = left, body = world.player.body
        
        entity player.hand.right:
            comp transform3
            comp body
            comp model: builtin = "cylinder,0.02,0.2", pitch = -90
            comp player.hand: hand = right, body = world.player.body
        
        entity tool:
            comp transform3
            comp tool: hand = world.player.hand.right
            comp model: always.enabled = true
            comp shoot
            comp paint
            comp throw

*/

NAMESPACE_UPP

void Run01OpenvrDemoroom(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run01OpenvrDemoroom: method %d not implemented yet", method));
	case 0:
		{
			String path = ShareDirFile("eon/tests/01_openvr_demoroom.eon");
			String content = LoadFile(path);
			if(content.IsEmpty()) {
				LOG("Error: could not load " << path);
				sys->PostLoadFile(path);
			}
			else {
				// Replace the entire ground model component line with correct indentation
				content.Replace("comp model: path = \"plane.obj\"", 
					"comp model:\n"
					"\t\t\t\tbuiltin = \"cube\"\n"
					"\t\t\t\tcolor = \"0.7,0.7,0.7,1\"\n"
					"\t\t\t\tcx = 100\n"
					"\t\t\t\tcy = 0.1\n"
					"\t\t\t\tcz = 100");
				
				// Fix skybox paths
				content.Replace("\"bg5\"", "\"" + ShareDirFile("demoroom/Environment/EnvHDR.dds") + "\"");
				content.Replace("\"bg1\"", "\"" + ShareDirFile("demoroom/Environment/DiffuseHDR.dds") + "\"");
				
				// Switch to mono for clearer Vision API analysis
				content.Replace("type = stereo", "type = mono");
				content.Replace("type =                stereo", "type = mono");
				
				// Inject marker cube - make it VERY OBVIOUS
				String marker;
				marker << "\n\t\tentity marker_cube:\n"
				       << "\t\t\tcomp transform3:\n"
				       << "\t\t\t\tx = 0\n"
				       << "\t\t\t\ty = 2\n"
				       << "\t\t\t\tz = 2\n"
				       << "\t\t\t\tcx = 0.5\n"
				       << "\t\t\t\tcy = 0.5\n"
				       << "\t\t\t\tcz = 0.5\n"
				       << "\t\t\tcomp model:\n"
				       << "\t\t\t\tbuiltin = \"cube\"\n"
				       << "\t\t\t\tcolor = \"1,1,1,1\"\n"; // Bright white

				marker << "\n\t\tentity test_sphere:\n"
				       << "\t\t\tcomp transform3:\n"
				       << "\t\t\t\tx = 0\n"
				       << "\t\t\t\ty = 2\n"
				       << "\t\t\t\tz = 4\n"
				       << "\t\t\tcomp model:\n"
				       << "\t\t\t\tbuiltin = \"sphere,1.0\"\n"
				       << "\t\t\t\tcolor = \"1,0,0,1\"\n"; // Red
				
				content.Replace("pool world:", "pool world:" + marker);
				
				sys->PostLoadString(content);
			}
		}
		break;
	default:
		throw Exc(Format("Run01OpenvrDemoroom: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
