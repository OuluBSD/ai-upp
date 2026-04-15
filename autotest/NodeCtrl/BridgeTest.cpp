#include <Node/Ctrl/Ctrl.h>

using namespace Upp;
using namespace Upp::Node;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	
	Graph g;
	g.AddNode("n1");
	
	Scene scene;
	BaselineSceneBuilder builder;
	builder.Build(scene, g);
	
	Viewport vp;
	
	NilDraw nw;
	PaintScene(nw, scene, vp);
	
	LOG("BridgeTest passed (compile and basic run).");
}
