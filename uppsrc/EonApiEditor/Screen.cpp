#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddScreen() {
	Package("Screen", "Scr");
	SetColor(112, 112, 112);
	Dependency("ParallelLib");
	Dependency("IGraphics");
	Library("X11 Xrandr Xxf86vm", "POSIX & !EMSCRIPTEN");
	Library("GLX GL GLU GLEW glut", "POSIX & OGL & !EMSCRIPTEN");
	Library("D3D11", "WIN32 & DX11");
	HaveRecvFinalize();
	HaveNegotiateFormat();
	HaveIsReady();
	
	Interface("SinkDevice", "SCREEN");
	Interface("Context", "SCREEN");
	Interface("EventsBase", "SCREEN");
	
	Vendor("X11",		"X11");
	Vendor("X11Sw",		"X11");
	Vendor("X11Ogl",	"X11&OGL");
	Vendor("Win",		"WIN32&!UWP");
	//Vendor("WinSw",	"WIN32");
	Vendor("WinD11",	"WIN32&DX11");
	
	
}


END_UPP_NAMESPACE
